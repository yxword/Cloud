#!/bin/bash
#提供三个状态值slave_connect_master、slave_running_state、slave_sql_thread
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh

CONFIG_FILE=$DBHA_CONFIG/DBHA.conf
TAG=checkMasterState

#mysql登录的用户名和密码
getUserCfg

#同步信息的用户名和密码
getSyncUserCfg

#检测master上同步用户是否存在:1,不存在；0，存在
function checkSyncUser()
{
    master_ip=$1
    if [ -n "${master_ip}" ];then
        sync_host=$(mysql -u$mysql_user -p$mysql_password -h$master_ip --connect_timeout=2 -e "select Host from mysql.user where User='$sync_user'\G" | grep "Host")
    else
        business_ip=$(cat $CONFIG_FILE | grep peer_business_IP  | awk -F": |:|," '{print $2}' | awk -F"\"" '{print $2}')
        heartbeat_ip=$(cat $CONFIG_FILE | grep peer_heartbeat_IP  | awk -F": |:|," '{print $2}' | awk -F"\"" '{print $2}')
        hosts=$(mysql -u$mysql_user -p$mysql_password -e "select Host from mysql.user where User='$sync_user'\G" | grep "Host")
        sync_host=$hosts
        #if [ ! -z "$business_ip" ];then
        #    exists=`echo $hosts | grep -c "$business_ip"`
        #    if [ $exists -eq 0 ];then
        #        sync_host=""
        #    fi
        #fi
        
        #if [ ! -z "$heartbeat_ip" ];then
        #    exists=`echo $hosts | grep -c "$heartbeat_ip"`
        #    if [ $exists -eq 0 ];then
        #        sync_host=""
        #    fi
        #fi
    fi
    
    #仅判断slave账号是否存在，host为%
    if [ -z "$sync_host" ];then
        logInfo "check master running status failed:$sync_user not exist!"
        return 1
    fi
    
    return 0
}

if [ $# -eq 1 ];then
	checkSyncUser $1
else 
	checkSyncUser
fi
exit $?