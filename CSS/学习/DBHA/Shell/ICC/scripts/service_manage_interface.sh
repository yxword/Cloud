#!/bin/bash

#-------------------------------------------------------
# Filename:		service_manage_interface.sh
# Revision:		1.0
# Date:			2021.11.05
# Author:		CloudStorage
#-------------------------------------------------------


# 当前脚本路径
cur_path=$(cd "$(dirname "$0")"; pwd)


# import processor scripts
source $cur_path/config_processor.sh

# 依据上报配置的json模板，更新已配置的属性值，供运维runs使用
config_read(){
	log_out i "Runs pull config-info start.."
	if [ $# -eq 1 ]; then
		result=$(read_config_for_runs $1)
	else
		result=$(read_config_for_runs)
	fi
	
	log_out i "Runs pull config-info end, result=$?"
	if [ $? -eq 0 ]; then
		log_out i "Runs pull config-info: $result"
		echo $result
		return 0
	else
		return $?
	fi
}

# 由运维调用，根据参数中的下发配置信息，更新本地配置文件
# args-1: 
#   <source-config-params> 运维下发的配置JSON
config_update(){
    rm -f ${install_config_path}
	if [ $# -lt 1 ]; then
		log_out e "invoke shell[$0] with invalid args .."
		echo "Usage : $0 [config-params]"
		exit 1
	fi
	
	log_out i "Runs update config-info start.."
	log_out i "invoke with args=$1"
	
	update_config_from_runs "$1"
    
    if [ ! -f ${install_config_path} ]; then
        log_out e "config_update error, ${install_config_path} not generate"
        exit 1
    fi
	
	log_out i "Runs update config-info end, result=$?"
	return $?
}


# 停止服务
service_stop(){
	# 执行用户自定义操作
	if [ "$(type -t stop_service)" = "function" ] ; then
		stop_service
	fi
}

# 启动服务
service_start(){
	# 执行用户自定义操作
	if [ "$(type -t start_service)" = "function" ] ; then
		start_service
	fi
}

# 重启服务
service_restart(){
	service_stop
	sleep 10
	service_start
}


#==================================================================
# main
# args-1：
#   1-更新配置
#   2-拉取配置
#   3-停止服务
#   4-启动服务
#   5-查询服务状态
#   6-创建DB用户
#   98-初始化（将模板json文件中的值赋值到对应的配置项上）
#   99-重启服务
#==================================================================

updateConfig=1
readConfig=2
stop=3
start=4
status=5
createUser=6
init=98
restart=99

if [ $# -eq 0 ]; then
	log_out e "invoke shell[$0] with invalid args"
	echo "Usage : $0 [method-code]"
	exit 1
fi

method=$1
case $method in
    $updateConfig)
	if [ $# -gt 1 ]; then
	    log_out i "begin install"
		config_update "$2"
        dbha_install
        [ $? -eq 1 ] && log_out e "DBHA install error"
		log_out i "end install"
	else
		log_out e "invoke method[config_update] need 2 args"
	fi
    ;;
    $readConfig)
	if [ $# -gt 1 ]; then
		config_read $2
	else
		config_read
	fi
    ;;
    $stop)
	service_stop
    ;;
    $start)
	service_start
    ;;
    $status)
	sh ${vcs_dbha_manager} status
    if [ $? -ne 0 ]; then
        log_out w "DBHAKeeper.sh or DB-Manager is stopped"
    fi
    ;;
    $createUser)
    log_out i "begin create user"
    test_link
    if [ $? -ne 0 ];then
        log_out e "test Link Database error."
        exit 1
    fi
    create_user $2 $3
    if [ $? -ne 0 ]; then
        log_out e "create Database user error."
    fi
    log_out i "end create user"
    ;;
    $init)
    # 初始化，将json模板中初始值赋值到对应的配置文件中
	result=`python $cur_path/generate_config_json.py $properties_template_json`
	if [[ ! -z "$result" ]]; then
	    config_update "$result"
	else
	    log_out e "invoke method[config_init] need 1 args"
	fi
    ;;
    $restart)
	service_restart
    ;;
esac

exit 0

 
