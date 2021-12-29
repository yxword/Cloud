#日志路径
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
. $cur_dir/DBHAConfig.sh

LOG_FILE=$DBHA_LOG/Shell.log
MYSQL_BACKUP_THROTTLE=1000		#mysql备份限制每秒的IO次数（经验值）
SHOW_CONSOLE=N
SQL_FILE=$DBHA_CONFIG/Sql.stat
CFG_FILE=$DBHA_CONFIG/Mysql.conf

#同步信息的用户名和密码
sync_user="";
sync_password="";
mysql_user=""
mysql_password=""
mysql_config=/etc/my.cnf
ignore_cfg=replicate-ignore-db
all_ignore_dbs=
total_bin_log_max=90
total_slave_relay_log_max=81
total_slave_bin_log_max=9
total_bin_log_cnt_threshold=400
backupFileRecordName=backFileRecord

# 全局变量：待删除文件列表。调用下面的函数会重新计算并赋值
g_backup_todel_files=

mysql_config=$DBHA_CONFIG/my.cnf
ignore_cfg=replicate-ignore-db
ignore_wild_cfg=replicate-wild-ignore-table
# 服务配置文件(含网卡信息的配置文件)
CONFIG_PATH=$DBHA_CONFIG/DBHA.conf

mysql_config=/etc/my.cnf
ignore_cfg=replicate-ignore-db

CONFIG_TOOL=$DBHA_BIN/ConfigModifier
peer_business_IP=

# 日志目录
if [ ! -d "$DBHA_LOG" ]; then
  mkdir -p $DBHA_LOG
fi

#参数1 模块名
#参数2..n message
function logInfo()
{
    echo -e "$(date +"%F %T")|[$TAG] $*" >> $LOG_FILE
    [ $SHOW_CONSOLE == Y ] && echo "$(date +"%F %T")|[$TAG] $*"
	return 0
}

function getUserCfg()
{
    mysql_user=`cat $CFG_FILE| grep -w "user_security" | awk -F "\"" '{print $4}' | base64 -d`
    mysql_password=`cat $CFG_FILE| grep -w "password_security" | awk -F "\"" '{print $4}' | base64 -d`
}

function getSyncUserCfg()
{
	sync_user=$(cat $CFG_FILE| grep -w "sync_user" | awk -F "\"" '{print $4}' | base64 -d)
	sync_password=$(cat $CFG_FILE| grep -w "sync_password" | awk -F "\"" '{print $4}' | base64 -d)
}

function getBinlogCfg()
{
    master_bin_log_capacity=`cat $CFG_FILE | grep -w "bin_log_capacity" | awk -F": |,|/*" '{print $2}'`
    #binlog的最大保留值：设定值的90%，但是最小为设定值-2个binlog大小
    let total_bin_log_max=$master_bin_log_capacity*1024*1024*1024*90/100

    #备机上的relaylog最大保留值,binlog最大保留值
    slave_relay_log_percentage=`cat $CFG_FILE | grep -w "relay_log_percentage" | awk -F": |,|/*" '{print $2}'| awk '{print $1}'`
    let total_slave_relay_log_max=$total_bin_log_max*$slave_relay_log_percentage/100
    let total_slave_bin_log_max=$total_bin_log_max-$total_slave_relay_log_max
}

function checkDir()
{
	file_dir=$1
	if [ ! -d $file_dir ];then
		logInfo "create $file_dir"
		mkdir -p $file_dir
	fi
}

function getIgnoreDB()
{
	all_ignore_dbs="Database|information_schema|performance_schema|sys"
	DBNames=`cat $mysql_config |grep "$ignore_cfg"|awk -F "=" '{print $2}'| awk '{gsub(" ","");print}'`
	for DBName in $DBNames;
	do
		if [ ! -z $DBName ];then
			all_ignore_dbs=$all_ignore_dbs\|$DBName
			logInfo "$DBName"
		fi
	done
	logInfo "ignore db is $all_ignore_dbs"
}

#备份文件，$1:备份文件的路径，$2:UUID，$3:server_id，$4:主备模式master slave，$5:自动还是手动auto,manual
function backupFileCommon()
{
    #检测备份空间容量
	backup_capacity=`df -h |grep '/cloud$'|awk '{print $(NF-1)}'|awk -F "%" '{print $1}'`
	if [[ $backup_capacity == '' ]];then
		backup_capacity=`df -h |grep '/$'|head -1|awk '{print $(NF-1)}'|awk -F "%" '{print $1}'`
	fi
	#检测data目录空间容量
	data_dir=`df -h |grep '/data$'|awk '{print $(NF-1)}'|awk -F "%" '{print $1}'`
	if [[ $backup_capacity -gt 90 ]] || [[ $data_dir -gt 90 ]];then
		logInfo "backup_mysql failed! the backup path capacity ($backup_capacity) is not enough"
		return 1
	fi
	
    file_dir=$1
    uuid=$2
    server_id=$3
    mode=$4
    auto=$5

    checkDir $file_dir
    getUserCfg
	getIgnoreDB
    logInfo "start to back up,file_dir:$file_dir,uuid:$uuid,server_id:$server_id,mode:$mode,auto:$auto.";
	
    master_log_info=`mysql -u$mysql_user -p$mysql_password  -e "show master status\G"`
    master_log_file_name=$(echo $master_log_info | awk -F "File: " '{print $2}' | awk -F " " '{print $1}')
    master_log_pos=$(echo $master_log_info | awk -F "Position: " '{print $2}' | awk -F " " '{print $1}')
	
    #清理历史备份日志
    rm -rf $file_dir/backup.log
    #导出存储过程，事件，触发器
    databases=$(mysql -u$mysql_user -p$mysql_password -e "show databases;" -N 2>/dev/null |grep -Ewv "$all_ignore_dbs" )
    if [ -z "$databases" ];then
        #仅写入change master to 
        echo "CHANGE MASTER TO MASTER_LOG_FILE='$master_log_file_name', MASTER_LOG_POS=$master_log_pos;" | gzip > ${file_dir}/${auto}_${mode}_${uuid}_${server_id}_tmp
    else
        mysql -u$mysql_user -p$mysql_password -e "show databases;" -N 2>/dev/null |grep -Ewv "$all_ignore_dbs" | xargs mysqldump -u$mysql_user -p$mysql_password --databases --master-data=1 --single-transaction --events --routines --triggers --add-drop-database --flush-privileges --log-error=$file_dir/backup.log |gzip > ${file_dir}/${auto}_${mode}_${uuid}_${server_id}_tmp
    fi
	#获取备份的错误日志
	errorMessage=$(cat $file_dir/backup.log|grep 'Error')
    errorNum=$(cat $file_dir/backup.log|grep 'Error'|wc -l)
    
	#如果日志中有“Error”日志，则认为备份失败，备份文件是不完整的，需要删除
	if [ $errorNum != 0 ];then
             logInfo "backup_mysql failed! mysqldump return:"$errorMessage
             rm ${file_dir}/${auto}_${mode}_${uuid}_${server_id}_tmp
             return 1
        fi
    
	#检测dump是否正在进行
    #pid=`ps -ef|grep "mysqldump"|grep "$mysql_user"|grep "$file_dir"|grep "$mode"|grep "$uuid" | grep "$server_id" | grep -v grep | awk '{print $2}'`
    #while [[ "$pid" != "" ]];
    #do
    #    sleep 1
    #    pid=`ps -ef|grep "mysqldump"|grep "$mysql_user"|grep "$file_dir"|grep "$mode"|grep "$uuid" | grep "$server_id" | grep -v grep | awk '{print $2}'`
    #done
    logInfo "end to back up ,file_dir:$file_dir,uuid:$uuid,server_id:$server_id,mode:$mode,auto:$auto."
    new_file_name=${file_dir}/${auto}_${mode}_${uuid}_${server_id}_$(date "+%Y_%m_%d_%H%M%S").gz
    mv ${file_dir}/${auto}_${mode}_${uuid}_${server_id}_tmp $new_file_name
    master_md5_sum=$(md5sum $new_file_name | awk -F " " '{print $1}')
	
    #增加备份记录
    if [[ "$mode" = "master" ]];then
        mysql_data_dir=$(cat /etc/my.cnf|grep "datadir"| awk -F "=" '{print $2}' | sed 's/[ ]//g')
        #每次记录都插入第1行,虽然binlog用的是相对路径，可能重复安装binlog会重掉，但是重复安装uuid肯定不会一样，靠文件唯一来保证
        if [[ ! -f "$file_dir/$backupFileRecordName" ]] || [[ -z `cat $file_dir/$backupFileRecordName` ]] ;then
            echo "$new_file_name $mysql_data_dir/$master_log_file_name $master_log_pos $master_md5_sum" > $file_dir/$backupFileRecordName
        else
            sed -i '1i\'$new_file_name' '$mysql_data_dir/$master_log_file_name' '$master_log_pos' '$master_md5_sum'' $file_dir/$backupFileRecordName
        fi
        logInfo "add record($new_file_name, $mysql_data_dir/$master_log_file_name, $master_log_pos) to $backup_dir/$backupFileRecordName"
        num=`cat $CFG_FILE | grep -w "backup_num" | awk -F": |,|/*" '{print $2}'`
        purgeMasterFileWithNum $file_dir $uuid $server_id $num
    else
        purgeSlaveBackupFile $file_dir $uuid $server_id
    fi
	
	backup_size=$(du -bh $new_file_name | awk '{print $1}')
    logInfo "end to back up, file_dir:$file_dir uuid:$uuid, server_id=$server_id,mode:$mode,auto:$auto,name:$new_file_name,size:$backup_size."
    return 0
}

#备份文件，$1:备份文件的路径，$2:UUID，$3:server_id，$4:主备模式master slave，$5:自动还是手动auto,manual
function backupFileWithXtrabackup()
{
    file_dir=$1
    uuid=$2
    server_id=$3
    mode=$4
    auto=$5

    checkDir $file_dir
    getUserCfg
    logInfo "start to back up,file_dir:$file_dir,uuid:$uuid,server_id:$server_id,mode:$mode,auto:$auto.";
	
	XTRABACKUP_HOME=/cloud/dahua/DataBaseServer/xtrabackup
	PATH=$PATH:$XTRABACKUP_HOME/bin
	export PATH XTRABACKUP_HOME
	source /etc/profile
	
	rm -rf $file_dir/backup.log
	innobackupex --user=$mysql_user --password=$mysql_password --throttle=$MYSQL_BACKUP_THROTTLE --no-lock \
		--stream=tar "$file_dir" 2>$file_dir/backup.log | gzip > "$file_dir/${auto}_${mode}_${uuid}_${server_id}_tmp"
	if [ $? != 0 ];then
		log_info $LOG_FILE "backup_mysql failed!"
		return 1
	fi
   
    logInfo "end to back up ,file_dir:$file_dir,uuid:$uuid,server_id:$server_id,mode:$mode,auto:$auto."
	date_info=$(date "+%Y_%m_%d_%H%M%S")
    new_file_name=${file_dir}/${auto}_${mode}_${uuid}_${server_id}_${date_info}.gz
	new_backup_log=${file_dir}/backup_${date_info}.log
    mv ${file_dir}/${auto}_${mode}_${uuid}_${server_id}_tmp $new_file_name
	mv $file_dir/backup.log $new_backup_log
	master_log_file_name=$(cat $new_backup_log | grep "binlog position" | awk -F "filename " '{print $2}' | awk -F "," '{print $1}' | tr -d "\'")
	master_log_pos=$(cat $new_backup_log | grep "binlog position" | awk -F "position " '{print $2}' | tr -d "\'")
	master_md5_sum=$(md5sum $new_file_name | awk -F " " '{print $1}')
	
    #增加备份记录
    if [[ "$mode" = "master" ]];then
		mysql_data_dir=$(cat /etc/my.cnf|grep "datadir"| awk -F "=" '{print $2}' | sed 's/[ ]//g')
		#每次记录都插入第1行,虽然binlog用的是相对路径，可能重复安装binlog会重掉，但是重复安装uuid肯定不会一样，靠文件唯一来保证
		if [[ ! -f "$file_dir/$backupFileRecordName" ]] || [[ -z `cat $file_dir/$backupFileRecordName` ]] ;then
			echo "$new_file_name $mysql_data_dir/$master_log_file_name $master_log_pos $master_md5_sum" > $file_dir/$backupFileRecordName
		else
			sed -i '1i\'$new_file_name' '$mysql_data_dir/$master_log_file_name' '$master_log_pos' '$master_md5_sum'' $file_dir/$backupFileRecordName
		fi
		logInfo "add record($new_file_name, $mysql_data_dir/$master_log_file_name, $master_log_pos) to $backup_dir/$backupFileRecordName"
		num=`cat $CFG_FILE | grep -w "backup_num" | awk -F": |,|/*" '{print $2}'`
		purgeMasterFileWithNum $file_dir $uuid $server_id $num
    else
        purgeSlaveBackupFile $file_dir $uuid $server_id
    fi

    logInfo "end to back up, file_dir:$file_dir uuid:$uuid, server_id=$server_id,mode:$mode,auto:$auto."
}

#写的binlog文件为绝对路径,$1:备份文件的目录$2:备份文件名，绝对路径;$3:模式，master或者slave，信息存放的路径为备份文件的路径/backupFileRecordName
function addMasterBackupFileInfo()
{
    backup_dir=$1
    file_name=$2
    #获取mysql当前data的目录
    mysql_data_dir=$(cat /etc/my.cnf|grep "datadir"| awk -F "=" '{print $2}' | sed 's/[ ]//g')
	master_log_info=`mysql -u$mysql_user -p$mysql_password  -e "show master status\G"`
    master_log_file_name=$(echo $master_log_info | awk -F "File: " '{print $2}' | awk -F " " '{print $1}')
	master_log_pos=$(echo $master_log_info | awk -F "Position: " '{print $2}' | awk -F " " '{print $1}')
	master_md5_sum=$(md5sum $file_name | awk -F " " '{print $1}')
	
	num=`cat $backup_dir/$backupFileRecordName|wc -l`
	if [ $num -eq 0 ];then
		rm -rf $backup_dir/$backupFileRecordName
		logInfo "rm $backup_dir/$backupFileRecordName"
	fi

	#每次记录都插入第1行,虽然binlog用的是相对路径，可能重复安装binlog会重掉，但是重复安装uuid肯定不会一样，靠文件唯一来保证
	if [[ ! -f "$backup_dir/$backupFileRecordName" ]] || [[ -z `cat $backup_dir/$backupFileRecordName` ]] ;then
		echo "$file_name $mysql_data_dir/$master_log_file_name $master_log_pos $master_md5_sum" > $backup_dir/$backupFileRecordName
	else
		sed -i '1i\'$file_name' '$mysql_data_dir/$master_log_file_name' '$master_log_pos' '$master_md5_sum'' $backup_dir/$backupFileRecordName
	fi
    #echo "$file_name $mysql_data_dir/$master_log_file_name" >> $backup_dir/$backupFileRecordName
    logInfo "add record($file_name, $mysql_data_dir/$master_log_file_name, $master_log_pos $master_md5_sum) to $backup_dir/$backupFileRecordName"
}

function purgeMasterFile()
{
	file_dir=$1
    uuid=$2
    server_id=$3
	reserve_files=`cat $file_dir/$backupFileRecordName|awk -F ' ' '{print $1}' | awk -F '/' '{print $NF}'`
    logInfo "reserve file: $file_dir, $reserve_files"
    #删除真正的文件，需要出去当前正在备份的tmp文件
    cd $file_dir
    all_files=`ls $file_dir  |grep "master" |grep "$uuid" |grep "$server_id"|grep -v "tmp"`
    for file in $all_files;
    do
        flag=0
        for reserve_file in $reserve_files;
        do
            if [[ "$file" = "$reserve_file" ]];then
                flag=1
                break
            fi
        done
        if [[ $flag -eq 0 ]];then
            rm $file_dir/$file
			#删除对应的log文件
			backup_date=$(echo $file | awk -F "${server_id}" '{print $2}' | tr -d ".gz")
			backup_log=$file_dir/backup${backup_date}.log
			rm -rf $backup_log
            logInfo "rm backup:$file_dir/$file and log:$backup_log"
        fi
    done
	logInfo "master rm backup success"
}

function purgeMasterFileWithName()
{
    file_dir=$1
    uuid=$2
    server_id=$3
    file_name=$4
    logInfo "start to remove backup file name($file_name)"

    # 先删除backFileRecord文件中的记录，防止误拿
    sed -i '/'$file_name'/,$d'  $file_dir/$backupFileRecordName
    purgeMasterFile $file_dir $uuid $server_id
}

#backup文件个数进行校验，$1:文件目录；$2:检测的文件标记，master slave
function purgeMasterFileWithNum()
{
    file_dir=$1
    uuid=$2
    server_id=$3
    num=$4
 
    current_num=`ls -At $file_dir |grep "master" |grep "$uuid" |grep "$server_id" | wc -l`
	logInfo "backup file num is $num, current_num is $current_num"
    if [[ $current_num -le $num ]];then
        return;
    fi

	# 获需要删除的备份文件名，存放在全局变量g_backup_todel_files中
	#files=`ls -At $file_dir |grep "master" |grep "$uuid" |grep "$server_id"` # 文件从新到旧排序，删新保旧
	#computePurgeBackupFileName "$files" $num # $files作为数组传递，中间有空格，所以需要用双引号包起来
	#logInfo "all todo remove files:  ${g_backup_todel_files[*]}"

	# 先删除backFileRecord文件中的记录，防止误拿
	#for todelfile in ${g_backup_todel_files[*]}
	#do
	    #echo "remove backup file name($todelfile) from backFileRecord"
		#logInfo "remove backup file name($todelfile) from backFileRecord"
		# sed命令根据文件名来删除不好用啊，因为todelfile变量里包含/字符。还是根据行号来删除吧
		#sed -i "/$todelfile/$d"  $file_dir/$backupFileRecordName

		#lineno=$(grep -n $todelfile  $file_dir/${backupFileRecordName} | awk -F: '{print $1}')
		#if [[ -n $lineno ]]; then
			#sed -i "${lineno}d" $file_dir/${backupFileRecordName}
		#fi

	#done

	 #先删除记录，防止误拿
    let start_num=$num+1
	logInfo "start_num $start_num $file_dir/$backupFileRecordName"
    sed -i ''$start_num',$d'  $file_dir/$backupFileRecordName
	#删除真正的文件
    purgeMasterFile $file_dir $uuid $server_id
}

function purgeSlaveBackupFile()
{
    file_dir=$1
    uuid=$2
    server_id=$3
    num=`cat $CFG_FILE | grep -w "backup_num" | awk -F": |,|/*" '{print $2}'`
    current_num=`ls -At $file_dir |grep "slave" |grep "$uuid" |grep "$server_id" | wc -l`
	logInfo "backup file num is $num, current_num is $current_num"
    if [[ $current_num -le $num ]];then
        return;
    fi

	# 获需要删除的备份文件名，存放在全局变量g_backup_todel_files中
	#files=`ls -At $file_dir |grep "slave" |grep "$uuid" |grep "$server_id"`  # 文件从新到旧排序，删新保旧
	#computePurgeBackupFileName "$files" $num # $files作为数组传递，中间有空格，所以需要用双引号包起来
	#logInfo "all todo remove files:  ${g_backup_todel_files[*]}"

	#删除真正的文件
    # for file in ${g_backup_todel_files[*]}
    # do
		#rm $file_dir/$file
		#删除对应的log文件
		#backup_date=$(echo $file | awk -F "${server_id}" '{print $2}' | tr -d ".gz")
		#backup_log=$file_dir/backup${backup_date}.log
		#rm -rf $backup_log
		#echo "rm backup:$file_dir/$file and log:$backup_log"
		#logInfo "rm backup:$file_dir/$file and log:$backup_log"
    #  done
	
	#删除真正的文件
    let rm_num=$current_num-$num
    cd $file_dir
    ls -lrt $file_dir |grep "slave" |grep "$uuid" |grep "$server_id" | head -${rm_num}|awk '{print $NF}'|xargs rm -rf
    logInfo "slave rm backup success $rm_num"
	logInfo "slave rm backup success"
}

#保存和清理auto自动备份路径下的历史备份文件
#$1:备份文件路径，$2:uuid，$3:serverid;$4:xtrabackup：表示备份任务失败，需要xtrabackup对数据库进行物理备份；dump：拷贝最新备份文件路径到auto下面
function purgeAutobackupFile()
{
	#常规备份路径
	file_dir=$1
	#历史自动备份路径
	auto_file_dir=$file_dir/auto
	
	#检查历史备份路径是否存在
	checkDir $auto_file_dir
    
    uuid=$2
    server_id=$3
    
    #获取配置文件中的备份日期
    backup_date=`cat $CFG_FILE | grep -w "auto_backup_date" | awk -F": |,|\"" '{print $5}'`
    OLD_IFS="$IFS"
    IFS="-"
    backup_dates=($backup_date)
    IFS="$OLD_IFS"
    
    local cur_date=`date +%d`
 
	#检测当前日期是否在配置文件中
	flag=0
    for date_num in ${backup_dates[@]};
    do
    	if [[ "$cur_date" == "$date_num" ]];then
		flag=1
		break
		fi
	done
    
    #当前日期不在配置文件中，则退出
    if [[ $flag == "0" ]];then
    return;
	fi
   
     #如果参数表示之前零点的备份是失败的，意味着没有最新的备份文件，则用xbackup生成一个，且标注是xbackup
	if [[ "$4" = "xtrabackup" ]];then
	
 	#dump备份失败的话，使用xtrabackup进行物理备份
	backupHistoryFileWithXtra $1 $uuid $server_id
	#根据自动备份的个数删除掉多余的备份文件
	deleteAutoBackupFiles $auto_file_dir
	return;
	
	fi
    
    local cur_can=`date +%Y_%m_%d`
    #如果之前备份任务是备份成功的，则直接把常规备份文件下的文件拷贝到auto目录即可
	new_backup_file=`ls -at $1|grep "auto"|grep "$uuid" |grep "$server_id"|grep "${cur_can}"|head -1`
	cp $1/$new_backup_file ${auto_file_dir}/
	
	#根据自动备份的个数删除掉多余的备份文件
	deleteAutoBackupFiles $auto_file_dir
}

#生成xtrabackup文件
function backupHistoryFileWithXtra(){
	#常规备份路径
	file_dir=$1
	#历史自动备份路径
	auto_file_dir=$file_dir/auto
	
	uuid=$2
    server_id=$3
    
	getUserCfg
    logInfo "auto start to back up with xtrabackup ,file_dir:$auto_file_dir,uuid:$uuid,server_id:$server_id";
	
	XTRABACKUP_HOME=/cloud/dahua/DataBaseServer/xtrabackup
	PATH=$PATH:$XTRABACKUP_HOME/bin
	export PATH XTRABACKUP_HOME
	source /etc/profile
	
	rm -rf $auto_file_dir/backup.log
	innobackupex --user=$mysql_user --password=$mysql_password --throttle=$MYSQL_BACKUP_THROTTLE --no-lock \
		--stream=tar "$auto_file_dir" 2>$auto_file_dir/backup.log | gzip > "$auto_file_dir/auto_master_${uuid}_${server_id}_xtrabackup_tmp"
	if [ $? != 0 ];then
		log_info $LOG_FILE "backup_mysql with xtrabackup failed!"
		return 1
	fi
   
    logInfo "end to back up with xtrabackup ,file_dir:$auto_file_dir,uuid:$uuid,server_id:$server_id"
    date_info=$(date "+%Y_%m_%d_%H%M%S")
    new_file_name=${auto_file_dir}/auto_master_${uuid}_${server_id}_${date_info}_xtrabackup.gz
    mv ${auto_file_dir}/auto_master_${uuid}_${server_id}_xtrabackup_tmp $new_file_name
    return 0
}

#根据自动备份的个数删除掉多余的备份文件
function deleteAutoBackupFiles(){
	file_dir=$1
	num=`cat $CFG_FILE | grep -w "auto_backup_num" | awk -F": |,|/*" '{print $2}'`
    current_num=`ls -At $file_dir |grep auto | grep "$uuid" |grep "$server_id" | wc -l`
    if [[ $current_num -le $num ]];then
        return;
    fi
	let rm_num=$current_num-$num
    cd $file_dir
	ls -lrt $file_dir|grep "$uuid" |grep "$server_id" | head -${rm_num}|awk '{print $NF}'|xargs rm -rf
    logInfo "auto path rm backup success $rm_num"
	logInfo "auto path rm backup success"
}

# 从备份文件名中解析出utc时间戳。参数：$1:文件名
function parseTimeFromBackupFileName()
{
	filename=$1
	
	# 文件名格式：${auto}_${mode}_${uuid}_${server_id}_$(date "+%Y_%m_%d_%H%M%S").gz
	# 比如：manual_master_1DC384B733A14D498E598825BB545152_2206879_2018_09_11_164624.gz
	
	# 解析为：20180911164624
	timestr=$(echo $filename | awk -F'.' '{ printf $1 }' | awk -F'_' '{ printf $5$6$7$8 }')
	# 转换为：2018-09-11 16:46:24
	timestr_fmt="${timestr:0:4}-${timestr:4:2}-${timestr:6:2} ${timestr:8:2}:${timestr:10:2}:${timestr:12:2}";
	# 转换为：1536655584
	timesecond=$(date -d "$timestr_fmt" +%s)
	
	echo $timesecond
}

# 根据文件名计算需要删除的文件名。参数：$1:备份文件列表；$2:需要保留的备份数
# 需要保留最新的一个备份，所以实际上保留的备份数会多一个
function computePurgeBackupFileName()
{
	files=$1
	backup_num=$2 #`cat $CFG_FILE | grep -w "backup_num" | awk -F": |,|/*" '{print $2}'`
	
	declare -a files_arr=($files)
	files_num=${#files_arr[@]}
	let todel_num=$files_num-$backup_num
	
	#echo "all files: " $files
	#echo "files_num: " $files_num
	#echo "backup_num: " $backup_num
	
	# 需要删除的文件数量<=0，直接返回
	if [[ $todel_num -le 0 ]];then
		return 0
	fi

	# 构造保留区间数组，备份最小保留4天内，间隔为4天。构造后的数组形如: [ 4, 8, 12 ]，数组大小和备份数量相同。
	# 从而划分出n+1个区间，最后一个区间不保留文件，形如：(, 4] (4, 8] (8, 12] (12, )
	# 另外需要保存最新的一个备份，所以一共保留n+1个备份
	declare -a backup_interval
	backup_interval[0]=4
	for((i=1; i<$backup_num; i++))
	do
		let backup_interval[$i]=${backup_interval[$i-1]}+4
	done
	
	#echo "backup_interval: ${backup_interval[@]}"
	logInfo "backup_interval: ${backup_interval[@]}"

	declare -a backup_reserve_files # 保留文件的数组，一个位置只保存一个保留区间内的文件。
	declare -a backup_todel_files   # 待删除文件的数组。不分区间，保存所有待删除的文件
	
	# 最新的文件，必须要保留的。如果最小区间有多个文件，那么这个新的文件就是额外多出来的一个
	lasted_file=${files_arr[0]}
	nowsecond=$(date +%s)

	# 一个一个文件循环处理，判断是要保留还是要删除
	for filename in ${files_arr[@]}
	do
		if [[ $todel_num -le ${#backup_todel_files[@]} ]]; then
			#echo "todel file enough"
			break;
		fi
		
		filesecond=$(parseTimeFromBackupFileName ${filename})
		
		if [[ -z $lasted_file ]]; then
			lasted_file=$filename
		fi
		lasted_second=$(parseTimeFromBackupFileName ${lasted_file})
		if [[ $filesecond -gt $lasted_second ]];then
			lasted_file=$filename
		fi

		let age=($nowsecond-$filesecond)/3600/24 # 换算成天
		#let age=($nowsecond/60-$filesecond/60) # 测试，分钟级别，忽略秒
		
		for((i=0; i<$backup_num; i++))
		do
			#found_index=0 #标记是否找到对应的区间
			backup_reserve_files_index=-1	 #标记找到的对应区间的索引，-1表示没找到
			if [[ $age -le ${backup_interval[0]} ]]; then
			# 很新的文件，在最小的保留区间内
				let backup_reserve_files_index=0
				reserve_file="${backup_reserve_files[0]}"
			elif [[ $age -gt ${backup_interval[$i]} ]] && [[ $age -le ${backup_interval[$i+1]} ]]; then
			# 稍微旧一些的文件，在前开后闭的保留区间内
				let backup_reserve_files_index=$i+1
				reserve_file="${backup_reserve_files[$i+1]}"
			elif [[ $age -gt ${backup_interval[$backup_num-1]} ]]; then
			# 太旧的文件，需要删除，直接添加到待删除列表
				backup_todel_files=( ${backup_todel_files[@]} $filename )
				break; # 跳出循环，不能执行下面的语句
			fi

			# 找到了对应的保留区间，判断是要保留还是要删除
			if [[ $backup_reserve_files_index -ge 0 ]]; then
				if [[ -z "${reserve_file}" ]]; then
					# 该区间还没有记录的保留文件，那么就记录这个
					backup_reserve_files[$backup_reserve_files_index]=$filename
				else
					# 该区间内已经有记录，替换成比较旧的（不能保留最新的，否则备份永远都是新的）
					reserve_filesecond=$(parseTimeFromBackupFileName ${reserve_file})
					if [[ $filesecond -lt $reserve_filesecond ]]; then
						backup_reserve_files[$backup_reserve_files_index]=$filename
						backup_todel_files=( ${backup_todel_files[@]} $reserve_file )
					else
						backup_todel_files=( ${backup_todel_files[@]} $filename )
					fi
				fi
				
				break; # 跳出来，跳出来。一个文件必然只落在一个区间内
			fi

		done # 区间循环

	done # 文件循环

	#echo "backup_todel_files: " ${backup_todel_files[@]}

	# 需要保留最新的文件，从待删除数组中清除掉
	if [[ "${lasted_file}" != "${backup_reserve_files[0]}" ]]; then
		#echo "to del lasted_file from backup_todel_files: " ${lasted_file}
		for (( i=0; i < ${#backup_todel_files[@]}; i++ ))
		do
			if [[ "${lasted_file}" = ${backup_todel_files[$i]} ]]; then
				unset backup_todel_files[$i]
			fi
		done

		# 重构数组，去除空洞
		backup_todel_files=( "${backup_todel_files[@]}" )
	fi
	
	# 待删除列表赋值给全局变量，方便调用方获取该值（相当于为该函数的返回值）
	g_backup_todel_files=${backup_todel_files[@]}
	
	#echo "backup_reserve_files: " ${backup_reserve_files[@]}
	#echo "g_backup_todel_files: " ${g_backup_todel_files[@]}
}

function setEvent()
{
	option=$1
	
	getIgnoreDB
	getUserCfg
	databases=(`mysql -u$mysql_user -p$mysql_password -e "select EVENT_SCHEMA from information_schema.Events" -N 2>/dev/null`)
	events=(`mysql -u$mysql_user -p$mysql_password -e "select EVENT_NAME from information_schema.Events" -N 2>/dev/null`)
	num=${#databases[@]}
	for((i=0; i<$num; i++))
	do
		need_ignore=`echo $all_ignore_dbs | grep -c ${databases[$i]}`
		if [ $need_ignore -gt 0 ];then
			cmd="alter event ${events[$i]} enable;"
			logInfo "ignoreDB:$cmd"
			mysql -u$mysql_user -p$mysql_password ${databases[$i]} -e "$cmd"
			continue
		fi
		
		cmd="alter event ${events[$i]} $option;"
		logInfo "$cmd"
		mysql -u$mysql_user -p$mysql_password ${databases[$i]} -e "$cmd"
	done
}

function checkDB()
{
	DBName=$1
	findName=`cat $mysql_config |grep -w "$ignore_cfg=$DBName"`
	if [ "$findName" != "" ];then
		return 0
	else
		return 1
	fi
}

function checkDBEvent()
{
	DBName=$1
	option=$2
    getUserCfg
	databases=(`mysql -u$mysql_user -p$mysql_password -e "select EVENT_SCHEMA from information_schema.Events" -N 2>/dev/null`)
	events=(`mysql -u$mysql_user -p$mysql_password -e "select EVENT_NAME from information_schema.Events" -N 2>/dev/null`)
	num=${#databases[@]}
	for((i=0; i<$num; i++))
	do
		exists=`echo $DBName | grep -c ${databases[$i]}`
		if [ $exists -gt 0 ];then
			cmd="alter event ${events[$i]} $option;"
			logInfo "$cmd"
			mysql -u$mysql_user -p$mysql_password ${databases[$i]} -e "$cmd"
		fi
	done
	return 0
}

#添加ignore db时为保证DBHA主不重启，需要将过滤信息动态下发到主机上
#需要考虑设置部分ignore db/完全未设置的场景
function addIgnoreDBDynamic()
{
	DBNames=$1
	if [ "$DBNames" == "" ];then
		logInfo "ignore DBName is empty."
		return 1
	fi
	
    getUserCfg
	#获取DBHA的虚IP，在虚IP上下发配置即可；slave后续会重启
	virtual_ip=`cat $CONFIG_PATH |grep "DBHA_virtual_IP\"" |awk -F "\"" '{print$4}'`
	mysqladmin -u$mysql_user -p$mysql_password -h$virtual_ip --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
	if [ $? -ne 0 ];then
		logInfo "$virtual_ip not exist,ignore addIgnoreDBDynamic operation"
		return 1
	fi
	
	local change_repl_wild_table_cmd="CHANGE REPLICATION FILTER REPLICATE_WILD_IGNORE_TABLE=("
	local change_repl_ignore_db_cmd="CHANGE REPLICATION FILTER REPLICATE_IGNORE_DB=("
	
	#读/etc/my.cnf将已有的replicate-ignore-db优先设置起来
	local exist_wild_db=$(cat $mysql_config |grep "replicate-ignore-db" | awk -F "=" '{print $2}')
	tmpIsFirstDB=0
	has_exist_wild_db=0
	for db in ${exist_wild_db[@]};do
		if [ $tmpIsFirstDB -eq 0 ];then
			change_repl_ignore_db_cmd=$change_repl_ignore_db_cmd"$db"
			tmpIsFirstDB=1
		else
			change_repl_ignore_db_cmd=$change_repl_ignore_db_cmd", $db"
		fi
		has_exist_wild_db=1
	done
	
	#读/etc/my.cnf将已有的replicate-wild-ignore-table优先设置起来
	local exist_wild_table=$(cat $mysql_config | grep "replicate-wild-ignore-table" | awk -F "=" '{print $2}')
	tmpIsFirstDB=0
	has_exist_wild_table=0
	for wild_table in ${exist_wild_table[@]};do
		if [ $tmpIsFirstDB -eq 0 ];then
			change_repl_wild_table_cmd=$change_repl_wild_table_cmd"'$wild_table'"
			tmpIsFirstDB=1
		else
			change_repl_wild_table_cmd=$change_repl_wild_table_cmd", '$wild_table'"
		fi
		has_exist_wild_table=1
	done
	
	local ALL_DBNAME=`echo $DBNames | sed -n 's/,/ /g;p'`
	tmpIsFirstDB=0
	for DBName in ${ALL_DBNAME[@]};do
		checkDB $DBName
		if [ $? -ne 0 ];then
			if [ $has_exist_wild_db -eq 0 ] && [ $tmpIsFirstDB -eq 0 ];then
				change_repl_ignore_db_cmd=$change_repl_ignore_db_cmd"$DBName"
			else
				change_repl_ignore_db_cmd=$change_repl_ignore_db_cmd", $DBName"
			fi
			
			if [ $has_exist_wild_table -eq 0 ] && [ $tmpIsFirstDB -eq 0 ];then
				change_repl_wild_table_cmd=$change_repl_wild_table_cmd"'$DBName.%'"
			else
				change_repl_wild_table_cmd=$change_repl_wild_table_cmd", '$DBName.%'"
			fi
			
			tmpIsFirstDB=1
		fi
	done
	
	change_repl_wild_table_cmd=$change_repl_wild_table_cmd")"
	change_repl_ignore_db_cmd=$change_repl_ignore_db_cmd")"
	if [ $tmpIsFirstDB -eq 1 ];then
		logInfo "master set wild table:$change_repl_wild_table_cmd, master set ignore db:$change_repl_ignore_db_cmd"
		mysql -u$mysql_user -p$mysql_password -h$virtual_ip -e "$change_repl_wild_table_cmd"
		mysql -u$mysql_user -p$mysql_password -h$virtual_ip -e "$change_repl_ignore_db_cmd"
	fi
	return 0
}

function addIgnoreDB()
{
	DBNames=$1
	if [ "$DBNames" == "" ];then
		logInfo "ignore DBName is empty."
		return 1
	fi
	
	#先在主机上下发配置信息
	addIgnoreDBDynamic $DBNames
	
	local needSync=0
	local ALL_DBNAME=`echo $DBNames | sed -n 's/,/ /g;p'`
	for DBName in ${ALL_DBNAME[@]};do
		checkDB $DBName
		if [ $? -eq 0 ];then
			logInfo "$DBName is exist in ignore DBs."
		else
			logInfo "add $DBName to $mysql_config."
			sed -i -c '/\[mysqld\]/a\'$ignore_cfg'='$DBName'' $mysql_config
                        sed -i -c '/\[mysqld\]/a\'$ignore_wild_cfg'='$DBName.%'' $mysql_config
			needSync=1
		fi
		
		#修改数据库对应事件为enable
		checkDBEvent $DBName enable
	done
    
    #只要本机不是主，也需要重启
	master_ret=`/etc/init.d/DB-Manager detail | grep SQLMode| awk -F ":" '{print $2}'`
    /etc/init.d/DB-Manager status
	ret_status=$?
	if [ $ret_status -eq 0 ] && [ $master_ret -ne 1 ] && [ $needSync -eq 1 ];then
		logInfo "restart DBHA server of self slave."
        /etc/init.d/DB-Manager restart
	else
		logInfo "DBHA server of self no need to restart."
	fi

	#配置文件无修改则不需要同步
	#if [ $needSync -eq 1 ];then
	#    synMysqlCfg
	#    if [ $? -ne 0 ];then
	#        return 1
	#    fi
	#fi

	return 0
}

function delIgnoreDB()
{
	DBNames=$1
	if [ "$DBNames" == "" ];then
		logInfo "ignore DBName is empty."
		return 1
	fi

    local needSync=0
	local ALL_DBNAME=`echo $DBNames | sed -n 's/,/ /g;p'`
	for DBName in ${ALL_DBNAME[@]};do
		checkDB $DBName
		if [ $? -ne 0 ];then
			logInfo "$DBName is not exist in ignore DBs."
		else
			logInfo "delete $DBName to $mysql_config."
			sed -i -c '/^'$ignore_cfg'='$DBName'$/d' $mysql_config
            		sed -i -c '/^'$ignore_wild_cfg'='$DBName.%'$/d' $mysql_config
            needSync=1
		fi
		
		#修改数据库对应事件为enable
		checkDBEvent $DBName disable
	done
    
    #只要本机不是主，也需要重启
	master_ret=`/etc/init.d/DB-Manager detail | grep SQLMode| awk -F ":" '{print $2}'`
    /etc/init.d/DB-Manager status
	ret_status=$?
	if [ $ret_status -eq 0 ] && [ $master_ret -ne 1 ] && [ $needSync -eq 1 ];then
		logInfo "restart DBHA server of self slave."
        /etc/init.d/DB-Manager restart
	else
		logInfo "DBHA server of self no need to restart."
	fi

    #配置文件无修改则不需要同步
	#if [ $needSync -eq 1 ];then
    #    synMysqlCfg
    #    if [ $? -ne 0 ];then
    #        return 1
    #    fi
    #fi

	return 0
}

function checkSingleMode()
{
	peer_business_IP=`cat $CONFIG_PATH |grep "peer_business_IP"|awk -F "\"" '{print $4}'`
	local_business_IP=`cat $CONFIG_PATH |grep "local_business_IP"|awk -F "\"" '{print $4}'`
	if [ "$peer_business_IP" == "" ] || [ "$peer_business_IP" == "$local_business_IP" ];then
		logInfo "attention:single mode."
		return 0
	fi

	return 1
}

#默认单机模式下就是传送成功
function syncSlaveFile()
{
	dir=$1
	file_name=$2

	checkSingleMode
	if [ $? -eq 0 ];then
		return 0
	fi

	logInfo "scp $file_name to $peer_business_IP:$dir"
	scp $file_name root@$peer_business_IP:$dir/
	if [ $? -eq 0 ];then
		logInfo "scp $file_name successed to $peer_business_IP:$dir"
		return 0;
	else
		logInfo "scp $file_name failed to $peer_business_IP:$dir"
		return 1;
	fi
}

#0是备机，1不是备机
function checkSlave()
{
	checkSingleMode
	if [ $? -eq 0 ];then
		return 1
	fi
	
	#根据本机的状态判断对端的状态，本机是主对端是备，本机是备对端是主
	ret=`/etc/init.d/DB-Manager detail | grep SQLMode| awk -F ":" '{print $2}'`
	if [ $ret -eq 1 ];then
		return 0
	else
		return 1
	fi
}

#默认单机模式下就是执行成功
function syncSlaveCmd()
{
	cmd=$1

	checkSingleMode
	if [ $? -eq 0 ];then
		return 0
	fi

	ssh root@$peer_business_IP  $cmd
	if [ $? -eq 0 ];then
		logInfo "execute cmd successed: ssh root@$peer_business_IP  $cmd"
		return 0;
	else
		logInfo "execute cmd failed: ssh root@$peer_business_IP  $cmd"
		return 1;
	fi
}

function synMysqlCfg()
{
	#拷贝文件
	syncSlaveFile /etc/ $mysql_config
	if [ $? -ne 0 ];then
		return 1
	fi

	#查看当前db状态，如果开启了，则重启服务；如果没有开启则跳过
	syncSlaveCmd "/etc/init.d/DB-Manager status"
	ret_status=$?
	checkSlave
	is_slave=$?
	if [ $ret_status -eq 0 ] && [ $is_slave -eq 0 ];then
		logInfo "restart DBHA server of slave."
		syncSlaveCmd "/etc/init.d/DB-Manager restart"
		if [ $? -ne 0 ];then
			logInfo "change slave($peer_business_IP) failed."
			return 1
		fi
	else
		logInfo "DBHA server of slave($peer_business_IP) is not started or is master."
	fi

	return 0
}

function changeBackupDir()
{
	#单机情况下默认是成功返回
	checkSingleMode
	if [ $? -eq 0 ];then
		return 0
	fi

	slave_backup_path=$1
	# 获取历史备份路径并且与新路径进行比较，不同则修改，相同退出
	local slave_old_backup_dir=`ssh $peer_business_IP cat ${CONFIG_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
	
	if [ "$slave_old_backup_dir" == "" ];then
		logInfo "salve old mysql_backup_path is null"
		return 1
	fi
	
	if [ "$slave_backup_path" == "" ];then
		logInfo "slave mysql_backup_path is empty"
		return 1
	fi
	
	local slave_old_backup_path=$(echo $slave_old_backup_dir | sed 's/\\//g')
	if [ "$slave_old_backup_path" == "$slave_backup_path" ];then
		logInfo "slave old mysql_backup_path and new mysql_backup_path is the same "
		return 0
	fi
	
	logInfo "slave mysql_backup_path is $slave_backup_path"
	echo "slave mysql_backup_path is $slave_backup_path"
	
	#修改路径之后，把历史路径下面的备份文件拷贝到新的路径下面
	syncSlaveCmd "[ ! -d $slave_backup_path ] && mkdir $slave_backup_path" 
	syncSlaveCmd "mv $slave_old_backup_dir/* $slave_backup_path/"
	
	syncSlaveCmd "$DBHA_BIN/$CONFIG_TOOL $CONFIG_PATH  '{\"mysql_backup_path\" : \"$slave_backup_path\"}'"
	
	#TODO 重启DBHA服务
	syncSlaveCmd "/etc/init.d/DB-Manager restart"
	
	return $?
}

