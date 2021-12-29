#!/bin/bash
#配置mysql主备中的主机模式,
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

TAG=purgeBinlog
source $parent_dir/Common/CommonFunc.sh

bin_log_name=mysql-bin
relay_log_name=relay-log

#mysql登录的用户名和密码
getUserCfg
#获取binlog容量控制信息
getBinlogCfg

mysql_data=`cat /etc/my.cnf |grep datadir | awk -F "=|= " {'print $2'}`

#检测是主机还是备机,1表示主机，0表示备机
function checkMode()
{
    sql_running_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Host" | awk -F": " '{print $2}' )
    #主机则自动退出
    if [ "$sql_running_status" = "" ]; then
        #echo -e "master status is ok"
        return 1;
    fi
    return 0;
}

function masterPurge()
{
	binlog_cnt=$1
	relay_cnt=$2

	#优先删除relaylog
	if [[ $relay_cnt -gt 1 ]];then
		relay_log_1=`ls -alfrt $mysql_data |grep "$relay_log_name" | grep -v "index" | sed -n '1p'`
		sed -i '/'$relay_log_1'/d' $mysql_data/$relay_log_name.index
		rm $mysql_data/$relay_log_1
		logInfo "master purge relay log $mysql_data/$relay_log_1"
		return 0
	fi

	#再删除binlog,至少保留一个binlog
	if [[ $binlog_cnt -gt 1 ]];then
		bin_log_1=`ls -alfrt $mysql_data |grep "$bin_log_name" | grep -v "index" | sed -n '1p'`
		bin_log_2=`ls -alfrt $mysql_data |grep "$bin_log_name" | grep -v "index" | sed -n '2p'`
		mysql -u$mysql_user -p$mysql_password -e "purge binary logs to \"$bin_log_2\";"
		if [ -f "$mysql_data/$bin_log_1" ];then
			sed -i '/'$bin_log_1'/d' $mysql_data/$bin_log_name.index
			rm $mysql_data/$bin_log_1
		fi
		logInfo "master purge bin log $mysql_data/$bin_log_1"
	fi

	return 0
}

function slavePurge()
{
	binlog_cnt=$1
	relay_cnt=$2
    total_bin_log=$3
	total_relay_log=$4
	#先删除binlog,至少保留一个binlog
	#当binlog大于1个并且binlog的容量超越了，则需要删除一个
	if ([[ $binlog_cnt -gt 1 ]] && [[ $total_bin_log -gt $total_slave_bin_log_max ]]) || [[ $binlog_cnt -gt $total_bin_log_cnt_threshold ]];then
		bin_log_1=`ls -alfrt $mysql_data | grep "$bin_log_name" | grep -v "index" | sed -n '1p'`
		bin_log_2=`ls -alfrt $mysql_data | grep "$bin_log_name" | grep -v "index" | sed -n '2p'`
		mysql -u$mysql_user -p$mysql_password -e "purge binary logs to \"$bin_log_2\";"
		if [ -f "$mysql_data/$bin_log_1" ];then
			sed -i '/'$bin_log_1'/d' $mysql_data/$bin_log_name.index
			rm $mysql_data/$bin_log_1
		fi
		logInfo "slave purge bin log $mysql_data/$bin_log_1"
		return 0
	fi

	#再删除relay,至少保留一个relaylog
	#echo -e "slave aaa binlog_cnt:$binlog_cnt"
	if ([[ $relay_cnt -gt 1 ]] &&  [[ $total_relay_log -gt $total_slave_relay_log_max ]]) || [[ $relay_cnt -gt $total_bin_log_cnt_threshold ]];then
		relay_log_1=`ls -alfrt $mysql_data |grep "$relay_log_name" | grep -v "index" | sed -n '1p'`
		echo -e "delete  $relay_log_1 $mysql_data/$relay_log_name.index"
		sed -i '/'$relay_log_1'/d' $mysql_data/$relay_log_name.index
		rm $mysql_data/$relay_log_1
		logInfo "slave purge relay log $mysql_data/$relay_log_1"
	fi

	return 0
}

function checkLog()
{
	while (( 1 ));
	do
		binlog_cnt=`ls $mysql_data/$bin_log_name* | grep -v "index" | wc -l`
		relay_cnt=`ls $mysql_data/$relay_log_name* | grep -v "index" | wc -l`
		if [[ $relay_cnt -le 1 ]] && [[ $binlog_cnt -le 1 ]];then
			return 0
		fi

		echo -e "binlog_cnt:$binlog_cnt, relay_cnt:$relay_cnt"
		bin_log_capacity=0
		relay_log_capacity=0
		if [[ $binlog_cnt -ne 0 ]];then
			bin_log_capacity=`ls $mysql_data/$bin_log_name* | xargs du -cb | grep -w "total" | awk '{s+=$1}END{print s}'`
		fi
		if [[ $relay_cnt -ne 0 ]];then
			relay_log_capacity=`ls $mysql_data/$relay_log_name* | xargs du -cb | grep -w "total" | awk '{s+=$1}END{print s}'`
		fi
		#echo -e "calculate total:$total_bin_log_max bin:$bin_log_capacity relay:$relay_log_capacity"
		#容量越界或个数超过上限
		let total_use=$bin_log_capacity+$relay_log_capacity
		if [[ $total_use -gt $total_bin_log_max ]] || [[ $binlog_cnt -gt $total_bin_log_cnt_threshold ]] || [[ $relay_cnt -gt $total_bin_log_cnt_threshold ]]; then
			checkMode
			if [[ $? -eq 1 ]];then
				masterPurge $binlog_cnt $relay_cnt
			else
				slavePurge $binlog_cnt $relay_cnt $bin_log_capacity $relay_log_capacity
			fi
		else
			return 0
		fi
	sleep 1
	done
}

self_pid=$$
#杀死上一次的purgeBinlog.sh
check_pid=`ps -ef | grep PurgeBinlog.sh | grep -v grep | grep -v $self_pid | awk '{print $2}' | wc -l`
if [ $check_pid -gt 0 ];then
    ps -ef | grep PurgeBinlog.sh | grep -v grep | grep -v $self_pid | awk '{print $2}' | xargs kill
    logInfo "kill purgeBinlog.sh when starting!"
fi

#建议20分钟执行一次
checkLog

exit 0