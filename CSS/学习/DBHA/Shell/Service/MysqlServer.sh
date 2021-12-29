#!/bin/bash
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh

mysql_cycle_sync_shell=$parent_dir/Common/CycleSyncMySQLLog.sh
purge_bin_log_path=$DBHA_SHELL/Binlog/PurgeBinlog.sh
mysql_state_file=$DBHA_CONFIG/Sql.stat
dbha_config=$DBHA_CONFIG/DBHA.conf
ha_state=$DBHA_CONFIG/HAState.stat

TAG=mysqlServer

mysql_cfg=/etc/my.cnf

# 判断操作系统类型
# systemV为使用init作为一号进程的系统，如CentOS6.x
# systemd为使用systemd作为一号进程的系统，如CentOS7.x
function is_systemd()
{
    if [ -f /usr/lib/systemd/systemd ] && [ X"$(pidof systemd)" == X"1" ]; then
        return 0
    fi
    return 1
}

function mysqld_operation()
{
    #根据系统不同，使用不同的命令
    oper_cmd=$1
    is_systemd
    if [ $? -eq 1 ];then
        /etc/init.d/mysqld $oper_cmd
    else
        mysql_server_rpm_el7=`rpm -qa |grep -i mysql |grep server |grep el7`
        if [ "$mysql_server_rpm_el7" != "" ];then
            if [ "$oper_cmd"  == "stop" ];then
                nohup systemctl stop mysqld.service > /dev/null 2>&1 &
            else
                systemctl $oper_cmd mysqld.service
            fi
        else
            /etc/init.d/mysqld $oper_cmd
        fi
    fi
}

function recoveryMysql()
{
	data_dir=`cat $mysql_cfg | grep "datadir" | awk -F '= |=' '{print $2}'`
	data_parent_dir=`dirname $data_dir`
	mv $data_dir $data_parent_dir/data.bak
	tar xvf $DBHA_SHELL/mysql.tar.gz -C $data_parent_dir
	mv $data_parent_dir/mysql $data_parent_dir/data
	sed -i -c "s/\"used_state\".*/\"used_state\" : 2/g" $ha_state
}

function startPurgeBinLog()
{
    num=`crontab -l |grep PurgeBinlog | wc -l`
    if [[ $num -eq 0 ]];then
        logInfo "add purge binlog"
        chmod +x $purge_bin_log_path
        #sed -i '$a*\/20 * * * * \/cloud\/dahua\/DataBaseServer\/DBHA\/Shell\/Binlog\/PurgeBinlog.sh' /var/spool/cron/root
		echo '*\/20 * * * * \/cloud\/dahua\/DataBaseServer\/DBHA\/Shell\/Binlog\/PurgeBinlog.sh' >> /var/spool/cron/root
    else
        logInfo "purge binlog is already added"
    fi
}

function stopPurgeBinLog()
{
    num=`crontab -l |grep PurgeBinlog | wc -l`
    for ((i=0;i<$num;i++));
    do
        logInfo "delete purge binlog"
        sed -i '/\/cloud\/dahua\/DataBaseServer\/DBHA\/Shell\/Binlog\/PurgeBinlog/d' /var/spool/cron/root
        sed -i '/\/cloud\/dahua\/DataBaseServer\/DBHA\/Shell\/purgeBinlog/d' /var/spool/cron/root
    done
}

function cleaup()
{
    #主机上出现的任务
    sed -i '/\/cloud\/dahua\/DataBaseServer\/DBHA\/Shell\/masterBackup/d' /var/spool/cron/root
    sed -i '/\/cloud\/dahua\/DataBaseServer\/DBHA\/Shell\/Backup\/MasterBackup/d' /var/spool/cron/root
    #变成只读
    #cd $DBHA_SHELL
    #./$config_tool $mysql_state_file '{"SQL_write_state": 0}'

    #尝试清理残留的备份tmp文件,todo后续可以精确匹配
    path=`cat $dbha_config |grep -w "mysql_backup_path" | awk -F": |:|," '{print $2}' | awk -F"\"" '{print $2}'`
    rm -f $path/*_tmp
}

function killMysqld()
{
	logInfo "kill all mysql"
	mysqld_operation stop
	
	mysql_pid=$(pidof mysqld)
	wait_timeout=10
	wait_count=0
	while [ "$mysql_pid"X != ""X ] && [ $wait_count -lt $wait_timeout ];do
		sleep 3
		mysql_pid=$(pidof mysqld)
		wait_count=$((wait_count + 1))
	done
	
	pids=`ps -def |grep mysqld |grep -v "grep" | grep -v "/etc/init.d/mysqld" | grep -v "systemctl" | awk -F " " '{print $2}'`
	for pid in ${pids[@]};
    do
		pid_name=`ps -ef|grep $pid |grep -v "grep" | awk -F " " '{print $8}'`
        logInfo "kill $pid $pid_name by force"
        kill -9 $pid
    done
}
#获取mysql进程是否存在,存在返回0，不存在返回1
function status()
{
    #如果mysqld检测了10次还未启动成功，则直接使用空白的预留文件进行mysql文件恢复
    total_num=1
    #10s检测mysql status是否正常运行
    for((i=1;i<=$total_num;i++));
    do
        #查看MySQL状态，区分Centos6和Centos7
        if [ -f /etc/init.d/mysqld ];then
            /etc/init.d/mysqld status
        else
            systemctl status mysqld.service > /dev/null 2>&1
        fi

        if [[ $? -eq 0 ]];then
            return 0
        fi
        sleep 1
    done
    
    logInfo "mysqld status maybe has some problems."
    #mysql正在启动中
    #service的方式启动判断
    mysql_start_value=`ps -ef|grep "/etc/init.d/mysqld start" |grep -v "grep"`
    if [ ! -z "$mysql_start_value" ];then
        logInfo "mysqld is starting,waiting..."
        return 0
    fi
    
    #systemctl的方式启动判断
    mysql_start_value=`ps -ef|grep "systemctl start mysqld.service" |grep -v "grep"`
    if [ ! -z "$mysql_start_value" ];then
        logInfo "mysqld is starting,waiting..."
        return 0
    fi

    logInfo "mysqld is not starting."
    #mysqld不存在
    mysqld_pids_tmp=`pidof mysqld`
    mysqld_pids=($mysqld_pids_tmp)

    #当status不为0的时候，有可能mysqld正在启动中
    if [ ${#mysqld_pids[@]} -eq 1 ];then
        logInfo "mysqld status is not equal with 0, but mysqld num is equal with 1"
        return 0
    else #当进程不是1个，则认为异常
        logInfo "mysqld num is not equal with 1"
        return 1
    fi
}

function create_mysql_pem()
{
    mysql_data_dir=$(cat $mysql_cfg | grep "datadir" | awk -F '= |=' '{print $2}')
    pem_exist=$(ls $mysql_data_dir | grep "pem" | wc -l)
	if [ $pem_exist -eq 0 ];then
        logInfo "create $mysql_data_dir pem"
        /usr/bin/mysql_ssl_rsa_setup --datadir=$mysql_data_dir >> $LOG_FILE 2>&1;
        chgrp -R mysql $mysql_data_dir && chown -R mysql $mysql_data_dir
		return 0
    fi

	server_cert_path=$mysql_data_dir/server-cert.pem
	if [ -f "$server_cert_path" ];then
		pem_startdate=$(openssl x509 -startdate -noout -in $server_cert_path | awk -F "notBefore=" '{print $2}')
		pem_enddate=$(openssl x509 -enddate -noout -in $server_cert_path | awk -F "notAfter=" '{print $2}')
		if [ ! -z "$pem_startdate" ] && [ ! -z "$pem_enddate" ];then
			pem_start_seconds=$(date -d "$pem_startdate" +%s)
			pem_end_seconds=$(date -d "$pem_enddate" +%s)
			current_seconds=$(date +%s)
			if [ $current_seconds -lt $pem_start_seconds ] || [ $current_seconds -gt $pem_end_seconds ];then
				logInfo "invalid certificate,start $pem_start_seconds end $pem_end_seconds now $current_seconds, create $mysql_data_dir pem"
				rm -rf $mysql_data_dir/*.pem
				/usr/bin/mysql_ssl_rsa_setup --datadir=$mysql_data_dir >> $LOG_FILE 2>&1;
				chgrp -R mysql $mysql_data_dir && chown -R mysql $mysql_data_dir
			fi
		fi	
	fi
    
    return 0
}

function start()
{
	#启动mysql之前强制关闭一下
	killMysqld

	logInfo "try to restart mysqld server by self."
	#尝试清理残留的备份tmp文件,todo后续可以精确匹配
	path=`cat $dbha_config |grep -w "mysql_backup_path" | awk -F": |:|," '{print $2}' | awk -F"\"" '{print $2}'`
	rm -f $path/*_tmp
    
	create_mysql_pem
	mysqld_operation start
	status
	if [[ $? -eq 0 ]];then
	    #启动binlog定时删除功能
		startPurgeBinLog
		logInfo "start mysqld success"
		return 0;
	fi

	#mysql有问题建议人工修复
	logInfo "attention:start mysqld error."
	mysql_data_dir=`cat /etc/my.cnf | grep datadir | awk -F "=" '{print $2}' | sed 's/[ ]//g'`
	if [ ! -e "$mysql_data_dir/mysql" ];then
		logInfo "$mysql_data_dir/mysql not exist,recovery it!"
		#recoveryMysql
		#$MYSQL_SERVICE start
	fi
	
	return 1
}

function stop()
{
    #清理中间出现过的任何定时任务
    cleaup

    #停止binlog定时删除功能
    stopPurgeBinLog

    #关闭mysqld
    #mysqld_operation stop
    killMysqld
	
	#日志转储到/cloud/dahua/DataBaseServer/DBHA/Log/mysql_backup_dir下
	nohup /bin/bash $mysql_cycle_sync_shell > /dev/null 2>&1 &

	logInfo "stop mysqld success"
    return 0
}

function mode()
{
    #主备选举判断时判断本机的数据库状态是主还是备，同时监控数据库状态
    retry=2
    for((i=1;i<=$retry;i++));do
        getUserCfg
        status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" 2>/dev/null)
        if [ $? -ne 0 ];then
            sleep 1
            continue
        else
            break
        fi
    done
    
    if [ $i -gt $retry ];then
        return 1
    fi
    
    if [ -z ${status} ];then
        echo "master"
    else
        echo "slave"
    fi
    return 0
}

case $1 in
  "start")
  start
  exit $?
  ;;
  "stop")
  stop
  exit $?
  ;;
  "status")
  status
  exit $?
  ;;
  "mode")
  mode
  exit $?
  ;;
  *)
  echo "Usage: $0 {start|stop|status|mode}"
  exit 1
  ;;
esac