## redmine

http://10.31.17.179/redmine/issues/69908

## 步骤

### 验证DH_CSS-SYS_CSS73-VCS-Micro型号

* 73Micro装机之后 ifconfig 没问题

### 验证曙光高性能子节点

- 烧机，修改ip配置后，重启网络未发现 ifconfig 查询到的ip与网卡配置文件中的ip不一致
- 编写脚本，循环进行更改ip并重启网络，将ifconfig和网卡配置的ip输出到 ip.log 日志中，查看ip是否一致

```shell
#!/bin/bash
log_file=/root/ip.log

while true
    do
    # 更改ip并重启网络
    cp -f /etc/sysconfig/network-scripts/ifcfg-bond0 /etc/sysconfig/network-scripts/ifcfg-bond0.tmp
    cp -f /etc/sysconfig/network-scripts/ifcfg-bond0.bak /etc/sysconfig/network-scripts/ifcfg-bond0
    cp -f /etc/sysconfig/network-scripts/ifcfg-bond0.tmp /etc/sysconfig/network-scripts/ifcfg-bond0.bak

    cp -f /etc/sysconfig/network-scripts/ifcfg-bond1 /etc/sysconfig/network-scripts/ifcfg-bond1.tmp
    cp -f /etc/sysconfig/network-scripts/ifcfg-bond1.bak /etc/sysconfig/network-scripts/ifcfg-bond1
    cp -f /etc/sysconfig/network-scripts/ifcfg-bond1.tmp /etc/sysconfig/network-scripts/ifcfg-bond1.bak

    systemctl restart network

    # 查看ifconfig和网卡配置
    date +"--------------------[%F %T]--------------------" >> ${log_file}
    i=0
    ifconfig | head -20 | grep broadcast | while read ifconfig
    do
        echo -n "[ipconfig-${i}]  " >> ${log_file}
        echo $ifconfig | sed -n 's/^ *//p' | awk -F ' ' '{print $2}' >> ${log_file}
        let i+=1
    done

    bond0=`cat /etc/sysconfig/network-scripts/ifcfg-bond0 | grep IPADDR | awk -F '=' '{print $2}'`
    echo "[ifcfg-bond0] ${bond0}" >> ${log_file}
    bond1=`cat /etc/sysconfig/network-scripts/ifcfg-bond1 | grep IPADDR | awk -F '=' '{print $2}'`
    echo "[ifcfg-bond1] ${bond1}" >> ${log_file}

    sleep 60
done &
```

- 脚本执行，查看日志，未发现ifconfig查询到的ip与网卡配置文件中的ip不一致，执行100次循环结果见附件 ip.log

```txt
......
--------------------[2021-08-25 14:19:13]--------------------
[ipconfig-0]  10.35.28.131
[ipconfig-1]  192.168.28.131
[ifcfg-bond0] 10.35.28.131
[ifcfg-bond1] 192.168.28.131
--------------------[2021-08-25 14:20:36]--------------------
[ipconfig-0]  10.35.28.132
[ipconfig-1]  192.168.28.132
[ifcfg-bond0] 10.35.28.132
[ifcfg-bond1] 192.168.28.132
......
```



## 问题与总结

grub-install安装grub时出错：/boot/grub/stage1 not read correctly.

上述问题在112.125服务器制作时会出现，在112.119服务器不会出现.

解决：将U盘第一个分区格式化为 ext3 可以成功安装grub。

