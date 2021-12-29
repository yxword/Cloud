#!/bin/bash
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)
mysql_config_path=/etc/my.cnf

TAG=prepare
source $parent_dir/Common/CommonFunc.sh

if [[ $# -lt 1 ]];then
    echo -e "USAGE:$0 server-id"
    logInfo "set server_id failed,$*."
    exit 1;
fi

#改变shell目录下面所有的文件的可执行权限
chmod +x $parent_dir/*

function setServerID()
{
    server_id=$1
    #mysql登录的用户名和密码
    getUserCfg
    
    #修改配置文件里面的server-id
    #先判断server-id是否与需要修改的值相同，如果相同则不需要修改
    old_server_id=$(cat /etc/my.cnf|grep "^server-id"| awk -F "=" '{print $2}' | sed 's/[ ]//g')
    if [ $old_server_id -ne $server_id ];then
        sed -i -c "s/server-id.*/server-id = $server_id/g" $mysql_config_path
    fi
    #server-id即时生效
    mysql -u$mysql_user -p$mysql_password -e "set global server_id=$server_id;"
    
    if [ $? -ne 0 ];then
        logInfo "set server_id failed, server-id=$server_id"
        return 1
    fi
    
    logInfo "set server_id success, server-id=$server_id"
    return 0
}

setServerID $1
exit $?
