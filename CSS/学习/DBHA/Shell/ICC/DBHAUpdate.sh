#!/bin/bash

# 升级服务目录
UPGRADE_PATH=/cloud/dahua/Upgrade/Service/DB-Manager

BITS=64
SYS=`uname -m`
if [ $SYS == "x86_64" ] || [ $SYS == "aarch64" ];then
    BITS=64
else
    BITS=
fi

#################固定参数################

# 服务总目录
SERVER_PATH=/cloud/dahua/DataBaseServer

# 日志文件
LOG_FILE=/cloud/dahua/DataBaseServer/DBHA/Log/upgrade.log

#################需配置参数################

# 服务名称
SERVER_NAME=DB-Manager

# 服务独立文件夹名
SERVER_FOLDER=DBHA

# 服务BIN
SERVER_BIN=DBHAServer${BITS}

# 网卡名
VIF_NAME=`cat /cloud/dahua/DataBaseServer/DBHA/Config/DBHA.conf |grep DBHA_if_name|awk -F'\"' '{print $4}'`

#########################################

function startNode()
{
	#清除日志
	[[ $flag_log = "appendLog" ]] || rm -f $LOG_FILE
	
	# 取IP地址
	result=`ip addr show dev ${VIF_NAME}`
	if [ $? -ne 0 ];then
		deploy=$(/etc/init.d/DB-Manager detail | grep install | awk -F ":" '{print $2}' | awk -F ",|/*" '{print $1}' | sed 's/[ ]//g')
		if [ $deploy -eq 0 ];then
			IP=`ip addr show | grep 'inet' | grep 'brd' | grep "scope global" | head -n 1 | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
		else
			log_detail "get ip failed,ret=$result"
			return 1
		fi
	else
        IP=`ip addr show dev ${VIF_NAME} | grep 'inet ' | grep 'brd' | grep "${VIF_NAME}$" | awk -F "/" '{print $1}' | awk -F "inet" '{print $2}' | tr -d " "`
	fi
	
    log_detail "start startUpgrade.sh:startNode"
    log_upgrading "127.0.0.1"
	###########升级前校验##########
	log_percent "0" 
	log_detail "preparing..."
	log_status "preparing"
	
	# 取软连接实际地址
	temp=`ls -l ${SERVER_PATH} | grep ${SERVER_FOLDER} | awk '{print $NF}'`
	if [ "$temp" == "" ];then
		log_detail "soft link get error,cmd=${temp}"
		return 1
	fi
	
	# 软链接是否真实存在
	result=`cd ${SERVER_PATH};cd ${temp};echo $?`
	if [ $result -ne 0 ];then
		log_detail "soft link does not exist, soft link=$temp"
		return 1
	fi
	soft_link=`cd ${SERVER_PATH};cd ${temp};pwd`
	
	# 检查预规划目录
	real_path_0="${SERVER_PATH}/Programs/${SERVER_FOLDER}0"
	real_path_1="${SERVER_PATH}/Programs/${SERVER_FOLDER}1"
	if [ ! -d ${real_path_0} ];then
		log_detail "${real_path_0} is not exist!"
		return 1
	fi
	if [ ! -d ${real_path_1} ];then
		log_detail "${real_path_1} is not exist!"
		return 1
	fi
	
	# 检查软链接是否在预规划目录中
	new_soft_link=""
	if [ "${soft_link}" == "${real_path_0}" ];then
		new_soft_link=${real_path_1}
	elif [ "${soft_link}" == "${real_path_1}" ];then
		new_soft_link=${real_path_0}
	else
		log_detail "soft_link is exception, please check ${soft_link}"
		return 1
	fi
	
	if [ ! -e ${new_soft_link}/needUpgrade ];then
		log_detail "No need change soft link!"
		return 0
	fi
	rm -rf ${new_soft_link}/needUpgrade
	#rm -rf ${soft_link}/Log #此处移除该软链，会导致下面log打印不出来
	
	###########升级##########
	log_detail "stopping..."
	log_status "stopping"

	
	# 改为停止服务
 	/etc/init.d/${SERVER_NAME} stop > /dev/null
	/etc/init.d/${SERVER_NAME} stop > /dev/null
 	if [ $? -ne 0 ];then
 	 	log_detail "stop failed"
 	 	return 1
 	 fi
 	# mv日志
 	#mv ${soft_link}/Log/* ${new_soft_link}/Log/
    
	rm -rf ${soft_link}/Log
	# 修改软链接
	cd ${SERVER_PATH}
	rm -f ${SERVER_PATH}/${SERVER_FOLDER}
	ln -s ${new_soft_link} ${SERVER_PATH}/${SERVER_FOLDER}
	
	#备份my.cnf
	local config_backup_path=$SERVER_PATH/config_backup
	date_info=$(date "+%Y_%m_%d_%H%M%S")
	[ ! -d $config_backup_path ] && mkdir -p $config_backup_path
	cp -r ${new_soft_link}/Config/my.cnf $config_backup_path/my_${date_info}_prepareupgrade.cnf	
	
	###########重启##########
	log_detail "starting..."
	log_status "starting"
	
	# 重启服务
	/etc/init.d/${SERVER_NAME} start > /dev/null
	if [ $? -ne 0 ];then
		log_detail "start failed"
		return 1
	fi
	
	log_detail "restart success"
	log_detail "upgrade success"

	return 0
}

# 升级
if [ $1 -eq 6 ]; then
    # 保存该升级脚本
    mkdir -p ${UPGRADE_PATH}
    cp -rf ./DBHAUpdate.sh ${UPGRADE_PATH}
    
    # 解压升级包
    tar -zxf DBHACluster_Upgrade_O.tar.gz > /dev/null 2>&1

    mkdir -p ./DBHACluster_MasterNode_upgrade/
    tar -zxf DBHACluster_MasterNode.tar.gz -C ./DBHACluster_MasterNode_upgrade/ > /dev/null 2>&1
    chmod +x ./DBHACluster_MasterNode_upgrade/shell/*
    cd ./DBHACluster_MasterNode_upgrade/shell/
    sh prepareUpgrade.sh appendLog noInfo
    #. ./startUpgrade.sh
    . ./upgraderFunc.sh
    startNode
    cd ..

fi

