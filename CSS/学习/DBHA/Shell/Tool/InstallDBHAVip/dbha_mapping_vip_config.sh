#!/bin/bash

#------------------------------------------------
##将bond3:11 191.168.0.4映射到bond2的bond2:11上；IP和掩码填写
## DBHA的实际vip
dbha_vip="191.168.0.4"
dbha_vnci="bond3:11"

mapping_dbha_vip="172.21.153.22"
mapping_dbha_netmask="255.255.255.0"
mapping_dbha_real_nci="bond2"
mapping_dbha_vnci="bond2:11"
#------------------------------------------------


