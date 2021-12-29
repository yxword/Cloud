#!/bin/bash

#-------------------------------------------------------
# Filename:		config_processor.sh
# Revision:		1.0
# Date:			2021.11.05
# Author:		CloudStorage
#-------------------------------------------------------


# 当前脚本路径
cur_path=$(cd "$(dirname "$0")"; pwd)

# 工程根目录
project_root_path=`cd $cur_path; cd ..; pwd`

# 本地配置文件更新策略的配置
properties_update_conf=$cur_path/properties_update.conf

# 本地配置项和运维端配置项的对应关系
properties_mapping_conf=$cur_path/properties_mapping.conf

# 运维下发配置信息的预处理临时文件
source_conf_tmp=$cur_path/source_tmp.conf

# 配置文件路径
config_base_dir=`cat $properties_update_conf | grep ^config-base-dir | awk -F '=' '{print $2}'`

# 需要被扫描的配置文件列表
mapping_file_list_ls=`cat $properties_update_conf | grep ^scan-files-mapping | awk -F '=' '{print $2}'`


# import user-defined scripts
source $cur_path/user_defined.sh


# 输出日志信息
# args-1：日志级别（ERROR, WARN, INFO）
# args-2：输出内容
function log_out(){
	cur_file=$0
	level_code=$1
	level="INFO"
	if [ "$level_code" == "d" ]; then
		level="DEBUG"
	elif [ "$level_code" == "i" ]; then
		level="INFO"
	elif [ "$level_code" == "w" ]; then
		level="WARN"
	elif [ "$level_code" == "e" ]; then
		level="ERROR"
	fi
	echo "$(date +'%Y-%m-%d %H:%M:%S'):[$cur_file][$level] $2" >> $cur_path/service_manage.log
}

# 对运维下发下来的配置JSON进行预处理，生成"key=value"格式的键值对内容
# args-1：
#   <source-config-params> 运维下发的配置JSON
function pre_processing(){
	content="$1"
	log_out i "start to pre-processing properties from runs: $content"

	# 去掉首尾大括号 { }
	content=${content%*\}}
	content=${content#*\{}
	# 写入临时文件
	echo $content > $source_conf_tmp
	# 将每个配置项做换行处理
	sed -i 's/,/\n/g' $source_conf_tmp
	# 去除行首尾空格
	#sed -i 's/^[ ]*//g' $source_conf_tmp
	#sed -i 's/[ ]*$//g' $source_conf_tmp

	tmp_file=${source_conf_tmp}"_2"

	# 替换配置文件中的含有占位符的配置项
	while read line; do
		if [ -z "$line" ] || [[ ! $line =~ ":" ]]; then
			continue
		fi

		# 删除第一个"="以及其右边的字符串，并删除首尾引号和空格，即为key
		key=${line%%:*}; key=${key#*\"}; key=${key%*\"}; key=`echo $key | awk '$1=$1'`

		# 删除第一个"="以及其左边的字符串，并删除首尾引号和空格，即为value
		value=${line#*:}; value=${value#*\"}; value=${value%*\"}; value=`echo $value | sed -e 's/^[ ]*//g' | sed -e 's/[ ]*$//g'`

		if [ -z "$key" ]; then
			log_out w "invalud key, skip this item"
			continue
		fi

		new_line="$key=$value"

		echo $new_line >> $tmp_file

	done < $source_conf_tmp

	rm -rf $source_conf_tmp; mv $tmp_file $source_conf_tmp

	log_out i "pre-processing properties completed"

	return 0
}

# 处理key-value映射格式的配置文件
# args-1: source_properties_conf 预处理好的键值对属性文件
function process_mapping_files(){
	# 判断文件是否存在
	if [ -f $properties_mapping_conf ]; then
		dos2unix $properties_mapping_conf > /dev/null 2>&1
	else
		log_out e "Mapping config file[$properties_mapping_conf] not exist, skip .."
		return 0
	fi

	# 预处理好的键值对属性文件
	source_properties_conf=$1

	if [ -z $config_base_dir ]; then
		mapping_file_list_ls=`echo $mapping_file_list_ls | sed 's/,/ /g'`
	else
		mapping_file_list_ls=`echo $mapping_file_list_ls | sed "s:,: $config_base_dir:g"`
		mapping_file_list_ls=$config_base_dir$mapping_file_list_ls
	fi
	log_out i "start to process mapping-files: $mapping_file_list_ls"

	# 替换配置文件中的相应配置项
	while read line; do
		if [ -z "$line" ] || [[ ! $line =~ "=" ]]; then
			continue
		fi

		key=${line%%=*}
		value=${line#*=}
		if [ -z $value ] || [ -z $key ]; then
			log_out e "key or value is empty[ $line ], skip and continue .."
			continue
		fi

		# echo "key: $key / value: $value"

		# 根据key去mapping配置中查找哪些配置项需要被替换
		if [ ! -z "$mapping_file_list_ls" ]; then
			for key_line in `cat $properties_mapping_conf | grep ".\+\(=$key\)$"`; do
				# 服务自有配置key
				local_key=`echo $key_line | sed "s/=$key//"`

			done
		else
			log_out w "no files need to be processed, this step will be skiped .."
		fi

		# 执行用户自定义操作
		if [ "$(type -t replace_local_config_by_user)" = "function" ] ; then
			replace_local_config_by_user $local_key $value
		fi
	done < $source_properties_conf

	log_out i "process mapping-files completed"
}


# 更新配置文件
# args-1:
#   <source-config-params> 运维下发的配置JSON
function update_config_from_runs(){
	if [ $# -lt 1 ]; then
		log_out e "invoke shell[$0] with invalid args .."
		echo "Usage : $0 [config-params]"
		exit 1
	fi

	# 预处理运维下发的配置，并写入临时文件 $source_conf_tmp
	pre_processing "$1"
	if [ $? -ne 0 ]; then
		log_out e "pre-process conf-file failed: $1"
		return 1
	fi

	# 处理key-value映射格式的配置文件
	process_mapping_files $source_conf_tmp

	log_out i "Done: all config-files are processed"

	return 0
}


# 读取模块待配置属性信息，供运维runs使用，目前暂不使用
# 当有一个参数传入，并且值为1的时候，会返回原模板数据，没有参数或者非1，则返回当前最新的真实配置值
function read_config_for_runs(){
	dos2unix $properties_template_json > /dev/null 2>&1
	if [ $# -eq 1 ] && [ $1 -eq 1 ]; then
		log_out i "invoke without args, or args[\$1] not equals 1, use original template."
		echo `cat $properties_template_json`
		return 0
	fi

    #更新source_tmp.conf文件
	read_current_config_for_runs

	if [ -f $source_conf_tmp ]; then
		log_out i "invoke with args[1], use replaced template."
		log_out i "invoke: python generate_config_json.py $properties_template_json $source_conf_tmp"
		result=`python $cur_path/generate_config_json.py $properties_template_json $source_conf_tmp`
		if [ $? -eq 0 ]; then
			echo $result
		else
			log_out w "update local-value to template failed, use original template instead."
			echo `cat $properties_template_json`
		fi
	else
		log_out w "first time to config, use original template directly."
		echo `cat $properties_template_json`
	fi

	return 0
}

#读取当前本地配置文件并更新source_tmp.conf文件，目前暂不使用，保留函数入口
function read_current_config_for_runs(){

}

