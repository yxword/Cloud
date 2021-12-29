#!/bin/bash

export PATH=${PATH}:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin

shell_dir=$(cd "$(dirname "$0")"; pwd)

log_file=${shell_dir}/dbha_mapping_monitor.log

#------------------------------------------------

# 参数：vnci、vip
function has_vip()
{
	netcard=$1
	vip=$2
	
	match_vip=$(ifconfig $netcard | grep $vip)
	#echo "$match_vip"  >> ${log_file}
	if [ -z "$match_vip" ]; then
		return 0
	fi
	
	return 1
}

# 参数：vnci、vip
function down_vip()
{
	vnetcard=$1

	echo "ifconfig $vnetcard down"
	ifconfig $vnetcard down
}

# 参数：vnci、vip, netmask
function up_vip()
{
	vnetcard=$1
	vip=$2
	netmask=$3

	echo "ifconfig $vnetcard $vip netmask $netmask up"
	ifconfig $vnetcard $vip netmask $netmask up
}

# 参数：real nci、vip
function arping_vip()
{
	real_netcard=$1
	vip=$2

	echo "arping -b -A -U -c 1 -I $real_netcard $vip"
	arping -b -A -U -c 1 -I $real_netcard $vip
}

# 参数：store vnci、store vip、business vnci、business vip, business netmask
function check_and_do_vip()
{
	real_netcard=$1
	real_vip=$2
	mapping_netcard=$3
	mapping_vip=$4
	mapping_netmask=$5

	has_vip $real_netcard $real_vip
	has_real_vip=$?
	
	has_vip $mapping_netcard $mapping_vip
	has_mapping_vip=$?
	
	if [ $has_real_vip -ne 0 ]; then
		if [ $has_mapping_vip -eq 0 ]; then
			echo "up_vip $mapping_netcard $mapping_vip $mapping_netmask"
			echo "up_vip $mapping_netcard $mapping_vip $mapping_netmask" >> ${log_file}
			up_vip $mapping_netcard $mapping_vip $mapping_netmask
		fi
	else
		if [ $has_mapping_vip -ne 0 ]; then
			echo "down_vip $mapping_netcard $mapping_vip"
			echo "down_vip $mapping_netcard $mapping_vip" >> ${log_file}
			down_vip $mapping_netcard $mapping_vip
		fi
	fi
}

# 参数：business real nci、business vnci、business vip
function check_and_do_arp()
{
	real_netcard=$1
	mapping_netcard=$2
	mapping_vip=$3
	
	has_vip $mapping_netcard $mapping_vip
	has_mapping_vip=$?
	
	if [ $has_mapping_vip -ne 0 ]; then
		arping_vip $real_netcard $mapping_vip
	fi
}

#------------------------------------------------

#main

echo "--start: $(date) ---"  >> ${log_file}

#load vip config
. ${shell_dir}/dbha_mapping_vip_config.sh

echo dbha: $dbha_vnci $dbha_vip $mapping_dbha_vnci $mapping_dbha_vip $mapping_dbha_netmask >> ${log_file}

check_and_do_vip $dbha_vnci $dbha_vip $mapping_dbha_vnci $mapping_dbha_vip $mapping_dbha_netmask

check_and_do_arp $mapping_dbha_real_nci $mapping_dbha_vnci $mapping_dbha_vip


#------------------------------------------------

# sample:
# ifconfig bond0:0 down
# ifconfig bond0:0 172.25.115.20 netmask 255.255.0.0 up
# arping -b -A -U -c 1  -I bond0 172.25.115.20
#



