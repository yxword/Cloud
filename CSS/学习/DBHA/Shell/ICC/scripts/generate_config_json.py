#!/usr/bin/python
# -*- coding: UTF-8 -*-
# filename: generate_config_json.py

import os
import sys
import io
import json


def is_number(s):
	try:
		float(s)
		return True
	except ValueError:
		pass
		
	try:
		import unicodedata
		unicodedata.numeric(s)
		return True
	except (TypeError, ValueError):
		pass
		
	return False

def import_json_data_from_file(path):
	"""
	从文件中获取json模板
	:param path: json模板路径
	:return json_data: json模板内容对应的字典
	"""
	try:
		with open(path, 'r+') as f:
			try:
				json_data = json.load(f)
			except Exception as e:
				print('invalid json-data: ' + str(e))
		return json_data
	except Exception as e:
		print('file does not exist: ' + str(e))

# 
# json_dict
# property_dict
def replace_configured_properties(json_dict, property_dict):
	"""
	根据传入的json，替换相应节点的value为当前配置值，并返回结果json
	:param json_dict: 原模板json对应的字典对象
	:param property_dict: 本地现有配置键值对字典对象
	:return 替换处理后的json字符串
	"""
	if not isinstance(json_dict, dict):
		print('error data-type: ' + str(json_dict))
		return 1
	
	if len(json_dict.keys()) == 0 :
		print('empty json-data: ' + str(json_dict))
		return 1
	
	if not json_dict.has_key("dependenceConfigs"):
		print('no valid key in json-data: dependenceConfigs, json-data=' + str(json_dict))
		return 1
	
	dependenceConfigs = json_dict.get('dependenceConfigs')
	if not isinstance(dependenceConfigs, list):
		print('no valid config-infos in json-data: ' + str(json_dict))
		return 1
		
	for item in dependenceConfigs:
		if not isinstance(item, dict):
			print('invalid config-item: ' + str(item))
			continue
			
		for config in item.get("configs"):
			if not isinstance(config, dict):
				print('invalid config: ' + str(config))
				continue
			
			config_key=config.get("key")
			if property_dict.has_key(config_key):
				config["value"] = property_dict.get(config_key)
	
	filename = os.getcwd() + "/replace_result_tmp.json"
	with open(filename, "w") as f:
		f.write(str(json.dumps(json_dict, indent=4)))
		f.close()
	
	return json.dumps(json_dict)

def import_local_properties_value(path):
	"""
	从文件中获取本地现有配置key对应的值
	:param path: 现有配置值键值对文件路径
	:return configured_properties: 读取出的文件内容对应的字典对象
	"""
	# 用于存储已配置的属性值
	configured_properties = {}
	try:
		with open(path, 'r+') as f:
			for line in f:
				# 去掉行尾换行符
				line=line.strip('\n')
				
				if not line.strip():
					continue
				
				results = line.split("=", 1)
				if len(results) == 2:
					key = results[0]
					value = results[1]
					configured_properties[key] = value
				else:
					print("invalid format of [key=value]: " + line)
					continue
	except Exception as e:
		print('file does not exist: ' + str(e))
	
	return configured_properties

# json_dict
def import_json_data_from_file_to_properties(json_dict):
	"""
	从文件中获取本地现有配置key对应的值
	:param json_dict: 现有配置值键值对文件路径
	:return configured_properties: 读取出的文件内容对应的字典对象
	"""
	# 用于存储已配置的属性值
	configured_properties = {}

	if not isinstance(json_dict, dict):
		print('error data-type: ' + str(json_dict))
		return 1

	if len(json_dict.keys()) == 0 :
		print('empty json-data: ' + str(json_dict))
		return 1

	if not json_dict.has_key("dependenceConfigs"):
		print('no valid key in json-data: dependenceConfigs, json-data=' + str(json_dict))
		return 1

	dependenceConfigs = json_dict.get('dependenceConfigs')
	if not isinstance(dependenceConfigs, list):
		print('no valid config-infos in json-data: ' + str(json_dict))
		return 1

	for item in dependenceConfigs:
		if not isinstance(item, dict):
			print('invalid config-item: ' + str(item))
			continue

		for config in item.get("configs"):
			if not isinstance(config, dict):
				print('invalid config: ' + str(config))
				continue

			config_key=config.get("key")
			config_value=config.get("value")
			configured_properties[config_key]=config_value

	return json.dumps(configured_properties)


if __name__ == "__main__" :
	# 实际需要传递给该python的参数为两个，长度为3是因为第一个参数是当前python脚本自身
	if len(sys.argv) >= 3:
		result_json_data = import_json_data_from_file(sys.argv[1])
		result_configured_properties = import_local_properties_value(sys.argv[2])
		result = replace_configured_properties(result_json_data, result_configured_properties)
		print(result)
		sys.exit(0)

	# 输入一个参数时，表示将json文件内容替换配置文件中内容（即初始化）
	elif len(sys.argv) == 2:
		result_json_data = import_json_data_from_file(sys.argv[1])
		result = import_json_data_from_file_to_properties(result_json_data)
		print(result)
		sys.exit(0)

	else:
		print('error number of args: ' + ', '.join(sys.argv))
		sys.exit(1)


