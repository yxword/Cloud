#!/bin/sh
#description: DBHA server monitor
# made by mjj 24182 2016-12-8
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh

BITS=
SYS=`uname -m`
if [ $SYS == "x86_64" ] || [ $SYS == "aarch64" ];then
    BITS=64
fi

# 独立服务文件夹
CONFIG_PATH=$DBHA_CONFIG/DBHA.conf
TAG=DBHAKeeper

####################################需根据服务配置#########################################
BIN_NAME=DBHAServer$BITS

#mysql服务名
MYSQL_SERVICE=${DBHA_SHELL}/Service/MysqlServer.sh

# 日志转存、压缩脚本声明
LOG_CYCLE_SYNC_NAME=CycleSyncDBHALog.sh
CYCLE_LOG=$DBHA_SHELL/Common/$LOG_CYCLE_SYNC_NAME

#####################################################################
dbha_install_mode=$(/etc/init.d/DB-Manager detail | grep install | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}' | sed 's/[ ]//g')
mysql_cfg=/etc/my.cnf
#mysql登录的用户名和密码
getUserCfg

# 查文件是否齐全
function checkFile()
{
	if [ ! -f $DBHA_BIN/$BIN_NAME ];then
		echo "server bin doesn't exist, path=$DBHA_BIN/$BIN_NAME"
		return 1
	fi

	return 0
}

function monitorVip()
{
    #双机和单机部署的情况都需要考虑，单机部署时虚IP可能和业务口一个IP地址
    server_pid=`pidof $BIN_NAME`
    is_master=`cat $DBHA_CONFIG/Sql.stat | grep -w "SQL_mode" | awk -F ": " '{print $2}' | awk -F ",|/*" '{print $1}'`
	vip_if_name=`cat $CONFIG_PATH | grep DBHA_vif_name | awk -F "\"" '{printf $4}'`
    business_if_name=`cat $CONFIG_PATH | grep DBHA_if_name | awk -F "\"" '{printf $4}'`
    vip=`cat $CONFIG_PATH | grep \"DBHA_virtual_IP\" | awk -F "\"" '{printf $4}'`
    if [ ! -z "$server_pid" ] && [ $is_master -eq 1 ];then
        local local_vip=` ip addr show dev ${vip_if_name} | grep 'inet ' | grep 'brd' | grep "${vip_if_name}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
        local local_business_ip=` ip addr show dev ${business_if_name} | grep 'inet ' | grep 'brd' | grep "${business_if_name}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
        if [ -z "$local_vip" ] && [ "$local_business_ip"X != "$vip"X ];then
            logInfo "DBHA master but vip gone!"
        fi
    fi
}

function checkServer()
{
	server_pid=`pidof $BIN_NAME`
	if [ -z "$server_pid" ];then
		logInfo "$BIN_NAME is not exist, try to start while checkServer"
		#优先拉起mysql
		sh $MYSQL_SERVICE status
		if [ $? -ne 0 ];then
			sh $MYSQL_SERVICE start
		fi
		chmod 755 ./$BIN_NAME
		#在启动DBHA服务之前强制杀死当前所有的进程
		killall $BIN_NAME
		nohup ./$BIN_NAME > /dev/null 2>&1 &
		logInfo "$BIN_NAME is started while checkServer."
	fi
    
    monitorVip
}

function check_mysql_bin_index()
{
	sh $MYSQL_SERVICE status
	if [[ $? -eq  0 ]];then
        return 0
    fi
    
    local mysql_path=$(grep datadir /etc/my.cnf | awk -F= '{print $2}' | awk '{print $1}')
    if [ -f $mysql_path/mysql-bin.index ];then
        local binlog_name=`ls -tr $mysql_path | grep "mysql-bin.[0-9]*" | grep -v "mysql-bin.index"`
        local binlog_num=`cat $mysql_path/mysql-bin.index | wc -l `
        
        while [ $binlog_num -gt 0 ] ;do
            local binlog_file=`tail -n 1 $mysql_path/mysql-bin.index | head -n 1 | awk -F '/' '{print $2}'`
            local need_remove=`echo $binlog_name | grep $binlog_file | wc -l`
            sh $MYSQL_SERVICE status
            mysql_dead=$?
            if [ $need_remove -eq 0 ] && [ $mysql_dead -ne 0 ];then
                sed -i "/$binlog_file/d" /data/mysql/data/mysql-bin.index
                try_restart_mysql=1
                redirect "csdriver.sh : remove $binlog_file from mysql-bin.index!"
            else
                break
            fi
            
            binlog_num=`cat $mysql_path/mysql-bin.index | wc -l `
        done
    fi
    
    sh $MYSQL_SERVICE status
    mysql_dead=$?
	if [ $try_restart_mysql == 1 ] && [ $mysql_dead -ne 0 ];then
		sh $MYSQL_SERVICE start
	fi
	return 0
}

#0无需检测，1需要检测
function need_do_check()
{
    #DBHA在恢复和备份时无需进行数据库检测
    #获取恢复脚本进程号
    recover_pid=`ps -ef | grep "Recovery"| grep "start"|grep -v grep | awk '{print $2}'`
    backup_pid=`ps -ef | grep "mysqldump"|grep -v grep | awk '{print $2}'`
    if [[ "$recover_pid" != "" ]] || [[ "$backup_pid" != "" ]];then
        redirect "recover:$recover_pid, backup:$backup_pid"
        return 0
    fi
    
    return 1
}

function check_mysql_health_status()
{
    sh $MYSQL_SERVICE status
	if [[ $? -ne  0 ]];then
        return 0
    fi
    
    need_do_check
    if [[ $? != 0 ]];then
        # 大于1200秒的事物会被kill掉
        python $DBHA_SHELL/Service/MysqlHealthCheck.py -u$mysql_user -p$mysql_password -H localhost -w 1200 -f $DBHA_LOG/mysql_health_check.log
        if [ $? != 0 ];then
            redirect "no mysqldb module installed, install it"
        fi
    fi
}

function checkMysql()
{	
	#如果mysql不在，则自杀框架，
	sh $MYSQL_SERVICE status
	if [[ $? -ne 0 ]];then
	logInfo "mysql is error,while checkMysql."
		pid=`pidof $BIN_NAME`
		if [ ! -z "$pid" ];then
			kill -2 $pid
			logInfo "stop $pid($BIN_NAME) while checkMysql with the mysqld stopping."

			try_max_count=15
			for((try_count=1;try_count<=try_max_count;try_count++))
			do
				sleep 1
				# pid存在则一直检测，不存在跳出循环。仅仅1情况才为停止正常
				pid=`pidof $BIN_NAME`
				if [ -z "$pid" ];then
					break
				fi
			done

			if [ $try_count -gt $try_max_count ];then
				logInfo "kill $BIN_NAME by force while checkMysql."
				pid=`pidof $BIN_NAME`
				kill -9 $pid
				killall $BIN_NAME
			fi
		fi
	fi
}

function monitor()
{
	logInfo "start monitor"
	# 监控主进程
	cd $DBHA_BIN
	while true
	do
		checkMysql
		checkServer
        
        #DBHA已经部署过了才去做mysql相关的检测
        if [ $dbha_install_mode -eq 1 ];then
            check_mysql_bin_index
            check_mysql_health_status
        fi
        
		sleep 2;
		
		COUNT=$((COUNT+1))
		# 每20分钟检测日志是否要转储、压缩
		if [ ${COUNT} -ge 600 ]; then
			logpid=`pidof $LOG_CYCLE_SYNC_NAME`
			if [ -z "$logpid" ];then
				# 运行日志转储、压缩。$1:日志打印目录 $2:日志文件名称
				nohup $CYCLE_LOG /log/DB/DBHA/Log DBHA_log_ &
			fi
			COUNT=0
		fi
	done
	logInfo "stop monitor"
}

checkFile
if [[ $? -ne 0 ]];then
	exit 1
fi

monitor
exit 0

