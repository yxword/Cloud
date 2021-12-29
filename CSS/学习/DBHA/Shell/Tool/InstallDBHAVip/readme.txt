
概述
智能云环境上DBHA只内部使用，如bond3:11;当外部定制业务也想要使用数据库时就难以访问

使用方法：

1、配置
编辑dbha_mapping_vip_config.sh，配置存储网段和业务网段的虚IP和网卡信息。
视频云环境下，因为元数据服务器默认有业务网卡bond2，所以可以直接在bond2上开启云存储的业务虚IP，不需要做额外的网络配置，性能也不会减半。
例子：
dbha_vip="191.168.0.4"
dbha_vnci="bond3:11"

mapping_dbha_vip="172.21.153.22"
mapping_dbha_netmask="255.255.255.0"
mapping_dbha_real_nci="bond2"
mapping_dbha_vnci="bond2:11"

2、安装
命令：sh install_dbha_vip_monitor.sh
说明：安装脚本会把检测脚本monitor_dbha_mapping_vip.sh添加到crontab中。
每一分钟检测一次虚IP，以开启云存储的业务段虚IP，并保持和对应的存储段虚IP在同一台机器上，并根据虚IP的情况发送arping包来声明虚IP所在机器。

3、卸载
命令：sh uninstall_dbha_vip_monitor.sh
说明：卸载脚本会把检测脚本monitor_dbha_mapping_vip.sh从crontab中删除，同时停止业务段的虚IP。

4、效果
