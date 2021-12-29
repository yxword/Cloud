#!/bin/bash
#chkconfig: 2345 60 40
#description: DBHA server driver

# made by mjj 24182 2016-12-8
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)
BITS=64
SYS=`uname -m`
if [ $SYS == "x86_64" ] || [ $SYS == "aarch64" ];then
	BITS=64
else
	BITS=
fi

####################################需根据服务配置#########################################

# 服务名称
SERVER_NAME=DB-Manager

# 服务守护脚本名称
MONIT_SHELL=DBHAKeeper.sh

# 服务BIN名称
BIN_NAME=DBHAServer$BITS
source $parent_dir/Common/DBHAConfig.sh

#mysql服务检测
MYSQL_SERVICE=$DBHA_SHELL/Service/MysqlServer.sh

# 服务配置文件(含网卡信息的配置文件)
CONFIG_PATH=$DBHA_CONFIG/DBHA.conf
#主备ha的配置文件
HA_CONFIG=$DBHA_CONFIG/HAConfig.conf
#主备ha的状态文件
HA_STATE=$DBHA_CONFIG/HAState.stat

# 数据库服务状态配置文件
DBSTATUS_PATH=$DBHA_CONFIG/Sql.stat

# 业务网卡配置项名称
BUSINESS_VIF_KEY=DBHA_if_name

# 心跳网卡(仅仅管理节点需要填)
MG_HEARTBEAT_VIF=DBHA_heartbeart_if_name

# Datanode的chkconfig脚本
DN_CHKCONFIG=

CYCLE_LOG=$DBHA_SHELL/Common/CycleSyncLog.sh

source $DBHA_SHELL/Common/CommonFunc.sh
source $DBHA_SHELL/Service/CleanupInfo.sh
source $DBHA_SHELL/Service/DockerOper.sh

ENCRY_TOOL=$DBHA_BIN/SecurityTool64

#mysql登录的用户名和密码
getUserCfg

TAG=VCS-DBHA

service_name=DB-Manager
 
function is_systemd()
{
   if [ -f /usr/lib/systemd/systemd ] && [ X"$(pidof systemd)" == X"1" ]; then
       return 0
   fi
 
   return 1
}
 
function start_function()
{
    is_systemd
    if [ $? -eq 0 ]; then
        logInfo  "[start_function] before systemctl start  ${SERVER_NAME}.service."
        systemctl start ${SERVER_NAME}.service
        getServerStatus
        logInfo  "[start_function] after systemctl start  ${SERVER_NAME}.service."
    else
        startServer
    fi
}

function stop_function()
{
    is_systemd
    if [ $? -eq 0 ]; then
        logInfo  "[stop_function] before systemctl stop  ${SERVER_NAME}.service."
        systemctl stop ${SERVER_NAME}.service
        getServerStatus
        logInfo  "[stop_function] after systemctl stop  ${SERVER_NAME}.service."
    else
        stopServer
    fi
}

function restart_function()
{
    is_systemd
    if [ $? -eq 0 ]; then
        logInfo  "[restart_function] before systemctl stop  ${SERVER_NAME}.service."
        systemctl stop  ${SERVER_NAME}.service
        getServerStatus
        logInfo  "[restart_function] after systemctl stop  ${SERVER_NAME}.service."
        logInfo  "[restart_function] before systemctl start ${SERVER_NAME}.service"
        systemctl start ${SERVER_NAME}.service
        getServerStatus
        logInfo  "[restart_function] after systemctl start  ${SERVER_NAME}.service."
    else
        stopServer 
        startServer
    fi
}

#####################################################################
# 获取系统服务状态
function getServerStatus(){
	# 检查守护脚本和主进程
	keep_exist=`ps -ef | grep /bin/sh | grep $MONIT_SHELL | awk '{print $2}'`
	server_pid=`pidof $BIN_NAME`

	#守护脚本和主进程都在
	if [ ! -z "$keep_exist" ] && [ ! -z "$server_pid" ];then
		echo "$MONIT_SHELL : running" 
		echo "$SERVER_NAME      : running" 
		return 0
	# 守护脚本和主进程都不在
	elif [ -z "$keep_exist" ] && [ -z "$server_pid" ];then
		echo "$MONIT_SHELL : stopped" 
		echo "$SERVER_NAME      : stopped" 
		return 1
	# 守护脚本在，主进程不在
	elif [ ! -z "$keep_exist" ] && [ -z "$server_pid" ];then
		echo "$MONIT_SHELL : running" 
		echo "$SERVER_NAME      : stopped" 
		if [ -d /usr/lib/systemd/system ]; then 
			#centos7.7 返回为 0 
			# centos7.7 服务启动start返回 非0 ，会把keeper 都杀死 ，导致服务停止。所以7.7 系统如果keeper 启动成功认为服务启动成功
			return 0 
		else 
			return 2
		fi
	# 守护脚本不在，主进程在
	elif [ -z "$keep_exist" ] && [ ! -z "$server_pid" ];then
		echo "$MONIT_SHELL : stopped" 
		echo "$SERVER_NAME      : running" 
		return 2
	fi
	return 2
}

function startHA()
{
	sh $MYSQL_SERVICE start
	if [[ $? -ne 0 ]];then
		logInfo  "[startHA]mysqld start failed."
		return 1
	fi

	server_pid=`pidof $BIN_NAME`
	if [ -z "$server_pid" ];then
		#优先拉起mysql
		cd $DBHA_BIN
		chmod 755 ./$BIN_NAME
		nohup ./$BIN_NAME > /dev/null 2>&1 &
		logInfo  "[startHA]start $BIN_NAME success."
	fi
	
	try_max_count=15
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		server_pid=`pidof $BIN_NAME`
		if [ ! -z "$server_pid" ];then
			break
		fi
		logInfo  "[startHA]Wait $BIN_NAME start,wait times: $try_count."
		sleep 1
	done
	
	server_pid=`pidof $BIN_NAME`
	if [ -z "$server_pid" ];then
		logInfo "[startHA]$BIN_NAME was stopped"
		return 1
	fi

	return 0
}

# 从配置文件中获取虚网卡名称，然后down掉
stop_ha_vip()
{
	VIRTUAL_NETCARD_NAME=`grep -n DBHA_vif_name $CONFIG_PATH | awk -F '"' '{print $4}'`
	ifconfig $VIRTUAL_NETCARD_NAME down > /dev/null 2>&1
}

function stopHA()
{
    #down虚IP
    stop_ha_vip
    
	server_pid=`pidof $BIN_NAME`
	if [ ! -z "$server_pid" ];then
		kill -2 $server_pid
		logInfo "[stopHA]stop $BIN_NAME"
	fi
	#强制杀死
	server_pid=`pidof $BIN_NAME`
	if [ ! -z "$server_pid" ];then
		kill -9 $server_pid
		logInfo "[stopHA]stop $BIN_NAME by force"
	fi

	sh $MYSQL_SERVICE stop
	if [[ $? -ne 0 ]];then
		logInfo  "[stopHA]mysqld stop failed."
		return 1
	fi

	return 0
}

function stopHAForce()
{
	try_max_count=15
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		server_pid=`pidof $BIN_NAME`
		if [ ! -z "$server_pid" ];then
			kill -9 $server_pid
			logInfo "[stopHAForce]stop $BIN_NAME force"
			sleep 1
		else
			break
		fi
	done
}

function startKeep()
{
	# 检查守护脚本和主进程
	keep_exist=`ps -ef | grep /bin/sh | grep $MONIT_SHELL | awk '{print $2}'`

	#守护脚本和主进程都不在
	if [ -z "$keep_exist" ];then
		logInfo "[startKeep]try to start $MONIT_SHELL and $SERVER_NAME" 
		nohup /bin/sh $DBHA_SHELL/Service/$MONIT_SHELL > /dev/null 2>&1 &
	fi
}

function stopKeep()
{
	try_max_count=15
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		keep_exist=`ps -ef | grep /bin/sh | grep $MONIT_SHELL | awk '{print $2}'`
		if [ ! -z "$keep_exist" ];then
			kill -9 $keep_exist
			logInfo "[stopKeep]stop $MONIT_SHELL"
			sleep 1
		else
			break
		fi
	done
}

function addCycleLog()
{
	#每隔5分钟执行一次
	if [[ ! -f "/var/spool/cron/root" ]];then
		touch /var/spool/cron/root
	fi
	
	sed -i '/cycleSyncLog/d' /var/spool/cron/root
	#sed -i '$a*/5 * * * *  '$CYCLE_LOG' '$DBHA_LOG'' /var/spool/cron/root
	echo '*/5 * * * *  '$CYCLE_LOG' '$DBHA_LOG'' >> /var/spool/cron/root
}

function delCycleLog()
{
	sed -i '/cycleSyncLog/d' /var/spool/cron/root
}

# 启动系统服务
function startServer(){
	logInfo "[startServer]start server by command"
	ulimit -n 10240
	ulimit -c unlimited

	keep_exist=`ps -ef | grep /bin/sh | grep $MONIT_SHELL | awk '{print $2}'`
	#守护脚本和主进程都不在
	if [ ! -z "$keep_exist" ];then
		logInfo "[startServer]keep($keep_exist) is exist"
		getServerStatus
		return 0
	fi

	startHA
	startKeep
	#增加日志转储功能
	#addCycleLog
	
	getServerStatus
	[ $? -eq 0 ] || return 1
	return 0
}

#启动docker
function startDocker(){
	logInfo "[startDocker]start to install DBHA!!!"
	
	dockerParam MONIT_SHELL $MONIT_SHELL
	dockerParam SERVER_NAME $SERVER_NAME
	dockerParam BIN_NAME $BIN_NAME
	dockerParam DBHA_CONFIG $DBHA_CONFIG
	dockerParam CONFIG_PATH $CONFIG_PATH
	dockerParam MYSQL_SERVICE $MYSQL_SERVICE

	dockerStartDocker
	#1-出错  0-DBHAKeeper服务不存在 2-DBHAKeeper服务已存在
	ret_status=$?
	if [ $ret_status -eq 1 ];then
        logInfo "[startDocker]finish to install DBHA."
		return 1
	elif [ $ret_status -eq 0 ];then
        logInfo "[startDocker]finish to install DBHA."
		return 0
	fi
	
	#重启服务
	stop_function
	#镜像的mysql-uuid可能都是一样的，需要重新设置下
	mysql_data_dir=$(cat /etc/my.cnf|grep "datadir"| awk -F "=" '{print $2}' | sed 's/[ ]//g')
	rm -rf $mysql_data_dir/auto.cnf
	sleep 3
	start_function
	
	#设置是否关闭DBHA，不进行自动切换
	dockerSetDBHASwitch
	
	logInfo "[startDocker]finish to install DBHA!!!"
}

# 停止系统服务
function stopServer(){
	logInfo "[stopServer]stop server by command"
	#先杀掉定时检测mysql服务和主备同步状态的脚本
	# 杀守护脚本和主进程SIGINT(主进程会捕捉)
	stopKeep
	stopHA

	try_max_count=15
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		sleep 1
		#stopHA
		# 仅仅1情况才为停止正常
		getServerStatus > /dev/null
		[ $? -eq 1 ] && break
	done

	stopHAForce
	
	#停止日志转储功能
	#delCycleLog

	# 仅仅1情况才为停止正常
	getServerStatus
	[ $? -eq 1 ] || return 1

	return 0
}

# 获取系统服务信息
function getServerInfo(){
	# 输出服务名称
	echo "name:$SERVER_NAME"

	# 输出服务状态
	keep_exist=`ps -ef | grep /bin/sh | grep $MONIT_SHELL | awk '{print $2}'`
	server_pid=`pidof $BIN_NAME`
	# 主进程在
	if [ ! -z "$server_pid" ];then
		echo "status:running" 
	# 守护进程和主进程都不在
	elif [ -z "$keep_exist" ] && [ -z "$server_pid" ];then
		echo "status:stop" 
	# 守护脚本在，主进程不在
	elif [ ! -z "$keep_exist" ] && [ -z "$server_pid" ];then
		echo "status:starting" 
	fi
	
	# 输出网卡信息
	business_ip=`cat ${CONFIG_PATH} | grep \"${BUSINESS_VIF_KEY}\" | awk -F '"' '{print $4}'`
	[[ "$business_ip" == "" ]] || echo "interface:${business_ip} ${SERVER_NAME}"
	address=`ip addr show dev ${business_ip} | grep 'inet ' | grep 'brd' | grep "${business_ip}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
	echo "address:$address"
    version=`cd /cloud/dahua/DataBaseServer/DBHA/Bin;./DBHAServer64 -v | grep version | awk -F " " '{print$2}'`
    echo "version:$version"

	heartbeart_if=`cat ${CONFIG_PATH} | grep \"${MG_HEARTBEAT_VIF}\" | awk -F '"' '{print $4}'`
	if [[ "$heartbeart_if" = "" ]];then
		echo "interface:bond3 VCS-Heartbeat"
	else
		echo "interface:${heartbeart_if} VCS-Heartbeat"
	fi

	# 获取DBHA开机自启动状态
    is_systemd
	if [ $? -eq 0 ];then
		service_on=`systemctl is-enabled DB-Manager 2>/dev/null`
		if [ "$service_on"X == "enabled"X ];then
			echo "startup:true"
		else
			echo "startup:false"
		fi
	else
		level_2=`chkconfig --list $SERVER_NAME | awk  '{print $4}' | awk -F ':' '{print $2}'`
		level_3=`chkconfig --list $SERVER_NAME | awk  '{print $5}' | awk -F ':' '{print $2}'`
		level_4=`chkconfig --list $SERVER_NAME | awk  '{print $6}' | awk -F ':' '{print $2}'`
		level_5=`chkconfig --list $SERVER_NAME | awk  '{print $7}' | awk -F ':' '{print $2}'`
		if [ "off" = "$level_2" ] || [ "off" = "$level_3" ] || [ "off" = "level_3" ] || [ "off" = "$level_4" ]; then
		  echo "startup:false"
		else
		  echo "startup:true"
		fi
	fi
	# 输出开机是否自启动信息
	
	#输出pid,供服务的cpu和内存统计
	echo "pid:$server_pid"
	
	return 0
}

# 控制开机启动
function ctrlBootup(){
	case $1 in
		"true")
		chkconfig $SERVER_NAME on
		[ $? -eq 0 ] || return 1
		break
		;;
		"false")
		chkconfig $SERVER_NAME off
		[ $? -eq 0 ] || return 1
		break
		;;
		*)
		echo "Usage: $0 {start|stop|restart|status|info|startup true/false}" 
		return 1
		;;
	esac
	return 0
}

#0表示为修改，1表示修改了
function writeConfig(){
	item=$1
	value=$2

	if [ -f "$CONFIG_PATH" ];then
		pre_value=`cat ${CONFIG_PATH} | grep \"$item\" | awk -F '"' '{print $4}'`
		if [[ "$pre_value" = "$value" ]];then
			logInfo "[writeConfig]$item: pre value($pre_value) is same as value($value)"
			return 0
		fi
	fi

	echo "$item $value"
	cd $DBHA_SHELL
	$CONFIG_TOOL $CONFIG_PATH	 '{"'$item'" : "'$value'"}'
	reboot_flag=1
	return 1
}

function writeConfigInt(){
	item=$1
	value=$2

	if [ -f "$CONFIG_PATH" ];then
		pre_value=`cat ${CONFIG_PATH} | grep \"$item\" | awk -F '"' '{print $4}'`
		if [[ "$pre_value" = "$value" ]];then
			logInfo "[writeConfigInt]$item: pre value($pre_value) is same as value($value)"
			return 0
		fi
	fi

	echo "$item $value"
	cd $DBHA_SHELL
	$CONFIG_TOOL $CONFIG_PATH	 '{"'$item'" : '$value'}'
	reboot_flag=1
	return 1
}

# 配置DBHA配置信息
#local_business_IP/local_heartbeat_IP/peer_business_IP/peer_heartbeat_IP/DBHA_virtual_IP/DBHA_virtual_IP_netmask/DBHA_gateway/DBHA_vif_name/DBHA_if_name
reboot_flag=0
function setHAConfig(){
	logInfo "[setHAConfig]setDBHAConfig start.$*"
	shift
	reboot_flag=0
	while true;
	do
		if [ "$1" = "" ] || [ "$2" = "" ];then
			break;
		fi
		case $1 in
			-local_business_IP)
				writeConfig local_business_IP $2
				business_IP=$2
				shift 2 ;;
			-local_heartbeat_IP)
				writeConfig local_heartbeat_IP $2
				heartbeat_IP=$2
				shift 2 ;;
			-peer_business_IP)
				writeConfig peer_business_IP $2
				business_IP2=$2
				shift 2	 ;;
			-peer_heartbeat_IP)
				writeConfig peer_heartbeat_IP $2
				shift 2	 ;;
			-virtual_IP)
				writeConfig DBHA_virtual_IP $2
				virtual_IP_Tmp=$2
				shift 2	 ;;
			-virtual_IP_netmask)
				writeConfig DBHA_virtual_IP_netmask $2
				shift 2	 ;;
			-gateway)
				writeConfig DBHA_gateway $2
				shift 2	 ;;
			-if_name)
				writeConfig DBHA_if_name $2
				shift 2 ;;
			-vif_name)
				writeConfig DBHA_vif_name $2
				shift 2	 ;;
			-heartbeart_if_name)
				writeConfig DBHA_heartbeart_if_name $2
				shift 2 ;;
			-mysql_backup_path)
				writeConfig mysql_backup_path $2
				shift 2 ;;
			*)
				logInfo "[setHAConfig]$1 config is error"
				echo "$1 config is error"
				echo "usage:-local_business_IP -local_heartbeat_IP -peer_business_IP -peer_heartbeat_IP -virtual_IP -virtual_IP_netmask -gateway -vif_name -if_name -heartbeart_if_name"
				break ;;
		esac
	done

	#改密码在配置下发之前进行
	sh $MYSQL_SERVICE status
	if [ $? -ne 0 ];then
		#拉起MySQL服务修改密码
		sh $MYSQL_SERVICE start
		if [[ $? -ne 0 ]];then
			logInfo  "[startHA]mysqld start failed.install DBHA failed"
			return 1
		fi
	fi

	#生成随机密码
	local passwd=$(createPasswd $business_IP $business_IP2 $virtual_IP_Tmp)
	updateMysqlPasswd $passwd 

	_DBHA_if_name=`ip addr | grep "$business_IP\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
	_DBHA_virtual_IP_netmask=`cat /etc/sysconfig/network-scripts/ifcfg-$_DBHA_if_name | grep NETMASK | awk -F "=" {'print $2'}`
	_DBHA_gateway=`cat /etc/sysconfig/network-scripts/ifcfg-$_DBHA_if_name | grep GATEWAY | awk -F "=" {'print $2'}`
	_DBHA_vif_name=$_DBHA_if_name:11
	_DBHA_heartbeart_if_name=`ip addr | grep "$heartbeat_IP\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
    _DBHA_expand_flag=-1
    _DBHA_install_flag=1
	writeConfig DBHA_virtual_IP_netmask  $_DBHA_virtual_IP_netmask
	writeConfig DBHA_gateway $_DBHA_gateway
	writeConfig DBHA_if_name $_DBHA_if_name
	writeConfig DBHA_vif_name $_DBHA_vif_name
	writeConfig DBHA_heartbeart_if_name $_DBHA_heartbeart_if_name	
    writeConfigInt DBHA_expand_flag $_DBHA_expand_flag
    writeConfigInt DBHA_install_flag $_DBHA_install_flag
	
	if [[ $reboot_flag -eq 0 ]];then
		echo "ha config is no changed"
		logInfo "[setHAConfig]ha config is no changed"
		return 0
	else
		echo "ha is setted by different"
	fi

	#由于密码修改，重新拉起服务，避免keeper脚本一直使用旧密码
	stop_function
	sleep 3
	start_function

    #检测cs的catalogdb数据库中是否有EVT_autoClean，有的话删除
    mysql -u$mysql_user -p$mysql_password -e "use catalogdb;drop event if exists EVT_autoClean;" >/dev/null 2>&1
    logInfo "[setHAConfig]drop EVT_autoClean in CS catlogdb ret=$?"
    
    logInfo "[setHAConfig]setDBHAConfig successed."

	return 0
}

function shutDown(){
	logInfo "[shutDown]mysql mode set double write,need stop DBHA service."
	stop_function
	#关闭开机自启动
	ctrlBootup false
	echo "shutdown success"
}

function getDBHAStatus(){
	clusterID=
        if [ -f "$HA_CONFIG" ];then
            clusterID=`cat $HA_CONFIG | grep -w "cluster_id" | awk -F ": " '{print $2}' | awk -F ",|/*" '{print $1}'`
        fi

        localIP=`cat ${CONFIG_PATH} | grep \"local_business_IP\" | awk -F '"' '{print $4}'`
        virtualIP=`cat ${CONFIG_PATH} | grep \"DBHA_virtual_IP\" | awk -F '"' '{print $4}'`
        SQLMode=`cat $DBSTATUS_PATH | grep -w "SQL_mode" | awk -F ": " '{print $2}' | awk -F ",|/*" '{print $1}'`
        SQLWriteState=`cat $DBSTATUS_PATH | grep -w "SQL_write_state" | awk -F ": " '{print $2}' | awk -F ",|/*" '{print $1}'`
        slaveSyncState=`cat $DBSTATUS_PATH | grep -w "slave_sync_info" | awk -F "\"" '{print $4}' | awk -F ",|/*" '{print $1}'`
		svn_version=`cd /cloud/dahua/DataBaseServer/DBHA/Bin;./DBHAServer64 -v | grep svn | awk -F ' ' '{print $3}'`
		version=`cd /cloud/dahua/DataBaseServer/DBHA/Bin;./DBHAServer64 -v | grep version | awk -F " " '{print$2}'`
		runningtime=`cd /cloud/dahua/DataBaseServer/DBHA/Bin/;date -d@$(stat -c %Y DBHAServer64.symbol) "+%Y-%m-%d %H:%M:%S"`
        #slaveConnectMasterState=`cat $DBSTATUS_PATH | grep -w "slave_connect_master" | awk -F "\"" '{print $4}' | awk -F ",|/*" '{print $1}'`
        DBHA=`sh /etc/init.d/DB-Manager status | grep DBHA | awk -F ':' {'print$2'}`
        if [ $DBHA == "running" ];then
            status=online
        else
            status=offline
			SQLWriteState=0
        fi
		
		installFlag=`cat ${CONFIG_PATH} |  grep -w "DBHA_install_flag" | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}'`
		if [ -z "$installFlag" ] || [ $installFlag -ne 1 ];then
			installFlag=0
		fi

        echo "clusterID:$clusterID"
        echo "localIP:$localIP"
        echo "virtualIP:$virtualIP"
        echo "SQLMode:$SQLMode"
        echo "SQLWriteState:$SQLWriteState"
        #echo "slaveConnectMasterState:$slaveConnectMasterState"
        echo "slaveSyncState:$slaveSyncState"
        echo "state:$status"
		echo "svn version:$svn_version"
		echo "version:$version"
		echo "runningtime:$runningtime"
		echo "install:$installFlag"
        return 0

}

#清理ha信息
function cleanupHA()
{
	rm -rf $HA_CONFIG
	rm -rf $HA_STATE
	$CONFIG_TOOL $CONFIG_PATH  '{"local_business_IP":"", "local_heartbeat_IP" : "","peer_business_IP" : "","peer_heartbeat_IP" : "", "slave_is_recovering" : 0}'
}

function format()
{
	getServerStatus > /dev/null
	if [[ $? -ne 0 ]];then
		echo "server is stopped"
		return 1
	fi
	#if [[ "$1" = "-f" ]];then
	#	logInfo "enter to format cluster info and databases"
	#	#清理数据库
	#	cleanupDatabases
	#fi

	logInfo "[format]enter to format cluster info"
	cleanupSQL
	stop_function
	cleanupHA
	#为了让mysql的日志从0001开始
	sh $MYSQL_SERVICE start
	cleanupSQL
	start_function
	echo "format cluster success"
	return 0
}

function ignoreDB()
{
	action=$1
	para=$2
	if [[ "$action" == "add" ]];then
		addIgnoreDB $para
	elif [[ "$action" == "del" ]];then
		delIgnoreDB $para
	else
		logInfo "[ignoreDB]action:$action is error"
		return 1
	fi
	
	if [ $? -eq 0 ];then
		logInfo "[ignoreDB]$action $para success"
		echo "$action $para success"
		return 0
	else
		logInfo "[ignoreDB]$action $para failed"
		echo "$action $para failed"
		return 1
	fi
}

function setBackupDir()
{
	backup_dir=$1
	
	# 获取历史备份路径并且与新路径进行比较，不同则修改，相同退出
	local old_backup_dir=`cat ${CONFIG_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
	
	if [ "$old_backup_dir" == "" ];then
		logInfo "[setBackupDir]old mysql_backup_path is null"
		return 1
	fi
	
	if [ "$backup_dir" == "" ];then
		logInfo "[setBackupDir]mysql_backup_path is empty"
		return 1
	fi
	
	local old_backup_path=$(echo $old_backup_dir | sed 's/\\//g')
	
	if [ "$old_backup_path" == "$backup_dir" ];then
		logInfo "[setBackupDir]old mysql_backup_path and new mysql_backup_path is the same "
		return 0
	fi
	
	$CONFIG_TOOL $CONFIG_PATH  '{"mysql_backup_path" : "'$backup_dir'"}'
	if [ $? -ne 0 ];then
		logInfo "[setBackupDir]change backup path($backup_dir) failed"
		return 1
	fi

	logInfo "[setBackupDir]change mysql_backup_path($backup_dir)"
	echo "mysql_backup_path is $backup_dir"
	
	#修改路径之后，把历史路径下面的备份文件拷贝到新的路径下面
	[ ! -d $backup_dir ] && mkdir -p $backup_dir 
	mv $old_backup_dir/* $backup_dir/
	
	#重启DBHA服务
	stop_function
	sleep 3
	start_function

	#修改备机的备份路径,不再操作对端，redmine:73488
	#changeBackupDir $backup_dir

	return $?
}

function install()
{
	#ip addr | grep -B 2 219.220.101.101 | grep 'Bcast' | awk -F " " '{print $4}' |awk -F ":" '{print $2}'   子网掩码
	logInfo "[install]DBHA is ready to install."
	logInfo "[install]hosts:[$DBHA_hosts] heartbeart_if_name:[$DBHA_heartbeart_if_name]"
	logInfo "[install]if_name:[$DBHA_if_name] hearts:[$DBHA_heartbeats] vif_name:[$DBHA_vif_name] vip:[$DBHA_virtual_IP]"
	
	#业务ip和心跳ip只有主备两个
	#进行ip选择，选出本地和对端
	local host_ip1=`echo $DBHA_hosts | awk -F',' '{print $1}'`
	local host_ip2=`echo $DBHA_hosts | awk -F',' '{print $2}'`
	
	#虚IP是否与实IP相同进行判断，如果相同则退出安装，需要兼容单机部署的情形
	if [ "$host_ip2" != "" ];then
		if [ "$DBHA_virtual_IP" == "$host_ip1" ] || [ "$DBHA_virtual_IP" == "$host_ip2" ];then
			logInfo "[install]ip is error, DBHA_virtual_IP:$DBHA_virtual_IP equal host_ip"
			return 1
		fi
	fi

	local _DBHA_if_name=`ip addr | grep "$host_ip1\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
	if [ ! -n "$_DBHA_if_name" ];then
		_DBHA_if_name=`ip addr | grep "$host_ip2\/" | awk -F "global" '{print $2}' |  awk -F " " '{print $NF}' | tr -d " "`
	fi
	local local_host_ip=` ip addr show dev ${_DBHA_if_name} | grep 'inet ' | grep 'brd' | grep "${_DBHA_if_name}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
	local peer_host_ip=
	if [[ "$host_ip1" = "$local_host_ip" ]];then
		peer_host_ip=$host_ip2
	elif [[ "$host_ip2" = "$local_host_ip" ]];then
		peer_host_ip=$host_ip1
	else
		logInfo "[install]ip is error, local_host_ip:$local_host_ip"
		return 1
	fi

	local heartbeat_ip1=`echo $DBHA_heartbeats | awk -F',' '{print $1}'`
	local heartbeat_ip2=`echo $DBHA_heartbeats | awk -F',' '{print $2}'`
	local _DBHA_heartbeart_if_name=`ip addr | grep "$heartbeat_ip1\/" | awk -F "global" '{print $2}' |  awk -F " " '{print $NF}' | tr -d " "`
	if [ ! -n "$_DBHA_heartbeart_if_name" ];then
		_DBHA_heartbeart_if_name=`ip addr | grep "$heartbeat_ip2\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
	fi
	local local_heartbeat_ip=` ip addr show dev ${_DBHA_heartbeart_if_name} | grep 'inet '  | grep 'brd' | grep "${_DBHA_heartbeart_if_name}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
	local peer_heartbeat_ip=
	if [[ "$heartbeat_ip1" = "$local_heartbeat_ip" ]];then
		peer_heartbeat_ip=$heartbeat_ip2
	elif [[ "$heartbeat_ip2" = "$local_heartbeat_ip" ]];then
		peer_heartbeat_ip=$heartbeat_ip1
	else
		logInfo "[install]ip is error, local_heartbeat_ip:$local_heartbeat_ip"
		return 1
	fi

	DBHA_gateway=`cat /etc/sysconfig/network-scripts/ifcfg-${_DBHA_if_name} | grep GATEWAY | awk -F "=" {'print $2'}`
	DBHA_virtual_IP_netmask=`cat /etc/sysconfig/network-scripts/ifcfg-${_DBHA_if_name} | grep NETMASK | awk -F "=" {'print $2'}`
	_DBHA_vif_name=$_DBHA_if_name:11
	_DBHA_expand_flag=-1
	_DBHA_install_flag=1

	#改密码在配置下发之前进行
	sh $MYSQL_SERVICE status
	if [ $? -ne 0 ];then
		#拉起MySQL服务修改密码
		sh $MYSQL_SERVICE start
		if [[ $? -ne 0 ]];then
			logInfo  "[startHA]mysqld start failed.install DBHA failed"
			return 1
		fi
	fi

	#生成随机密码
	local passwd=$(createPasswd $host_ip1 $host_ip2 $DBHA_virtual_IP)
	updateMysqlPasswd $passwd

	writeConfig local_business_IP $local_host_ip
	writeConfig local_heartbeat_IP $local_heartbeat_ip
	writeConfig peer_business_IP $peer_host_ip
	writeConfig peer_heartbeat_IP $peer_heartbeat_ip
	writeConfig DBHA_heartbeart_if_name $_DBHA_heartbeart_if_name
	writeConfig DBHA_virtual_IP $DBHA_virtual_IP
	writeConfig DBHA_vif_name $_DBHA_vif_name
	writeConfig DBHA_if_name $_DBHA_if_name
	writeConfig DBHA_gateway $DBHA_gateway
	writeConfig DBHA_virtual_IP_netmask  $DBHA_virtual_IP_netmask
	writeConfigInt DBHA_expand_flag $_DBHA_expand_flag
	writeConfigInt DBHA_install_flag $_DBHA_install_flag
	
	#重启服务
	stop_function
	sleep 3
	start_function

    #检测cs的catalogdb数据库中是否有EVT_autoClean，有的话删除
    mysql -u$mysql_user -p$mysql_password -e "use catalogdb;drop event if exists EVT_autoClean;" >/dev/null 2>&1
    #drop_autoClean_ret=$(mysql -u$mysql_user -p$mysql_password -e "use catalogdb;drop event if exists EVT_autoClean;")
    logInfo "[install]drop EVT_autoClean in CS catlogdb ret=$?"
    
    logInfo "[install]finish to install DBHA."

    return 0
}

function createPasswd()
{
	local passwd="Dh@21"
	if [ $# -eq 3 ];then
	    #避免传入顺序不同导致生成密码不同，规定IP大的在前
		if [[ $1 > $2 ]];then
			local business_ip1=$1
			local business_ip2=$2
		else
			local business_ip1=$2
			local business_ip2=$1
		fi
		local vip=$3
	elif [ $# -eq 2 ];then
		local business_ip1=$1
		local business_ip2=$1
		local vip=$2
	else
		logInfo "[createPasswd]error:input num error."
		return 1
	fi

	encry_key1=`$ENCRY_TOOL -content $business_ip1 -mode 0  |awk -F':' '{print $2}' |cut -c 1-4` 
	encry_key2=`$ENCRY_TOOL -content $business_ip2 -mode 0 |awk -F':' '{print $2}' |cut -c 5-8`
	encry_key3=`$ENCRY_TOOL -content $vip -mode 0 |awk -F':' '{print $2}' |cut -c 1-4`

	passwd=$passwd$encry_key1$encry_key2$encry_key3

	echo $passwd
	return 0
}

function updateMysqlPasswd()
{
	local user_pwd=$1

	if [ $# -eq 2 ];then
		local force_change=$2
	else
		local force_change=0
	fi 
	
	#已经修改过内部账户和密码则不需要再修改,除非扩容时需要强制修改
	local MYSQL_CONF=$DBHA_CONFIG/Mysql.conf
	local security_has_changed=$(grep "security_has_changed" $MYSQL_CONF | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}')
	if [ $security_has_changed -eq 1 ] && [ $force_change -eq 0 ];then
		return 0
	fi

	#检测虚IP访问是否成功
    local retry=3
    for((i=0;i<$retry;i++));do
        mysqladmin -u$mysql_user -p$mysql_password --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
        if [ $? -eq 0 ];then
            break
        else
            logInfo "[updateMysqlPasswd]mysql ping  timeout:$retry!"
            sleep 3
        fi
    done

	mysqladmin -u$mysql_user -p$mysql_password  --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
    if [ $? -ne 0 ];then
        logInfo "[updateMysqlPasswd]mysql connect failed,updateMysqlPasswd failed!"
        return 1
    fi

	mysql -u$mysql_user -p$mysql_password  --connect-expired-password -e"update mysql.user set authentication_string=password('$user_pwd') where User='root';flush privileges;"
	if [ $? -ne 0 ]; then
		logInfo "[updateMysqlPasswd]change mysql $mysql_user password false"
		return 1
	fi
    logInfo "[updateMysqlPasswd]change mysql $mysql_user to *** success"

	#测试密码是否设置成功
	mysql -u$mysql_user -p"${user_pwd}" -e"select 1;" > /dev/null
	if [ $? -ne 0 ]; then
		logInfo "[updateMysqlPasswd]check mysql $mysql_user failed"
		return 1
	else
		logInfo "[updateMysqlPasswd]check mysql $mysql_user success"
	fi
	
	#修改配置信息并设置到配置文件中去
	local base64_user=`echo "${mysql_user}" | base64`
	local base64_pwd=`echo "${user_pwd}" | base64`
	$CONFIG_TOOL $MYSQL_CONF '{"user_security" : "'$base64_user'"}'
	$CONFIG_TOOL $MYSQL_CONF '{"password_security" : "'$base64_pwd'"}'
	$CONFIG_TOOL $MYSQL_CONF '{"security_has_changed" : 1}'

	#重新读取密码
	getUserCfg
}

##标记
function modifyServiceConfig()
{
	if [ $# != 7 ];then
		logInfo "[modifyServiceConfig]error:input num error."
		return 1
	fi
	
	local bond_name=$1
	local old_ip=$2
	local old_netmask=$3
	local old_gateway=$4
	local new_ip=$5
	local new_netmask=$6
	local new_gateway=$7
	logInfo "[modifyServiceConfig]DBHA modify config."
	logInfo "[modifyServiceConfig]bond_name: $bond_name, old_ip: $old_ip, old_netmask: $old_netmask, old_gateway: $old_gateway"
	logInfo "[modifyServiceConfig]new_ip: $new_ip, new_netmask: $new_netmask, new_gateway: $new_gateway"
	
	if [[ "$old_ip" = "$new_ip" ]] && [[ "$old_netmask" = "$new_netmask" ]] && [[ "$old_gateway" = "$new_gateway" ]];then
		logInfo "[modifyServiceConfig]ip is no change."
		return 0
	fi

	bond_configs=`cat ${CONFIG_PATH} | grep -w \"$bond_name\" | awk -F '"' '{print $2}'`
	
	if [ "$bond_configs" != "" ];then
		sed -i "s/\"$old_ip\"/\"$new_ip\"/g" $CONFIG_PATH
		sed -i "s/\"$old_netmask\"/\"$new_netmask\"/g" $CONFIG_PATH
		sed -i "s/\"$old_gateway\"/\"$new_gateway\"/g" $CONFIG_PATH
        
		#重启服务
		stop_function
		sleep 3
		start_function
	else
		logInfo "[modifyServiceConfig]there is nothing to modify config."
	fi

	return 0
}

##Redmine:26642标记
function updateIpInfo()
{	
	local bond_name=$ethName
	local old_ip=$oldAddress
	local old_netmask=$oldNetmask
	local old_gateway=$oldGateway
	local new_ip=$address
	local new_netmask=$netmask
	local new_gateway=$gateway
	local service_name_temp=$serviceName
	logInfo "[updateIpInfo]DBHA update ip info."
	logInfo "[updateIpInfo]bond_name: $bond_name, old_ip: $old_ip, old_netmask: $old_netmask, old_gateway: $old_gateway"
	logInfo "[updateIpInfo]new_ip: $new_ip, new_netmask: $new_netmask, new_gateway: $new_gateway, service_name: $service_name_temp"
	
	if [[ "$old_ip" = "$new_ip" ]] && [[ "$old_netmask" = "$new_netmask" ]] && [[ "$old_gateway" = "$new_gateway" ]];then
		logInfo "[updateIpInfo]ip is no change."
		return 0
	fi
	
	#参数判断，检查old_ip和new_ip是否为空
	if [ "$old_ip"X == ""X ] || [ "$new_ip"X == ""X ];then
		logInfo "[updateIpInfo]ip is empty,$old_ip,$new_ip."
		return 0
	fi

	#统一运维传入DB-Manager的service名时修改DBHA的虚IP
	if [ "$service_name_temp"X != ""X ];then
		if [ "$service_name_temp"X == "DB-Manager"X ];then
            stop_function
			sed -i "s/\"$old_ip\"/\"$new_ip\"/g" $CONFIG_PATH
            start_function
			logInfo "[updateIpInfo]modify $service_name_temp old_ip:$old_ip , new_ip:$new_ip."
		fi
	else
		bond_configs=`cat ${CONFIG_PATH} | grep -w \"$bond_name\" | awk -F '"' '{print $2}'`
	
		if [ "$bond_configs" != "" ];then
            stop_function
			sed -i "s/\"$old_ip\"/\"$new_ip\"/g" $CONFIG_PATH
			sed -i "s/\"$old_netmask\"/\"$new_netmask\"/g" $CONFIG_PATH
			sed -i "s/\"$old_gateway\"/\"$new_gateway\"/g" $CONFIG_PATH
            start_function
            logInfo "[updateIpInfo]modify $service_name_temp old_ip:$old_ip , new_ip:$new_ip."
		else
			logInfo "[updateIpInfo]there is nothing to modify config."
		fi
	fi
	
	return 0
}

# 单机扩容接口
function expandHA()
{
	# 先判断当前是否已经扩容过
	virtual_ip=`cat $CONFIG_PATH |grep "DBHA_virtual_IP\"" |awk -F "\"" '{print$4}'`
	peer_bus_ip=`cat $CONFIG_PATH |grep "peer_business_IP\"" |awk -F "\"" '{print$4}'`
	expand_flag=`cat $CONFIG_PATH | grep "DBHA_expand_flag\"" | awk -F ": " '{print $2}' | awk -F ",|/*" '{print $1}'`
	if [ "${peer_bus_ip}" == "" ] && [ "${expand_flag}" == "-1" ]; then
		logInfo "[expandHA]The service can expand,peer_bus_ip:${peer_bus_ip}."
	else
		logInfo "[expandHA]The service has expanded:${peer_bus_ip},${expand_flag}."
		return 0
	fi
	logInfo "[expandHA]Begin expand"
	
	#ip addr | grep -B 2 219.220.101.101 | grep 'Bcast' | awk -F " " '{print $4}' |awk -F ":" '{print $2}'   子网掩码
	logInfo "[expandHA]DBHA is ready to expand."
	logInfo "[expandHA]hosts:[$DBHA_hosts] heartbeart_if_name:[$DBHA_heartbeart_if_name]"
	logInfo "[expandHA]if_name:[$DBHA_if_name] hearts:[$DBHA_heartbeats] vif_name:[$DBHA_vif_name] vip:[$DBHA_virtual_IP]"
	
	#业务ip和心跳ip只有主备两个
	#进行ip选择，选出本地和对端
	local host_ip1=`echo $DBHA_hosts | awk -F',' '{print $1}'`
	local host_ip2=`echo $DBHA_hosts | awk -F',' '{print $2}'`
	local _DBHA_if_name=`ip addr | grep "$host_ip1\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
	if [ ! -n "$_DBHA_if_name" ];then
		_DBHA_if_name=`ip addr | grep "$host_ip2\/" | awk -F "global" '{print $2}' |  awk -F " " '{print $NF}' | tr -d " "`
	fi
	local local_host_ip=`ip addr show dev ${_DBHA_if_name} | grep 'inet '  | grep 'brd' | grep "${_DBHA_if_name}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
	local peer_host_ip=
	if [[ "$host_ip1" = "$local_host_ip" ]];then
		peer_host_ip=$host_ip2
	elif [[ "$host_ip2" = "$local_host_ip" ]];then
		peer_host_ip=$host_ip1
	else
		logInfo "[expandHA]ip is error, local_host_ip:$local_host_ip"
		return 1
	fi

	local heartbeat_ip1=`echo $DBHA_heartbeats | awk -F',' '{print $1}'`
	local heartbeat_ip2=`echo $DBHA_heartbeats | awk -F',' '{print $2}'`
	local _DBHA_heartbeart_if_name=`ip addr | grep "$heartbeat_ip1\/" | awk -F "global" '{print $2}' |  awk -F " " '{print $NF}' | tr -d " "`
	if [ ! -n "$_DBHA_heartbeart_if_name" ];then
		_DBHA_heartbeart_if_name=`ip addr | grep "$heartbeat_ip2\/" | awk -F "global" '{print $2}' |  awk -F " " '{print $NF}' | tr -d " "`
	fi
	local local_heartbeat_ip=`ip addr show dev ${_DBHA_heartbeart_if_name} | grep 'inet ' | grep 'brd' | grep "${_DBHA_heartbeart_if_name}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
	local peer_heartbeat_ip=
	if [[ "$heartbeat_ip1" = "$local_heartbeat_ip" ]];then
		peer_heartbeat_ip=$heartbeat_ip2
	elif [[ "$heartbeat_ip2" = "$local_heartbeat_ip" ]];then
		peer_heartbeat_ip=$heartbeat_ip1
	else
		logInfo "[expandHA]ip is error, local_heartbeat_ip:$local_heartbeat_ip"
		return 1;
	fi
	
	#判断自己是否为扩容节点
	SQLMode=`cat $DBSTATUS_PATH | grep -w "SQL_mode" | awk -F ": " '{print $2}' | awk -F ",|/*" '{print $1}'`
	if [[ "$expandIp"X == "$local_heartbeat_ip"X ]] || [[ "$expandIp"X == "$local_host_ip"X ]];then
		#如果配置信息存在则打印warn信息到log中
		if [ "$SQLMode" != "-1"  ];then
			logInfo "[expandHA]$local_host_ip is expanding node,but not new machine check it!"
			return 1
		fi
	fi
	
	DBHA_gateway=`cat /etc/sysconfig/network-scripts/ifcfg-${_DBHA_if_name} | grep GATEWAY | awk -F "=" {'print $2'}`
	DBHA_virtual_IP_netmask=`cat /etc/sysconfig/network-scripts/ifcfg-${_DBHA_if_name} | grep NETMASK | awk -F "=" {'print $2'}`
	_DBHA_vif_name=$_DBHA_if_name:11
	#正在扩容标记,由程序内设置，1正在扩容由脚本设置；默认为-1出厂设置或扩容结束
	_DBHA_expand_flag=1
	_DBHA_install_flag=1

	#改密码在配置下发之前进行
	sh $MYSQL_SERVICE status
	if [ $? -ne 0 ];then
		#拉起MySQL服务修改密码
		sh $MYSQL_SERVICE start
		if [[ $? -ne 0 ]];then
			logInfo  "[startHA]mysqld start failed.expandHA DBHA failed"
			return 1
		fi
	fi

	#清理原有的binlog,避免binlog中存在原始密码导致同步异常 #73488
	#先新产生一个binlog
	mysql -u$mysql_user -p$mysql_password -e "flush binary logs;"
    local mysql_data=`cat /etc/my.cnf |grep datadir | awk -F "=|= " {'print $2'}`
	local bin_log_name=mysql-bin
	bin_log_new=`ls -alfrt $mysql_data |grep "$bin_log_name" | grep -v "index" | tail -n1`
	mysql -u$mysql_user -p$mysql_password -e "purge binary logs to \"$bin_log_new\";"
	logInfo "master purge bin log $mysql_data/$bin_log_new"

	#生成随机密码
	local passwd=$(createPasswd $host_ip1 $host_ip2 $DBHA_virtual_IP)
	#force_change=1表示强制修改root密码，保证扩容时密码为新生成的密码，两台设备一致
	local force_change=1
	updateMysqlPasswd $passwd $force_change

	writeConfig local_business_IP $local_host_ip
	writeConfig local_heartbeat_IP $local_heartbeat_ip
	writeConfig peer_business_IP $peer_host_ip
	writeConfig peer_heartbeat_IP $peer_heartbeat_ip
	writeConfig DBHA_heartbeart_if_name $_DBHA_heartbeart_if_name
	writeConfig DBHA_virtual_IP $DBHA_virtual_IP
	writeConfig DBHA_vif_name $_DBHA_vif_name
	writeConfig DBHA_if_name $_DBHA_if_name
	writeConfig DBHA_gateway $DBHA_gateway
	writeConfig DBHA_virtual_IP_netmask  $DBHA_virtual_IP_netmask
	writeConfigInt DBHA_expand_flag $_DBHA_expand_flag
	writeConfigInt DBHA_install_flag $_DBHA_install_flag

	#由于密码修改，重新拉起服务，避免keeper脚本一直使用旧密码
	stop_function
	sleep 3
	start_function

	logInfo "[expandHA]finish to expand DBHA."
	return 0
}

function migrateManager()
{      
    logInfo "[migrateManager]DBHA is ready to migrant."
	logInfo "[migrateManager]migrantIP:[$migrantIP] vcsBusinessEthName:[$vcsBusinessEthName] vcsHeartEthName[$vcsHeartEthName]"
	logInfo "[migrateManager]localVcsHeartbeatIP:[$localVcsHeartbeatIP] peerVcsHeartbeatIP:[$peerVcsHeartbeatIP] mysqlVip:[$mysqlVip] HeartbeatNetMask:[$HeartbeatNetMask]"
    
    # 3.停止管理节点服务
    stop_function

    # 5.ssh到对端，停止迁移目的端管理节点服务
    logInfo "[migrateManager]ssh $migrantIP stop DB-Manager."
    /usr/bin/ssh $migrantIP "/etc/init.d/DB-Manager stop"

    # 6.修改管理节点开机自启动项，改为关闭
    ctrlBootup false 
    rm -rf /cloud/service/services/DB-Manager

    if [ "${vcsHeartEthName}" == "" ]; then
		# 如果心跳网卡传成空的则,则是单机外迁只需要外迁节点的业务网卡名
		log_info "Single manager migrate not need set heartbeat info."
        local local_DBHA_vif_name=$vcsBusinessEthName:11
        writeConfig DBHA_if_name $vcsBusinessEthName
        writeConfig DBHA_heartbeart_if_name $vcsBusinessEthName
        writeConfig DBHA_vif_name $local_DBHA_vif_name
	else
        #参数检查
        if [ -z "$localVcsHeartbeatIP" ] || [ -z "$HeartbeatNetMask" ] || [ -z "$vcsHeartEthName" ] || [ -z "$mysqlVip" ];then
            logInfo "[migrateManager]parameter check failed!"
            return 1
        fi
        
        # 9.修改服务IP信息
        local local_DBHA_vif_name=$vcsHeartEthName:11
        writeConfig local_business_IP $localVcsHeartbeatIP
        writeConfig DBHA_if_name $vcsHeartEthName
        writeConfig DBHA_virtual_IP $mysqlVip
        writeConfig DBHA_vif_name $local_DBHA_vif_name
        #writeConfig DBHA_gateway $HeartbeatNetMask
        writeConfig DBHA_virtual_IP_netmask  $HeartbeatNetMask
        
        local local_heartbeat_IP=`cat $CONFIG_PATH |grep "local_heartbeat_IP\"" |awk -F "\"" '{print$4}'`
        if [ ! -z "$local_heartbeat_IP" ];then
            writeConfig local_heartbeat_IP $localVcsHeartbeatIP
            writeConfig DBHA_heartbeart_if_name $vcsHeartEthName
        fi
        
        local peer_business_IP=`cat $CONFIG_PATH |grep "peer_business_IP\"" |awk -F "\"" '{print$4}'`
        if [ ! -z "$peer_business_IP" ];then
            writeConfig peer_business_IP $peerVcsHeartbeatIP
        fi
        
        local peer_heartbeat_IP=`cat $CONFIG_PATH |grep "peer_heartbeat_IP\"" |awk -F "\"" '{print$4}'`
        if [ ! -z "$peer_heartbeat_IP" ];then
            writeConfig peer_heartbeat_IP $peerVcsHeartbeatIP
        fi
    fi
	
	# 11.将服务的配置文件和数据拷贝到外迁标准云管理节点对应目录下，使用scp的方式拷贝。日志不用拷贝过去
	/usr/bin/scp $SERVER_PATH/Config/* root@$migrantIP:$SERVER_PATH/Config/
	if [ $? -ne 0 ];then
		logInfo "[migrateManager]scp  config to migrant manager:$migrantIP failed."
		return 1
	fi
    
    # 12.拷贝数据库数据
    mysql_data_dir=$(grep datadir $SERVER_PATH/Config/my.cnf | awk -F "=" '{print $2}' | sed 's/ //g')
    /usr/bin/scp -r $mysql_data_dir/* root@$migrantIP:$mysql_data_dir/
    if [ $? -ne 0 ];then
		logInfo "[migrateManager]scp  $$mysql_date_dir to migrant manager:$migrantIP failed."
		return 1
	fi
	/usr/bin/ssh $migrantIP "chown -R mysql:mysql $mysql_data_dir"
    
	logInfo "[migrateManager]DB-Manager:$localOldIP, migrate to manager:$migrantIP success."
    return 0
}

function testLink()
{
    if [ $# -ne 2 ];then
        return 1
    fi
    
    name=$1
    pwd=$2
    
    virtual_ip=`cat $CONFIG_PATH |grep "DBHA_virtual_IP\"" |awk -F "\"" '{print$4}'`
    if [ -z "$virtual_ip" ];then
        logInfo "[testLink]$userName, $userPwd get vip failed!"
        echo "errMessage:{ \"code\": \"dbTestLinkError\", \"message\": \"service not install ready\"}"
        return 1
    fi
    
	mysqladmin -u$name -p$pwd -h$virtual_ip --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
    return $?
}

function updateInnerUserPasswd()
{
	#已经修改过内部账户和密码则不需要再修改
	local MYSQL_CONF=$DBHA_CONFIG/Mysql.conf
	security_has_changed=$(grep "security_has_changed" $MYSQL_CONF | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}')
	if [ $security_has_changed -eq 1 ];then
		return 0
	fi
	
	local user_pwd=$1
	installFlag=`cat ${CONFIG_PATH} |  grep -w "DBHA_install_flag" | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}'`
	if [ $installFlag -eq 1 ];then
		#获取DBHA的虚IP地址，在虚IP上创建该用户
		virtual_ip=`cat $CONFIG_PATH |grep "DBHA_virtual_IP\"" |awk -F "\"" '{print$4}'`
		if [ -z "$virtual_ip" ];then
			logInfo "[updateInnerUserPasswd]create user:$userName, $userPwd get vip failed!"
			echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service not install ready\"}"
			return 1
		fi
		db_host=$virtual_ip
		
		#判断主备是否正常同步，避免创建用户时主备同步，此时主备不正常同步，导致root新密码无法同步。故在此再判断一次
		checkSyncStatus
		if [ $? -ne 0 ];then
			echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service not run normal,the sync status abnormal\"}"
			logInfo "[updateInnerUserPasswd]modify the password of root failed,the status of slave is abnormal!"
			return 1
		fi
	else
		db_host=127.0.0.1
	fi
	#generate_random_pwd=$(date +%s | sha256sum | base64 | head -c 16 ; echo)
	#使用统一运维下发下来的密码

	mysql -u$mysql_user -p$mysql_password -h$db_host --connect-expired-password -e"update mysql.user set authentication_string=password('$user_pwd') where User='$mysql_user';flush privileges;"
	if [ $? -ne 0 ]; then
		logInfo "[updateInnerUserPasswd]change mysql $mysql_user password false"
		return 1
	fi
    logInfo "[updateInnerUserPasswd]change mysql $mysql_user to $user_pwd"

	#测试密码是否设置成功
	mysql -u$mysql_user -p"${user_pwd}" -h$db_host -e"select 1;" > /dev/null
	if [ $? -ne 0 ]; then
		logInfo "[updateInnerUserPasswd]check mysql $mysql_user failed"
		return 1
	else
		logInfo "[updateInnerUserPasswd]check mysql $mysql_user $user_pwd success"
	fi
	
	#修改配置信息并设置到配置文件中去
	local base64_user=`echo "${mysql_user}" | base64`
	local base64_pwd=`echo "${user_pwd}" | base64`
	$CONFIG_TOOL $MYSQL_CONF '{"user_security" : "'$base64_user'"}'
	$CONFIG_TOOL $MYSQL_CONF '{"password_security" : "'$base64_pwd'"}'
	$CONFIG_TOOL $MYSQL_CONF '{"security_has_changed" : 1}'
	if [ $installFlag -eq 1 ];then
		logInfo "[updateInnerUserPasswd]start change peer Mysql.conf"
		peer_business_IP=`cat $CONFIG_PATH |grep "peer_business_IP"|awk -F "\"" '{print $4}'`
		local_business_IP=`cat $CONFIG_PATH |grep "local_business_IP"|awk -F "\"" '{print $4}'`
		if [ "$peer_business_IP" == "" ] || [ "$peer_business_IP" == "$local_business_IP" ];then
			logInfo "[updateInnerUserPasswd]peer_business_IP=$peer_business_IP,local_business_IP=$local_business_IP,return"
			return 0
		else
			ssh $peer_business_IP "$CONFIG_TOOL $MYSQL_CONF '{\"user_security\": \"$base64_user\"}'"
			ssh $peer_business_IP "$CONFIG_TOOL $MYSQL_CONF '{\"password_security\": \"$base64_pwd\"}'"
			ssh $peer_business_IP "$CONFIG_TOOL $MYSQL_CONF '{\"security_has_changed\": 1}'"
			
			peer_password=`ssh $peer_business_IP cat $MYSQL_CONF | grep \"password_security\" | awk -F '"' '{print $4}'`
			if [ "$peer_password" == "$base64_pwd" ];then
				logInfo "[updateInnerUserPasswd]change peer Mysql.conf password success"
			else
				logInfo "[updateInnerUserPasswd]change peer Mysql.conf password failed"
			fi
		fi
	fi
	#暂时不删除DHCloud账户
	#mysql -u$mysql_user -p"${userPwd}" -h$virtual_ip mysql -e "delete from mysql.user where User='DHCloud';FLUSH PRIVILEGES;" 2>/dev/null
}

function createUser()
{
	if [ -z "$userName" ] || [ -z "$userPwd" ];then
		logInfo "[createUser]$userName, $userPwd invalid param!"
		echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"invalid param\"}"
		return 1
	fi
    
    #获取DBHA的虚IP地址，在虚IP上创建该用户
    virtual_ip=`cat $CONFIG_PATH |grep "DBHA_virtual_IP\"" |awk -F "\"" '{print$4}'`
    if [ -z "$virtual_ip" ];then
        logInfo "[createUser]$userName, $userPwd get vip failed!"
        echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service not install ready\"}"
        return 1
    fi
    
    #检测虚IP访问是否成功
    local retry=3
    for((i=0;i<$retry;i++));do
        mysqladmin -u$mysql_user -p$mysql_password -h$virtual_ip --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
        if [ $? -eq 0 ];then
            break
        else
            logInfo "[createUser]mysql ping $virtual_ip timeout:$retry!"
            sleep 3
        fi
    done
    
	mysqladmin -u$mysql_user -p$mysql_password -h$virtual_ip --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
    if [ $? -ne 0 ];then
        logInfo "[createUser]$userName, $userPwd vip $virtual_ip not exist!"
        echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service virtual ip $virtual_ip not exist\"}"
        return 1
    fi
    
    #检测用户是否存在，存在则创建失败
    local sql="SELECT User FROM mysql.user WHERE User='$userName'"
    exist=$(mysql -u$mysql_user -p$mysql_password -h$virtual_ip -e "$sql" 2>/dev/null | grep  -v 'User' | grep -c $userName)
    if [ $exist -gt 0 ];then
        echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"user already exist\"}"
        logInfo "[createUser]$userName, $userPwd already exist!"
        return 1
    fi
    
    #检测主备是否正常同步，正常同步则允许创建用户，否则创建用户失败 redmine:63961
    checkSyncStatus
    if [ $? -ne 0 ];then
        echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service not run normal,the sync status abnormal\"}"
        logInfo "[createUser]create user:$userName, $userPwd failed,the status of slave is abnormal!"
        return 1
    fi

    mysql -u$mysql_user -p$mysql_password -h$virtual_ip mysql -e "GRANT ALL PRIVILEGES ON *.* TO '$userName'@'%' IDENTIFIED BY '$userPwd' WITH GRANT OPTION;FLUSH PRIVILEGES;" 2>/dev/null
    exist=$(mysql -u$mysql_user -p$mysql_password -h$virtual_ip -e "$sql" 2>/dev/null | grep  -v 'User' | grep -c $userName)
    if [ $exist -gt 0 ];then
        #修改成功后还需要更改root账户的密码
        logInfo "[createUser]$userName, $userPwd"
        dbhaInstallFlag=`cat ${CONFIG_PATH} |  grep -w "DBHA_install_flag" | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}'`
        logInfo "[createUser]local dbhaInstallFlag value is $dbhaInstallFlag "
        #if [ $dbhaInstallFlag -eq 1 ];then
            #为了兼容云库旧版本升级，当DBHA.conf的DBHA_install_flag=1(调用过install接口)时，才修改root密码。
            #updateInnerUserPasswd $userPwd
        #fi
        return 0
    else
        logInfo "[createUser]$userName, $userPwd failed!"
        echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"create user failed\"}"
        return 1
    fi
}

#在DBHA未部署或者已部署的情况下都有可能创建用户
function createEfsUser()
{
	if [ $# -ne 2 ];then
		logInfo "[createEfsUser]invalid param!"
		echo "errMessage:{ \"code\": \"dbCreateEfsUserError\", \"message\": \"invalid param\"}"
		return 1
	fi
	
	local user_name=$1
	local user_pwd=$2
	#DBHA如果已经部署则在虚IP上创建，如果没部署就在本机上创建
	install_flag=`cat ${CONFIG_PATH} |  grep -w "DBHA_install_flag" | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}'`
	if [ $install_flag -eq 1 ];then
		#获取DBHA的虚IP地址，在虚IP上创建该用户
		virtual_ip=`cat $CONFIG_PATH |grep "DBHA_virtual_IP\"" |awk -F "\"" '{print$4}'`
		if [ -z "$virtual_ip" ];then
			logInfo "[createEfsUser]$user_name, $user_pwd get vip failed!"
			echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service not install ready\"}"
			return 1
		fi
		db_host=$virtual_ip
		#判断主备是否同步
		checkSyncStatus
		if [ $? -ne 0 ];then
			echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"service not run normal,the sync status abnormal\"}"
			logInfo "[createEfsUser]create user:$user_name, $user_pwd failed,the status of slave is abnormal!"
			return 1
		fi
	else
		db_host=127.0.0.1
	fi
	
	#检测IP能否访问成功		
	mysqladmin -u$mysql_user -p$mysql_password -h$db_host --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
	if [ $? -ne 0 ];then
		logInfo "[createEfsUser]$user_name, $user_pwd $db_host mysql connect failed!"
		echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"$db_host mysql connect failed\"}"
		return 1
	fi
	
	#检测用户是否存在，存在则修改密码
	local sql="SELECT User FROM mysql.user WHERE User='$user_name'"
	exist=$(mysql -u$mysql_user -p$mysql_password -h$db_host -e "$sql" 2>/dev/null | grep  -v 'User' | grep -c $user_name)
	if [ $exist -gt 0 ];then
		mysql -u$mysql_user -p$mysql_password -h$db_host -e "update mysql.user set authentication_string=password('$user_pwd') where User='$user_name';FLUSH PRIVILEGES;" 2>/dev/null
	else
		mysql -u$mysql_user -p$mysql_password -h$db_host -e "GRANT ALL PRIVILEGES ON *.* TO '$user_name'@'%' IDENTIFIED BY '$user_pwd';FLUSH PRIVILEGES;" 2>/dev/null
	fi
	
	#测试密码是否设置成功
	mysql -u$user_name -p"${user_pwd}" -h$db_host -e"select 1;" > /dev/null
	if [ $? -ne 0 ]; then
		logInfo "[createEfsUser]create mysql $user_name $user_pwd failed"
		return 1
	else
		logInfo "[createEfsUser]create mysql $user_name $user_pwd success"
	fi
	
	#判断root账户是否需要修改密码
	updateInnerUserPasswd $user_pwd
	return 0
}

function checkSyncStatus()
{
    #判断主备是否一致
    local peer_business_IP=`cat $CONFIG_PATH |grep "peer_business_IP"|awk -F "\"" '{print $4}'`
    local local_business_IP=`cat $CONFIG_PATH |grep "local_business_IP"|awk -F "\"" '{print $4}'`
    if [ "$local_business_IP" != "" ] && [ "$peer_business_IP" != "" ];then
        local local_io_status=$(mysql -u$mysql_user -p$mysql_password -h$local_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
        local local_slave_status=$(mysql -u$mysql_user -p$mysql_password -h$local_business_IP -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
        local peer_io_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
        local peer_slave_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
        if [ "$local_io_status" == "" ];then
            if [[ "$peer_io_status" != "Yes" ]] || [[ "$peer_slave_status" != "Yes" ]];then
                return 1
            fi
        else
            if [[ "$local_io_status" != "Yes" ]] || [[ "$local_slave_status" != "Yes" ]];then
                return 1
            fi
        fi
    fi
    return 0
}

# 查文件是否齐全
# 守护脚本
if [ ! -f $DBHA_SHELL/Service/$MONIT_SHELL ];then
	logInfo "monit shell doesn't exist, path=$DBHA_SHELL/$MONIT_SHELL"
	exit 1
fi
# 配置文件
if [ ! -f $CONFIG_PATH ];then
	logInfo "config file doesn't exist, path=$CONFIG_PATH"
	exit 1
fi

# DN网卡配置文件
DN_INTERFACE_PATH="/etc/network/interfaces"

if [ "$1" == "startDocker" ] && [ ! -d "/data/mysql/data/mysql" ];then
	mv -f /data/mysql_tmp/* /data/mysql/
	chgrp -R mysql /data/mysql/data
	chown -R mysql /data/mysql/data
fi

if [ "$1" == "startDocker" ] && [ "$ENABLE_SSH"X != "false"X ];then
	ssh_pid=$(pidof sshd)
	if [ -z "$ssh_pid" ];then
		/usr/sbin/sshd -D > /dev/null 2>&1 &
	fi
fi


case $1 in
  "start")
  start_function
  exit $?
  ;;
  "startService")
  startServer
  exit $?
  ;;
  "stopService")
  stopServer
  exit $?
  ;;
  "stop")
  stop_function
  exit $?
  ;;
  "restartService")
  stopServer
  [ $? -eq 0 ] || exit 1
  startServer
  exit $?
  ;;
  "restart")
  restart_function
  exit $?
  ;;
  "info")
  getServerInfo
  exit $?
  ;;
  "status")
  getServerStatus
  exit $?
  ;;
  #为了兼容升级，后续可能会去掉
  "setConfig")
  setHAConfig $*
  exit $?
  ;;
  #为了兼容升级改接口暂时保留，新版本使用detail接口
  "getBusinessInfo")
  getDBHAStatus
  exit $?
  ;;
  "detail")
  getDBHAStatus
  exit $?
  ;;
  "shutdown")
  shutDown
  exit $?
  ;;
  "startup")
  ctrlBootup $2
  exit $?
  ;;
   "format")
  format $2
  exit $?
  ;;
  "ignoreDB")
  ignoreDB $2 $3
  exit $?
  ;;
  "setBackupDir")
  setBackupDir $2
  exit $?
  ;;
  "install")
  source $2
  install
  exit $?
  ;;
  "modifyServiceConfig")
  modifyServiceConfig $2 $3 $4 $5 $6 $7 $8
  exit $?
  ;;
  "expandHA")
  source $2
  expandHA
  exit $?
  ;;
  "migrateManager")
  source $2
  migrateManager
  exit $?
  ;;
  "updateIpInfo")
  source $2
  updateIpInfo
  exit $?
  ;;
  "createUser")
  source $2
  createUser
  exit $?
  ;;
  "createEfsUser")
  shift
  createEfsUser $*
  exit $?
  ;;
  "testUser")
  source $2
  testLink $userName $userPwd
  ret=$?
  if [ $ret -ne 0 ];then
    echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"invalid user $userName\"}"
  fi
  exit $ret
  ;;
  "testLink")
  testLink $mysql_user $mysql_password
  ret=$?
  if [ $ret -ne 0 ];then
    echo "errMessage:{ \"code\": \"dbCreateUserError\", \"message\": \"mysqld is not running\"}"
  fi
  exit $ret
  ;;
  "startDocker")
  startDocker
  exit 0
  ;;
  *)
  echo "Usage: $0 {start|stop|restart|status|info|getBusinessInfo|startup true/false|shutdown|format [-f]|}"
  echo "Usage: $0 {ignoreDB add/del databaseName | setBackupDir backupDirName}"
  echo "Usage: $0 {setConfig -local_heartbeat_IP local_heartbeat_IP -peer_heartbeat_IP peer_heartbeat_IP}"
  echo "Usage: $0 {install filename | expand filename | migrateManager filename | modifyServiceConfig bond_name old_ip old_netmask old_gateway new_ip new_netmask new_gateway}"
  exit 1
  ;;
esac


