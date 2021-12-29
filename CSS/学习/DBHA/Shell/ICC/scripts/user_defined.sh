#!/bin/bash

#-------------------------------------------------------
# Filename:		user_defined.sh
# Revision:		1.0
# Date:			2021.11.05
# Author:		CloudStorage
#-------------------------------------------------------


# 当前脚本路径
cur_path=$(cd "$(dirname "$0")"; pwd)

# 工程根目录
project_root_path=`cd $cur_path; cd ..; pwd`

# 模块上报运维的配置json模板
properties_template_json=$project_root_path/scripts/module-dependence-config.json

# 模块名称
module_name=DBHA

# 服务bin目录
service_bin_dir=$project_root_path/bin

# 安装部署的配置文件路径
install_config_path=$cur_path/DB-Manager_install.conf

# 创建DB用户的配置文件路径
createUser_config_path=$cur_path/DB-Manager_createUser.conf

# VCS-DBHA.sh脚本路径
vcs_dbha_manager=/cloud/dahua/DataBaseServer/DBHA/Shell/VCS-DBHA.sh

# 用户自定义配置属性替换逻辑
# args-1：配置项key
# args-2：配置项value
replace_local_config_by_user(){
    key=$1
    value=$2
	
    echo "${key}=${value}" | sed "s/;/,/g" | sed "s/|/,/g" >> ${install_config_path}
}

# 根据下发的配置文件,进行DBHA的部署并且拉起服务
dbha_install(){
    . ${install_config_path}
    sh ${vcs_dbha_manager} install ${install_config_path}
    return $?
}

test_link(){
    sh ${vcs_dbha_manager} testLink
    return $?
}

# 创建DB用户
create_user(){
    rm -f ${createUser_config_path}
    echo "userName=$1" >> ${createUser_config_path}
    echo "userPwd=$2" >> ${createUser_config_path}
    sh ${vcs_dbha_manager} createUser ${createUser_config_path}
    return $?
}

# 停止服务
stop_service(){
    sh ${vcs_dbha_manager} stop
}

# 启动服务
start_service(){
    sh ${vcs_dbha_manager} start
}


