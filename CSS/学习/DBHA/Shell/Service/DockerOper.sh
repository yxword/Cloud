#!/bin/bash
#chkconfig: 2345 60 40
#description: DBHA Docket Operation
 
# made by lw 228088 20200801
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/DBHAConfig.sh
source $DBHA_SHELL/Common/CommonFunc.sh

mysql_data_dir=$(cat /etc/my.cnf|grep "datadir"| awk -F "=" '{print $2}' | sed 's/[ ]//g')

DOCKER_SHELL_SCRIPT=$DBHA_SHELL/Service/MysqlKeeper.sh

DOCKER_DBHA_BIN=$DBHA_BIN
DOCKER_DBHA_SHELL=$DBHA_SHELL
DOCKER_MONIT_SHELL=
DOCKER_SERVER_NAME=
DOCKER_BIN_NAME=
DOCKER_DBHA_CONFIG=
DOCKER_CONFIG_TOOL=$CONFIG_TOOL
DOCKER_CONFIG_PATH=
DOCKER_MYSQL_SERVICE=
DOCKER_MYSQL_MONIT_SHELL=MysqlKeeper.sh
DOCKER_DBHA_CONFIG_BACKUP_PATH=/data/mysql/Config
DOCKER_DBHA_LOG_PATH=/data/mysql/log

#获取参数
function dockerParam(){
	item=$1
	value=$2
	
	case $item in
		"MONIT_SHELL")
			DOCKER_MONIT_SHELL=$value
			;;
		"SERVER_NAME")
			DOCKER_SERVER_NAME=$value
			;;
		"BIN_NAME")
			DOCKER_BIN_NAME=$value
			;;
		"DBHA_CONFIG")
			DOCKER_DBHA_CONFIG=$value
			;;
		"CONFIG_PATH")
			DOCKER_CONFIG_PATH=$value
			;;
		"MYSQL_SERVICE")
			DOCKER_MYSQL_SERVICE=$value
			;;
		*)
	esac
}

# 获取系统服务状态
function dockerGetServerStatus(){
	# 检查守护脚本和主进程
	local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MONIT_SHELL | awk '{print $2}'`
	server_pid=`pidof $DOCKER_BIN_NAME`

	#守护脚本和主进程都在
	if [ ! -z "$keep_exist" ] && [ ! -z "$server_pid" ];then
		echo "$DOCKER_MONIT_SHELL : running" 
		echo "$DOCKER_SERVER_NAME : running" 
		return 0
	# 守护脚本和主进程都不在
	elif [ -z "$keep_exist" ] && [ -z "$server_pid" ];then
		echo "$DOCKER_MONIT_SHELL : stopped" 
		echo "$DOCKER_SERVER_NAME : stopped" 
		return 1
	# 守护脚本在，主进程不在
	elif [ ! -z "$keep_exist" ] && [ -z "$server_pid" ];then
		echo "$DOCKER_MONIT_SHELL : running" 
		echo "$DOCKER_SERVER_NAME : stopped" 
		if [ -d /usr/lib/systemd/system ]; then 
			#centos7.7 返回为 0 
			# centos7.7 服务启动start返回 非0 ，会把keeper 都杀死 ，导致服务停止。所以7.7 系统如果keeper 启动成功认为服务启动成功
			return 0 
		else 
			return 2
		fi
	# 守护脚本不在，主进程在
	elif [ -z "$keep_exist" ] && [ ! -z "$server_pid" ];then
		echo "$DOCKER_MONIT_SHELL : stopped" 
		echo "$DOCKER_SERVER_NAME : running" 
		return 2
	fi
	return 2
}

function dockerStartKeep(){
	# 检查守护脚本和主进程
	local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MONIT_SHELL | awk '{print $2}'`

	#守护脚本和主进程都不在
	if [ -z "$keep_exist" ];then
		logInfo "[dockerStartKeep]try to start $DOCKER_MONIT_SHELL and $DOCKER_SERVER_NAME" 
		nohup /bin/sh $DOCKER_DBHA_SHELL/Service/$DOCKER_MONIT_SHELL > /dev/null 2>&1 &
	fi
}

function dockerStopKeep(){
	local try_max_count=15
	local try_count=1
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MONIT_SHELL | awk '{print $2}'`
		if [ ! -z "$keep_exist" ];then
			kill -9 $keep_exist
			logInfo "[dockerStopKeep]stop $DOCKER_MONIT_SHELL"
			sleep 1
		else
			break
		fi
	done
}

function dockerStartMysqlKeep(){
	# 检查守护脚本和主进程
	local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MYSQL_MONIT_SHELL | awk '{print $2}'`

	#守护脚本和主进程都不在
	if [ -z "$keep_exist" ];then
		logInfo "[dockerStartMysqlKeep]try to start $DOCKER_MYSQL_MONIT_SHELL" 
		nohup /bin/sh $DOCKER_DBHA_SHELL/Service/$DOCKER_MYSQL_MONIT_SHELL > /dev/null 2>&1 &
	fi
	
	local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MYSQL_MONIT_SHELL | awk '{print $2}'`
	if [ ! -z "$keep_exist" ];then
		echo "$DOCKER_MYSQL_MONIT_SHELL : running" 
	fi
}

function dockerStopMysqlKeep(){
	local try_max_count=15
	local try_count=1
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MYSQL_MONIT_SHELL | awk '{print $2}'`
		if [ ! -z "$keep_exist" ];then
			kill -9 $keep_exist
			logInfo "[dockerStopMysqlKeep]stop $DOCKER_MYSQL_MONIT_SHELL"
			sleep 1
		else
			break
		fi
	done
	
	local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MYSQL_MONIT_SHELL | awk '{print $2}'`
	if [ -z "$keep_exist" ];then
		echo "$DOCKER_MYSQL_MONIT_SHELL : stopped" 
	fi
}

#0表示为修改，1表示修改了
function dockerWriteConfig(){
	item=$1
	value=$2

	if [ -f "$DOCKER_CONFIG_PATH" ];then
		pre_value=`cat ${DOCKER_CONFIG_PATH} | grep \"$item\" | awk -F '"' '{print $4}'`
		if [[ "$pre_value" = "$value" ]];then
			logInfo "[dockerWriteConfig]$item: pre value($pre_value) is same as value($value)"
			return 0
		fi
	fi

	echo "$item $value"
	cd $DOCKER_DBHA_SHELL
	$DOCKER_CONFIG_TOOL $DOCKER_CONFIG_PATH	 '{"'$item'" : "'$value'"}'
	reboot_flag=1
	return 1
}

function dockerStopHAForce()
{
	local try_max_count=15
	local try_count=1
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		server_pid=`pidof $DOCKER_BIN_NAME`
		if [ ! -z "$server_pid" ];then
			kill -9 $server_pid
			logInfo "[dockerStopHAForce]stop $DOCKER_BIN_NAME force"
			sleep 1
		else
			break
		fi
	done
}

function dockerWriteConfigInt(){
	item=$1
	value=$2
	
	if [ -f "$DOCKER_CONFIG_PATH" ];then
		pre_value=`cat ${DOCKER_CONFIG_PATH} | grep \"$item\" | awk -F '"' '{print $4}'`
		if [[ "$pre_value" = "$value" ]];then
			logInfo "[dockerWriteConfigInt]$item: pre value($pre_value) is same as value($value)"
			return 0
		fi
	fi

	echo "$item $value"
	cd $DOCKER_DBHA_SHELL
	$DOCKER_CONFIG_TOOL $DOCKER_CONFIG_PATH	 '{"'$item'" : '$value'}'
	reboot_flag=1
	return 1
}

function dockerAddOrUpdateCfg()
{
	cfg_key=$1
	sed -i "/^$cfg_key/d" /etc/my.cnf
	
	if [ $# -eq 1 ]; then
		sed -i "/^\[mysqld\]/a $cfg_key" /etc/my.cnf
	elif [ $# -eq 2 ]; then
		sed -i "/^\[mysqld\]/a $cfg_key = $2" /etc/my.cnf
	elif [ $# -eq 3 ]; then
		sed -i "/^\[mysqld\]/a $cfg_key = $2" /etc/my.cnf
		sed -i "/^\[mysqld\]/a $cfg_key = $3" /etc/my.cnf
	fi
	return 0
}

#将数据库read_only修改为OFF
function dockerSetReadOnly()
{
    if [ $# -eq 0 ];then
        mysql_conn_ip=127.0.0.1
    else
        mysql_conn_ip=$1
    fi
    
    logInfo "[dockerSetReadOnly]MySQL:$mysql_conn_ip SetReadOnly OFF"
    
	local try_max_count=30
	local try_count=1
	for((try_count=1;try_count<=try_max_count;try_count++))
	do
		sleep 1
		mysqladmin -u$mysql_user -p$mysql_password -h$mysql_conn_ip --connect-timeout=1 ping 2>&1 | grep -q "mysqld is alive"
		if [ $? -eq 0 ];then
			break
		fi
	done
		
	mysqladmin -u$mysql_user -p$mysql_password -h$mysql_conn_ip --connect-timeout=1 ping 2>&1 | grep -q "mysqld is alive"
	if [ $? -ne 0 ];then
		logInfo "[dockerSetReadOnly]mysqld is not alive,startDocker failed"
		#容器里面只对数据库目录进行了持久化，因此数据库的密码可能已经是修改过的，但DBHA还没有
		return 1
	fi
			
	#修改配置信息并设置到配置文件中去
	mysql -u$mysql_user -p$mysql_password -e "set global read_only=0;"
	
	dockerAddOrUpdateCfg "read_only" "0"
}

#修改mysql的默认密码
function dockerChangeDefaultPassword()
{
    local MYSQL_CONF=$DOCKER_DBHA_CONFIG/Mysql.conf
	new_password=$(env | grep docker_magic_mysql_password | awk -F '=' '{print $2}')
	if [ ! -z "$new_password" ] && [ "$mysql_password"X != "$new_password"X ];then
		logInfo "[dockerChangeDefaultPassword]setPassword $new_password"
		try_max_count=30
		for((try_count=1;try_count<=try_max_count;try_count++))
		do
			sleep 1
			mysqladmin -u$mysql_user -p$mysql_password --connect-timeout=1 ping 2>&1 | grep -q "mysqld is alive"
			if [ $? -eq 0 ];then
				break
			fi
		done
		
		mysqladmin -u$mysql_user -p$mysql_password --connect-timeout=1 ping 2>&1 | grep -q "mysqld is alive"
		if [ $? -ne 0 ];then
			logInfo "[dockerChangeDefaultPassword]mysqld is not alive,startDocker failed"
			#容器里面只对数据库目录进行了持久化，因此数据库的密码可能已经是修改过的，但DBHA还没有
			return 1
		fi
		
		mysqladmin -u$mysql_user -p$mysql_password password $new_password
		if [ $? -ne 0 ];then
			logInfo "[dockerChangeDefaultPassword]set mysql passwd $new_password failed"
		else
			logInfo "[dockerChangeDefaultPassword]set mysql passwd $new_password success"
			
			#修改配置信息并设置到配置文件中去
			local base64_user=`echo "${mysql_user}" | base64`
			local base64_pwd=`echo "${new_password}" | base64`
			$DOCKER_CONFIG_TOOL $MYSQL_CONF '{"user_security" : "'$base64_user'"}'
			$DOCKER_CONFIG_TOOL $MYSQL_CONF '{"password_security" : "'$base64_pwd'"}'
			$DOCKER_CONFIG_TOOL $MYSQL_CONF '{"security_has_changed" : 1}'
            #持久化
			cp -rf $MYSQL_CONF $DOCKER_DBHA_CONFIG_BACKUP_PATH
		fi
	fi
}

#设置双副本时DBHA参数
function dockerSetDBHAParam()
{
	jq_tool=$DOCKER_DBHA_BIN/jq
	dbha_vip=$(env | grep magic_vip | awk -F '=' '{print $2}')
	host_name=$(hostname)
    
    business_if_name=$(env | grep docker_magic_business_if_name | awk -F "=" '{print $2}')
	heartbeat_if_name=$(env | grep docker_magic_heartbeat_if_name | awk -F "=" '{print $2}')
    
    #小集群环境
    local node_list=$(env | grep cluster_node_list)
    if [ ! -z "$node_list" ];then
        cluster_node_list_ip_1=$(env | grep cluster_node_list | awk -F '=' '{print $2}' | $jq_tool -r '.clusterInfo.role[0]')
        cluster_node_list_ip_1_host=$(echo $cluster_node_list_ip_1 | $jq_tool -r '.podName')
        cluster_node_list_ip_2=$(env | grep cluster_node_list | awk -F '=' '{print $2}' | $jq_tool -r '.clusterInfo.role[1]')
        cluster_node_list_ip_2_host=$(echo $cluster_node_list_ip_2 | $jq_tool -r '.podName')
    
    	if [ "$host_name"X == "$cluster_node_list_ip_1_host"X  ];then
            cluster_node_list_ip_local=$cluster_node_list_ip_1
            cluster_node_list_ip_remote=$cluster_node_list_ip_2
        else
            cluster_node_list_ip_local=$cluster_node_list_ip_2
            cluster_node_list_ip_remote=$cluster_node_list_ip_1
        fi

        logInfo "[dockerSetDBHAParam]vip=$dbha_vip, business_if_name=$business_if_name, heartbeat_if_name=$heartbeat_if_name, cluster_node_list_ip_local=$cluster_node_list_ip_local, cluster_node_list_ip_remote=$cluster_node_list_ip_remote"
        if [ -z "$dbha_vip" ] || [ -z "$business_if_name" ] || [ -z "$heartbeat_if_name" ] || [ -z "$cluster_node_list_ip_local" ];then
            logInfo "[dockerSetDBHAParam]docker configure is empty, exit"
            return 1
        fi
        
        local local_host_ip=`echo $cluster_node_list_ip_local | $jq_tool -r ".netCard.${business_if_name}.ip"`
        local peer_host_ip=`echo $cluster_node_list_ip_remote | $jq_tool -r ".netCard.${business_if_name}.ip"`
        local local_heartbeat_ip=`echo $cluster_node_list_ip_local | $jq_tool -r ".netCard.${heartbeat_if_name}.ip"`
        local peer_heartbeat_ip=`echo $cluster_node_list_ip_remote | $jq_tool -r ".netCard.${heartbeat_if_name}.ip"`
        if [ -z "$local_host_ip" ] || [ -z "$local_heartbeat_ip" ];then
            logInfo "[dockerSetDBHAParam]ip configure is empty, local_host_ip=$local_host_ip, local_heartbeat_ip=$local_heartbeat_ip"
            return 1
        fi
    else
        #容器云环境
        node_list=$(env | grep magic_node_list)
        if [ ! -z "$node_list" ];then
            node_list_ip_1=$(env | grep magic_node_list | awk -F '"' '{print $2}' | awk -F ':' '{print $1}')
            node_list_ip_2=$(env | grep magic_node_list | awk -F ',' '{print $2}' | awk -F '"' '{print $2}' | awk -F ':' '{print $1}')
            local vif_name=`ip addr | grep "$node_list_ip_1\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
            if [ "$vif_name" != "" ];then
                local local_host_ip=$node_list_ip_1
                local peer_host_ip=$node_list_ip_2
                local local_heartbeat_ip=$local_host_ip
                local peer_heartbeat_ip=$peer_host_ip
            else
                vif_name=`ip addr | grep "$node_list_ip_2\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
                if [ "$vif_name" != "" ];then
                    local local_host_ip=$node_list_ip_2
                    local peer_host_ip=$node_list_ip_1
                    local local_heartbeat_ip=$local_host_ip
                    local peer_heartbeat_ip=$peer_host_ip
                fi
            fi
        fi
        
        logInfo "[dockerSetDBHAParam]vip=$dbha_vip, business_if_name=$business_if_name, heartbeat_if_name=$heartbeat_if_name, local_host_ip=$local_host_ip, peer_host_ip=$peer_host_ip"
        if [ -z "$dbha_vip" ] || [ -z "$business_if_name" ] || [ -z "$local_host_ip" ] || [ -z "$peer_host_ip" ];then
            logInfo "[dockerSetDBHAParam]docker configure is empty, exit!"
            return 1
        fi
	fi

	DBHA_virtual_IP_netmask=`ifconfig ${business_if_name} | grep netmask | awk -F "netmask" '{print $2}' | awk -F "broadcast" '{print $1}' | sed 's/ //g'`
	_DBHA_vif_name=$business_if_name:11
	_DBHA_expand_flag=-1
	_DBHA_install_flag=1
	
	dockerWriteConfig local_business_IP $local_host_ip
	dockerWriteConfig local_heartbeat_IP $local_heartbeat_ip
	dockerWriteConfig peer_business_IP $peer_host_ip
	dockerWriteConfig peer_heartbeat_IP $peer_heartbeat_ip
	dockerWriteConfig DBHA_heartbeart_if_name $heartbeat_if_name
	dockerWriteConfig DBHA_virtual_IP $dbha_vip
	dockerWriteConfig DBHA_vif_name $_DBHA_vif_name
	dockerWriteConfig DBHA_if_name $business_if_name
	dockerWriteConfig DBHA_virtual_IP_netmask  $DBHA_virtual_IP_netmask
	dockerWriteConfigInt DBHA_expand_flag $_DBHA_expand_flag
	dockerWriteConfigInt DBHA_install_flag $_DBHA_install_flag
	dockerWriteConfigInt DBHA_same_binlog_diff_pos_high_threshold 0
	dockerWriteConfigInt DBHA_same_binlog_diff_pos_low_threshold 0
}

function dockerCheckCrond()
{
    crond_exist=`ps -ef | grep /usr/sbin/crond | grep start`
    if [ -z "$crond_exist" ];then
        logInfo "[dockerCheckCrond]crond not start,begin start"
        /usr/sbin/crond start
        crond_exist=`ps -ef | grep /usr/sbin/crond | grep start`
        if [ ! -z "$crond_exist" ];then
            logInfo "[dockerCheckCrond]crond start success"
        else 
            logInfo "[dockerCheckCrond]crond start failed"
        fi
    fi
}

#启动docker
function dockerStartDocker()
{
	ulimit -n 10240
	ulimit -c unlimited
    
    #创建目录
    if [ ! -d $DOCKER_DBHA_CONFIG_BACKUP_PATH ];then
        mkdir -p $DOCKER_DBHA_CONFIG_BACKUP_PATH
    fi
    
    if [ ! -d $DOCKER_DBHA_LOG_PATH ];then
        mkdir -p $DOCKER_DBHA_LOG_PATH
    fi

	#修改innodb缓冲池大小
	docker_memory_size=$(cat /sys/fs/cgroup/memory/memory.limit_in_bytes)
	if [ -z "$docker_memory_size" ];then
		mysql_memory_size=1024
	else
		mysql_memory_size=$(($docker_memory_size*3/5/1024/1024))
		#避免分配2G内存时，buffer实际分配2G内存
        #1500对应的内存大小为2500MB,低于2500MB,buffer分配1G,高于2500MB,可根据实际情况向上兼容。如内存分配2600MB,buffer分配2G,内存仍有剩余600MB
		if [ $mysql_memory_size -gt 1024 ] && [ $mysql_memory_size -lt 1500 ];then
			mysql_memory_size=1024
		fi
	fi
	dockerAddOrUpdateCfg "innodb_buffer_pool_size" "${mysql_memory_size}M"
	logInfo "[dockerStartDocker]set innodb_buffer_pool_size:${mysql_memory_size}M"
	
	#修改MySQL的wait_timeout参数
	mysql_wait_timeout=$(env | grep docker_magic_wait_timeout | awk -F "=" '{print $2}')
	if [ "$mysql_wait_timeout" != "" ] && [ "$mysql_wait_timeout" != "0" ];then
		dockerAddOrUpdateCfg "wait_timeout" "$mysql_wait_timeout"
		logInfo "[dockerStartDocker]set wait_timeout:${mysql_wait_timeout}"
	fi
	
	#修改备份地址
	mysql_data_backup_tmp=`echo "/data/mysql/backup" | sed 's#\/#\\\/#g'`
	dockerWriteConfig mysql_backup_path $mysql_data_backup_tmp
    
    #log日志位置重新创建软连接
    rm -rf $DBHA_LOG
    ln -s $DOCKER_DBHA_LOG_PATH  $DBHA_LOG
    
    #恢复Mysql.conf文件
    #由于docker中仅对数据库的目录进行持久化,因此需要把持久化文件拷贝回来
    if [ -f $DOCKER_DBHA_CONFIG_BACKUP_PATH/Mysql.conf ];then
        rm -rf $DOCKER_DBHA_CONFIG/Mysql.conf
        cp $DOCKER_DBHA_CONFIG_BACKUP_PATH/Mysql.conf $DOCKER_DBHA_CONFIG
    fi
    
    getUserCfg
    
    #检测Crond是否运行
    dockerCheckCrond

	#docker_magic_double_duplicate为No，则为单副本
	double_duplicate=$(env | grep docker_magic_double_duplicate | awk -F "=" '{print $2}')
	if [ "$double_duplicate" == "No" ];then
		logInfo "[dockerStartDocker]install DBHA with one duplicate."
		
		#拉起守护,守护会拉起mysql
		dockerStartMysqlKeep
		#将数据库read_only修改为OFF
		dockerSetReadOnly
		#修改mysql密码
		dockerChangeDefaultPassword
		
		logInfo "[dockerStartDocker]One duplicate:restart service without DBHA"
		#停止服务
		dockerStopMysqlKeep
		sh $DOCKER_MYSQL_SERVICE stop
		if [[ $? -ne 0 ]];then
			logInfo "[dockerStartDocker]mysqld stop failed."
			return 1
		fi
		
		#镜像的mysql-uuid可能都是一样的，需要重新设置下
		rm -rf $mysql_data_dir/auto.cnf
		sleep 3
		
		#启动守护脚本
		dockerStartMysqlKeep
		local keep_exist=`ps -ef | grep /bin/sh | grep $DOCKER_MYSQL_MONIT_SHELL | awk '{print $2}'`
		#守护脚本和主进程都在
		if [ ! -z "$keep_exist" ];then
			logInfo "[dockerStartDocker]$DOCKER_MYSQL_MONIT_SHELL is running"
		fi
        
        #起备份任务
        local backup_script=MasterBackup.sh
        sed -i '/MasterBackup/d' /var/spool/cron/root
        local backup_full_path=$parent_dir/Backup/$backup_script
        chmod +x $backup_full_path
        local script_tmp=`echo $backup_full_path | sed 's#\/#\\\/#g'`
        
        echo '0 1 * * * '$script_tmp' '\/data\/mysql\/backup' '1' '1'' >>/var/spool/cron/root
        logInfo "[dockerStartDocker]add cron task(master_backup)."

		return 0
	else
		logInfo "[dockerStartDocker]install DBHA with two duplicate."
		
		dockerJudgeRunDBHA
		if [[ $? -ne 0 ]];then
			logInfo  "[dockerStartDocker]two duplicate without DBHA,because of NOT AUTO SWITCH!!!."
			return 0
        else
            logInfo  "[dockerStartDocker]two duplicate with DBHA!!!"
		fi
		
		#拉起守护,守护会拉起mysql
		dockerStartKeep
		#增加日志转储功能
		#addCycleLog
		#修改默认密码
		dockerChangeDefaultPassword
		#设置DBHA配置文件
		dockerSetDBHAParam
	fi

	return 2
}

#主备启动正常后，判断是否关闭DBHA
function dockerSetDBHASwitch()
{
    local dbha_auto_switch=$(env | grep docker_magic_dbha_auto_switch | awk -F "=" '{print $2}')
    if [ "$dbha_auto_switch" == "No" ];then
        logInfo "[dockerSetDBHASwitch]start close DBHA,because of NOT AUTO SWITCH."
        #判断虚IP是否启动正常,正常启动则说明主备设置成功，开始进行下一步
        local peer_business_IP=`cat $DOCKER_CONFIG_PATH |grep "peer_business_IP"|awk -F "\"" '{print $4}'`
        local local_business_IP=`cat $DOCKER_CONFIG_PATH |grep "local_business_IP"|awk -F "\"" '{print $4}'`
        local dbha_vip=$(env | grep magic_vip | awk -F '=' '{print $2}')
        local cmd=`ip addr | grep "$peer_business_IP\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
        
        local try_max_count=30
        local try_count=0
        for((try_count=0;try_count<try_max_count;try_count++))
        do
            sleep 1
            local DBHA_vif_name=`ip addr | grep "$dbha_vip\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
            if [ -z "$DBHA_vif_name" ];then
                DBHA_vif_name=`ssh $peer_business_IP ip addr | grep "$dbha_vip\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
            else
                break
            fi
        
            if [ ! -z "$DBHA_vif_name" ];then
                logInfo "[dockerSetDBHASwitch] vip is normal!"
                break
            fi
        done
    
        if [ $try_count -eq $try_max_count ];then
            logInfo  "[dockerSetDBHASwitch]vip not exist,return!"
            return 1
        fi
        
        #查看备机状态--备机未正常同步则返回
        try_count=0
        for((try_count=0;try_count<try_max_count;try_count++))
        do
            sleep 1
            local local_io_status=$(mysql -u$mysql_user -p$mysql_password -h$local_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
            local local_slave_status=$(mysql -u$mysql_user -p$mysql_password -h$local_business_IP -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
            local peer_io_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
            local peer_slave_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
            if [ "$local_io_status" == "" ];then
                if [[ "$peer_io_status" == "Yes" ]] && [[ "$peer_slave_status" == "Yes" ]];then
                    break
                fi
            else
                if [[ "$local_io_status" == "Yes" ]] && [[ "$local_slave_status" == "Yes" ]];then
                    break
                fi
            fi
        done
        
        if [ $try_count -eq $try_max_count ];then
            logInfo  "[dockerSetDBHASwitch]the state of slave abnormal,return!!!"
            return 1
        fi
        
        #将DBHA的配置文件备份到持久化盘中，用于下次启动判断
        cp $DOCKER_DBHA_CONFIG/DBHA.conf $DOCKER_DBHA_CONFIG_BACKUP_PATH
        cp $DOCKER_DBHA_CONFIG/HAConfig.conf $DOCKER_DBHA_CONFIG_BACKUP_PATH
        
        #解决mysql的配置文件/etc/my.cnf在重启后会丢失问题
        cp /etc/my.cnf $mysql_data_dir
        
        #关闭守护进程
        dockerStopKeep
        #关闭DBHA程序
        dockerStopHAForce
        #关闭DBHA开机自启动
        #chkconfig $DOCKER_SERVER_NAME off
        #if [ $? -eq 0 ];then
        #    logInfo "chkconfig $DOCKER_SERVER_NAME off success."
        #fi
        #拉起mysql守护进程
        dockerStartMysqlKeep
        logInfo "[dockerSetDBHASwitch]close DBHA success,because of NOT AUTO SWITCH."
    fi
}

#再次启动时，判断是否启动DBHA
function dockerJudgeRunDBHA()
{
    #是否开启自动切换
    local dbha_auto_switch=$(env | grep docker_magic_dbha_auto_switch | awk -F "=" '{print $2}')
    if [ "$dbha_auto_switch" == "No" ];then
        logInfo  "[dockerJudgeRunDBHA]DBHA not auto switch master and slave!!!"
        #是否已安装过DBHA
        if [ -f "$DOCKER_DBHA_CONFIG_BACKUP_PATH/DBHA.conf" ];then
            #转储数据库my.cnf待重启时恢复
            if [ -f $mysql_data_dir/my.cnf ];then
                rm -rf /etc/my.cnf
                cp $mysql_data_dir/my.cnf /etc
            fi
            #本端启动过DBHA,先关闭mysql服务，然后重启，因为配置文件替换了
            sh $DOCKER_MYSQL_SERVICE stop
            if [[ $? -ne 0 ]];then
                logInfo  "[dockerJudgeRunDBHA]mysqld stop failed."
                return 1
            fi
            
            #拉起mysql守护进程
            dockerStartMysqlKeep
            #拉起虚IP
            dockerUpVirtualIp
            return 1
        else
            logInfo "[dockerJudgeRunDBHA]First time deploy DBHA"
            return 0
        fi
    fi
    
    return 0
}

function dockerUpVirtualIp()
{
    local local_business_IP=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "local_business_IP"|awk -F "\"" '{print $4}'`
    local peer_business_IP=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "peer_business_IP"|awk -F "\"" '{print $4}'`
    
    #本端mysql是否可连接
    checkMysqlState $local_business_IP
    if [ $? -ne 0 ];then
        dockerStopMysqlKeep
        return 1
    fi
    
    #对端mysql是否可连接
    checkMysqlState $peer_business_IP
    if [ $? -ne 0 ];then
        dockerStopMysqlKeep
        sh $DOCKER_MYSQL_SERVICE stop
        if [[ $? -ne 0 ]];then
            logInfo  "[dockerUpVirtualIp]mysqld stop failed."
            return 1
        fi
        return 1
    fi
    
    #避免备机刚启动mysql，守护进程没有拉起slave，导致虚IP无法拉起。
    sleep 2 
    
    local local_io_status=$(mysql -u$mysql_user -p$mysql_password -h$local_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    local peer_io_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    local peer_slave_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    #本端无slave信息，认为是主机
    if [ "$local_io_status" == "" ];then 
        #对端有slave信息，认为是备机。
        #多次检测，避免备机的mysql仍在启动中导致提前退出
        local try_count=0
        local try_max_count=30
        for((try_count=0;try_count<try_max_count;try_count++))
        do
            sleep 1
            local peer_io_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
            local peer_slave_status=$(mysql -u$mysql_user -p$mysql_password -h$peer_business_IP -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
            if [[ "$peer_io_status" == "Yes" ]] && [[ "$peer_slave_status" == "Yes" ]];then
                break
            fi
        done
            
        if [ $try_count -eq $try_max_count ];then
            logInfo  "[dockerUpVirtualIp]The state of mysql abnormal,should recovery manual,exit!!!"
            return 1
        else
            upVirtualIP
            dockerSetReadOnly $local_business_IP
        fi
        
        #起备份任务
        startBackup
    fi
}

function startBackup()
{
    #获取masterback的路径
    local backup_script=MasterBackup.sh
    local mysql_backup_path=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "mysql_backup_path"|awk -F "\"" '{print $4}'`
    local uuid=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/HAConfig.conf |grep "uid"|awk -F "\"" '{print $4}'`
    local serverID=$(cat /etc/my.cnf|grep "server-id"| awk -F "=" '{print $2}' | sed 's/[ ]//g')
    
    sed -i '/MasterBackup/d' /var/spool/cron/root
    local backup_full_path=$parent_dir/Backup/$backup_script
    chmod +x $backup_full_path
    local script_tmp=`echo $backup_full_path | sed 's#\/#\\\/#g'`
    
    echo '0 1 * * * '$script_tmp' '$mysql_backup_path' '$uuid' '$serverID'' >>/var/spool/cron/root
    logInfo "[startBackup]add cron task(master_backup)"
}

function checkMysqlState()
{
    local mysql_host=$1
    local try_max_count=30
    local try_count=0
    for((try_count=0;try_count<try_max_count;try_count++))
    do
        sleep 1
        mysqladmin -u$mysql_user -p$mysql_password -h$mysql_host --connect-timeout=1 ping 2>&1 | grep -q "mysqld is alive"
        if [ $? -eq 0 ];then
            break;
        fi
    done
    
    if [ $try_count -eq $try_max_count ];then
        logInfo  "[checkMysqlState]MySQL:$mysql_host can not connected.exit!"
        return 1
    fi
    
    return 0
}

function upVirtualIP()
{
    local DBHA_vif_name=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "DBHA_vif_name"|awk -F "\"" '{print $4}'`
    local DBHA_if_name=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "DBHA_if_name"|awk -F "\"" '{print $4}'`
    local DBHA_virtual_IP=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "DBHA_virtual_IP\""|awk -F "\"" '{print $4}'`
    local DBHA_virtual_IP_netmask=`cat ${DOCKER_DBHA_CONFIG_BACKUP_PATH}/DBHA.conf |grep "DBHA_virtual_IP_netmask"|awk -F "\"" '{print $4}'`
    local vif_name=`ip addr | grep "$DBHA_virtual_IP\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
    if [ -z "$vif_name" ];then
        ifconfig $DBHA_vif_name $DBHA_virtual_IP netmask $DBHA_virtual_IP_netmask up > /dev/null 2>&1
        if [ $? -eq 0 ];then
            logInfo "[upVirtualIP]Virtual IP:$DBHA_virtual_IP up success"
            if [ ! -z "$DBHA_if_name" ];then
                arping -b -U -c 2 -I $DBHA_if_name $DBHA_virtual_IP -w 10
            fi
        else 
            logInfo "[upVirtualIP]Virtual IP:$DBHA_virtual_IP up failed"
        fi
    fi
}