#!/bin/bash
#脚本路径
cur_dir=$(cd "$(dirname "$0")"; pwd)

# 需要备份的日志文件前缀
HOSTNAME=$(hostname)
MYSQL_DATA_DIR=$(cat /etc/my.cnf | grep datadir | awk -F "=" '{print $2}' | sed 's/ //g')
LOG_ERROR_FILE=$(grep log-error /etc/my.cnf | awk -F "." '{print $1}')
if [ -z "$LOG_ERROR_FILE" ];then
	SOURCE_LOG_FILE_PREFIX=$HOSTNAME
else
	SOURCE_LOG_FILE_PREFIX=$LOG_ERROR_FILE
fi

# 日志备份目录
TARGET_LOG_DIR_PATH=/cloud/dahua/DataBaseServer/DBHA/Log/
# 转储后日志文件名称前缀
TARGET_LOG_PREFIX=${SOURCE_LOG_FILE_PREFIX}_
# 最多允许备份几个日志文件(不压缩)
BACK_UP_LOG_NUM=100
FILE_MAX_SIZE=100

# 本脚本打印的日志文件
SHELL_LOG=$CURRENT_LOG_DIR_PATH/Shell.log

#默认7天打一个压缩包
DAYS_IN_TAR=7
#每天有多少秒
SECS_PER_DAY=86400
#打包时间阀值
TAR_INTERVAL_TIME=`expr $DAYS_IN_TAR \* $SECS_PER_DAY`
#默认保存2个月左右的日志
BACKUP_TAR_MAX=8
MYSQL_CONFG=/cloud/dahua/DataBaseServer/DBHA/Config/Mysql.conf
BUCKET_CAP=$(grep "mysql_log_capacity" $MYSQL_CONFG | awk -F": |,|/*" '{print $2}')
BACKUP_TAR_CAP=`expr $BUCKET_CAP \* 1024 \* 1024 \* 1024`

#备份文件夹名称
BACKUP_LOG_DIR=backup_mysql_dir
BACKUP_LOG_DIR_PREFIX=backup_mysql

#拷贝cs日志限速
CP_LIMIT_BIN=$cur_dir/cpWithSpeed
CP_LIMIT_SPEED=20MB

redirect()
{
	if [ -z "$1" ];then
		return 1
	fi
	  
	echo "`date +"%F %T"` CycleSyncMySQLLog.sh"  "$1" >> $SHELL_LOG
}

#############################################################################################################
#compress_sync_all_log 是新的转储方式，转储$CURRENT_LOG_DIR_PATH下的current.log之外的全部日志
# 标准模式, 转储并压缩日志
# 不传参数：默认只转储除current.log之外的其他日志
# 传任意参数(比如1)：则转储包括current.log在内的所有日志
function compress_sync_all_log()
{	
	# 不转储 current.log
	# 检查转储目录(默认是机械盘 /cloud/dahua/DataBaseServer/DBHA/Log)是否存在
	if [ ! -d $TARGET_LOG_DIR_PATH ];then
		redirect "target log directory(${TARGET_LOG_DIR_PATH}) does not exist, create it"
		mkdir -p $TARGET_LOG_DIR_PATH
		if [ $? -ne 0 ];then
			redirect "failed to create directory ${TARGET_LOG_DIR_PATH}"
			exit 0
		fi
	fi
	
	# 检查转储目录下的临时目录(/cloud/dahua/DataBaseServer/DBHA/Log/backup_mysql_dir)是否存在
	if [ ! -d $TARGET_LOG_DIR_PATH/$BACKUP_LOG_DIR ];then
		redirect "target backup directory($TARGET_LOG_DIR_PATH/$BACKUP_LOG_DIR) does not exist, create it"
		mkdir -p $TARGET_LOG_DIR_PATH/$BACKUP_LOG_DIR
		if [ $? -ne 0 ];then
			redirect "failed to create directory $TARGET_LOG_DIR_PATH/$BACKUP_LOG_DIR"
			exit 0
		fi
	fi
	
	#获取临时目录下(/cloud/dahua/DataBaseServer/DBHA/Log/backup_mysql_dir)下，log文件的最大编号
	declare -i dir_log_no_max=0
	last_log_file=$(ls -tr $TARGET_LOG_DIR_PATH/$BACKUP_LOG_DIR|grep $TARGET_LOG_PREFIX|tail -n 1 2>/dev/null)
	if [ -n "$last_log_file" ];then
		dir_log_no_max=$(echo $last_log_file|awk -F[_.] '{print $3}')
	fi
	
	# 获取当前日志文件清单,按照时间从小到达显示(ls -tr)
	log_list=$(ls -tr ${MYSQL_DATA_DIR}/${SOURCE_LOG_FILE_PREFIX}*.err 2>/dev/null)
	declare -i dest_logfile_no=$dir_log_no_max
	for logfile in ${log_list[@]};do
		logfile_size=$(ls -l --block-size=M $logfile | awk '{print $(NF-4)}' | sed 's/M//g')
		if [ $logfile_size -lt $FILE_MAX_SIZE ];then
			continue
		fi
		
		dest_logfile_no=$[$dest_logfile_no+1]
		
		# 转储日志
		# 转储后的日志以"日期时间"为后缀,例如 _20141121161056
		file_suffix=$(date -d "today" +"%Y%m%d%H%M%S")
		target_file=${TARGET_LOG_PREFIX}${dest_logfile_no}_${file_suffix}.err
		
		# 拷贝/dev/shm/CatalogServer/ 目录下的日志到转储目录
		if [ -x $CP_LIMIT_BIN ]; then
			redirect "$CP_LIMIT_BIN --limit-speed $CP_LIMIT_SPEED $logfile ${TARGET_LOG_DIR_PATH}/${BACKUP_LOG_DIR}/$target_file"
			chmod +x $CP_LIMIT_BIN
			$CP_LIMIT_BIN --limit-speed $CP_LIMIT_SPEED $logfile ${TARGET_LOG_DIR_PATH}/${BACKUP_LOG_DIR}/$target_file
			redirect "rm -rf $logfile"
			rm -rf $logfile     
		else
			redirect "mv $logfile  ${TARGET_LOG_DIR_PATH}/${BACKUP_LOG_DIR}/$target_file"
			mv $logfile ${TARGET_LOG_DIR_PATH}/${BACKUP_LOG_DIR}/$target_file  > /dev/null 2>&1
		fi        
	done
	
	#判断是否需要打压缩包
	declare -i need_tar_flag=0
	#比较时间戳对应的时间,比较当前时间和目录中最早文件时间的时间间隔
	declare -i cur_timestamp=$(date -d "today" +%s)
	declare -i tmp_timestamp=`expr $cur_timestamp - $TAR_INTERVAL_TIME`
	declare -i tmp_time=$(date -d "@$tmp_timestamp" +"%Y%m%d%H%M%S")
	declare -i dir_first_log_time=0
	first_log_file=$(ls -tr $TARGET_LOG_DIR_PATH/$BACKUP_LOG_DIR|grep $TARGET_LOG_PREFIX|head -n 1 2>/dev/null)
	if [ -n "$first_log_file" ];then
		dir_first_log_time=$(echo $first_log_file|awk -F[_.] '{print $3}')
		#如果时间间隔大于指定阀值，则需要打包
		if [ $dir_first_log_time -le $tmp_time ];then
			need_tar_flag=1
		fi
	else
		#说明目录中没有文件，也就不用打包了
		return 0
	fi
	
	#压缩、打包; 删除超过阀值的TAR包
	if [ $need_tar_flag -eq 1 ];then
		cd  ${TARGET_LOG_DIR_PATH}
		
		#先重命名文件夹，然后压缩打包、再删除文件夹
		cur_time=$(date -d "today" +"%Y%m%d%H%M%S")
		tmp_backup_log_dir=${BACKUP_LOG_DIR_PREFIX}${cur_time}
		redirect "mv ${TARGET_LOG_DIR_PATH}/${BACKUP_LOG_DIR} ${TARGET_LOG_DIR_PATH}/$tmp_backup_log_dir"
		mv ${BACKUP_LOG_DIR} $tmp_backup_log_dir
		
		#打压缩包
		redirect "tar czvf ${TARGET_LOG_PREFIX}${cur_time}.tar -C ${TARGET_LOG_DIR_PATH}  ${TARGET_LOG_DIR_PATH}/$tmp_backup_log_dir"
		tar czvf ${TARGET_LOG_PREFIX}${cur_time}.tar.gz -C ${TARGET_LOG_DIR_PATH} $tmp_backup_log_dir/ > /dev/null 2>&1

		#删除临时文件
		redirect "delete tmp dir: rm -rf ${TARGET_LOG_DIR_PATH}/$tmp_backup_log_dir"
		rm -rf $tmp_backup_log_dir 
		
		#判断是否需要删除超过阀值的tar包,如果个数已经达到上限，就循环覆盖.通过 ls 按时间排序,删除时间最早的日志文件
		tar_num=$(ls -lh ${TARGET_LOG_PREFIX}*.tar*  2>/dev/null  | wc -l)
		if [ ${tar_num} -gt ${BACKUP_TAR_MAX} ];then
			# 日志转储数量已达上限,循环删除最老的日志
			to_del_num=`expr $tar_num - $BACKUP_TAR_MAX`
			for((index=0;index<$to_del_num;index++));do
				tmp=$(ls -ltr ${TARGET_LOG_PREFIX}*.tar* | head -n 1 | awk '{print $NF}')
				redirect "number of tars reaches the limit($BACKUP_TAR_MAX) , rm ${tmp}"
				rm ${tmp}
			done
		fi
		
		#判断容量是否达到上限;仅删除1个
		log_capacity=`ls ${TARGET_LOG_PREFIX}*.tar*  2>/dev/null | xargs du -cb | grep -w "total" | awk '{s+=$1}END{print s}'`
		if [ $log_capacity -gt $BACKUP_TAR_CAP ];then
			tmp=$(ls -ltr ${TARGET_LOG_PREFIX}*.tar* | head -n 1 | awk '{print $NF}')
			redirect "capacity of tars reaches the limit($BACKUP_TAR_CAP) , rm ${tmp}"
			rm ${tmp}
		fi
	fi
}

#############################################################################################################
# 转储并压缩
# 本脚本使用方式：
# 直接运行(不同步当前日志文件):sh SyncLog.sh
compress_sync_all_log


