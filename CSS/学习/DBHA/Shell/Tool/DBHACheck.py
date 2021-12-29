#!/bin/env python

import os,sys
import time
import logging
from signal import signal, SIGPIPE, SIG_DFL

signal(SIGPIPE,SIG_DFL) 

#path
log_path="/cloud/dahua/DataBaseServer/DBHA/Log"
bin_path="/cloud/dahua/DataBaseServer/DBHA/Bin"
shell_path="/cloud/dahua/DataBaseServer/DBHA/Shell/Service"
config_path="/cloud/dahua/DataBaseServer/DBHA/Config"

class DBHACheck():
    def __init__(self):
        # base info DBHA.conf
        self.vip = ''
        self.vip_netmask = ''

        self.master_ip=''
        self.master_mysql_user=''
        self.master_mysql_password=''
        self.slave_ip=''
        self.slave_mysql_user = ''
        self.slave_mysql_password = ''

        self.local_business_ip = ''
        self.local_heart_ip = ''
        self.peer_business_ip = ''
        self.peer_heart_ip = ''

        self.heartbeart_if_name = ''
        self.if_name = ''
        self.vif_name = ''

        self.local_install_flag = 0
        self.peer_install_flag=0
        self.backup_path = ''

        # base info HAConfig.conf
        self.local_cluster_id = ''
        self.peer_cluster_id = ''
        self.local_uid = 0
        self.peer_uid = 0

        # base info HAState.stat
        self.local_master_timestamp = ''
        self.peer_master_timestamp = ''
        self.local_master_state = 0
        self.peer_master_state = 0

        # base info Mysql.conf
        self.local_user = ''
        self.local_password = ''
        self.local_password_changed = 0

        self.peer_user = ''
        self.peer_password = ''
        self.peer_password_changed = 0

        self.bool_vip=False
        self.bool_mysqlstate=False

        self.ssh_user='root'
        #self.ssh_passwd=''
    
    def call_tip(self):
        if len(sys.argv) <2:
            print "useage:python DBHACheck.py [ssh_user]"
            print "if not input ssh_user, will use root."
        else:
            self.ssh_user=sys.argv[1]
            #self.ssh_passwd=sys.argv[2]

    def set_log(self):
        if self.ssh_user != "root":
            pass
        else:
            logger = logging.getLogger()
            logger.setLevel(logging.INFO)
            handler = logging.FileHandler("/cloud/dahua/DataBaseServer/DBHA/Log/DBHA_check.log",mode='w')
            handler.setLevel(logging.INFO)
            formatter = logging.Formatter('%(asctime)s: %(message)s')
            handler.setFormatter(formatter)
            logger.addHandler(handler)

    def read_baseinfo(self):
        # get vip
        vip_cmd = 'cat %s/DBHA.conf |grep "DBHA_virtual_IP\\""|awk -F "\\"" \'{print $4}\'' % config_path
        self.vip = os.popen(vip_cmd).readlines()[0].replace('\n', '')

        vip_netmask_cmd = 'cat %s/DBHA.conf |grep "DBHA_virtual_IP_netmask"|awk -F "\\"" \'{print $4}\'' % config_path
        self.vip_netmask = os.popen(vip_netmask_cmd).readlines()[0].replace('\n', '')

        # get local ip
        local_ip_cmd = 'cat %s/DBHA.conf |grep "local_business_IP"|awk -F "\\"" \'{print $4}\'' % config_path
        self.local_business_ip = os.popen(local_ip_cmd).readlines()[0].replace('\n', '')

        local_ip_cmd = 'cat %s/DBHA.conf |grep "local_heartbeat_IP"|awk -F "\\"" \'{print $4}\'' % config_path
        self.local_heart_ip = os.popen(local_ip_cmd).readlines()[0].replace('\n', '')

        # get peer ip
        peer_ip_cmd = 'cat %s/DBHA.conf |grep "peer_business_IP"|awk -F "\\"" \'{print $4}\'' % config_path
        self.peer_business_ip = os.popen(peer_ip_cmd).readlines()[0].replace('\n', '')

        peer_ip_cmd = 'cat %s/DBHA.conf |grep "peer_heartbeat_IP"|awk -F "\\"" \'{print $4}\'' % config_path
        self.peer_heart_ip = os.popen(peer_ip_cmd).readlines()[0].replace('\n', '')

        # get if name
        if_name_cmd = 'cat %s/DBHA.conf |grep "DBHA_if_name"|awk -F "\\"" \'{print $4}\'' % config_path
        self.if_name = os.popen(if_name_cmd).readlines()[0].replace('\n', '')

        heartbeart_if_name_cmd = 'cat %s/DBHA.conf |grep "DBHA_heartbeart_if_name"|awk -F "\\"" \'{print $4}\'' % config_path
        self.heartbeart_if_name = os.popen(heartbeart_if_name_cmd).readlines()[0].replace('\n', '')

        vif_name_cmd = 'cat %s/DBHA.conf |grep "DBHA_vif_name"|awk -F "\\"" \'{print $4}\'' % config_path
        self.vif_name = os.popen(vif_name_cmd).readlines()[0].replace('\n', '')

        backup_path_cmd = 'cat %s/DBHA.conf |grep "mysql_backup_path"|awk -F "\\"" \'{print $4}\'' % config_path
        self.backup_path = os.popen(backup_path_cmd).readlines()[0].replace('\n', '')

        #local
        local_install_flag_cmd = 'cat %s/DBHA.conf |grep "DBHA_install_flag"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % config_path
        self.local_install_flag = os.popen(local_install_flag_cmd).readlines()[0].replace('\n', '')

        # get cluster id and uid
        file_check=os.path.exists('%s/HAConfig.conf'% config_path)
        if file_check:
            local_uid_cmd = 'cat %s/HAConfig.conf |grep "uid"|awk -F "\\"" \'{print $4}\'' % config_path
            self.local_uid = os.popen(local_uid_cmd).readlines()[0].replace('\n', '')

            local_cluster_id_cmd = 'cat %s/HAConfig.conf |grep "cluster_id"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % config_path
            self.local_cluster_id = os.popen(local_cluster_id_cmd).readlines()[0].replace('\n', '')
        else:
            self.printlog('\033[1;31mError:file HAConfig.conf do not exist!!!,ErrorCode:0x0011\033[0m',1)

        # get master_timestamp and state
        file_check = os.path.exists('%s/HAState.stat' % config_path)
        if file_check:
            local_master_timestamp_cmd = 'cat %s/HAState.stat |grep "master_timestamp"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % config_path
            self.local_master_timestamp = os.popen(local_master_timestamp_cmd).readlines()[0].replace('\n', '')

            local_master_state_cmd = 'cat %s/HAState.stat |grep "used_state"|awk -F ":" \'{print $2}\'' % config_path
            self.local_master_state = os.popen(local_master_state_cmd).readlines()[0].replace('\n', '')
        else:
            self.printlog('\033[1;31mError:file HAState.stat do not exist,ErrorCode:0x0011\033[0m',1)

        # get mysql user and password
        local_user_cmd = 'cat %s/Mysql.conf |grep "user_security"|awk -F "\\"" \'{print $4}\' |base64 -d' % config_path
        self.local_user = os.popen(local_user_cmd).readlines()[0].replace('\n', '')

        local_password_cmd = 'cat %s/Mysql.conf |grep "password_security"|awk -F "\\"" \'{print $4}\' |base64 -d' % config_path
        self.local_password = os.popen(local_password_cmd).readlines()[0].replace('\n', '')
        #used to print in log
        local_password_cmd = 'cat %s/Mysql.conf |grep "password_security"|awk -F "\\"" \'{print $4}\'' % config_path
        local_password = os.popen(local_password_cmd).readlines()[0].replace('\n', '')

        local_password_changed_cmd = 'cat %s/Mysql.conf |grep "security_has_changed"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % config_path
        local_pw_changed=os.popen(local_password_changed_cmd).readlines()
        if local_pw_changed:
            self.local_password_changed = os.popen(local_password_changed_cmd).readlines()[0].replace('\n', '')
        else:
            self.printlog('\033[1;31mLocal<%s>,In this version <security_has_changed> do not exist,it is normal!!!\033[0m'%self.local_business_ip)

        #version
        version_cmd = '%s/DBHAServer64 -V |grep "version" |awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % bin_path
        version = os.popen(version_cmd).readlines()[0].replace('\n', '')
        svnrevision_cmd = '%s/DBHAServer64 -V |grep "svn revision" |awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % bin_path
        svnrevision = os.popen(svnrevision_cmd).readlines()[0].replace('\n', '')
        compiletime_cmd = '%s/DBHAServer64 -V |grep "compile time" |awk -F "compile time:" \'{print $2}\'' % bin_path
        compiletime = os.popen(compiletime_cmd).readlines()[0].replace('\n', '')
        mysqlinfo_cmd = 'mysql -V'
        mysqlinfo = os.popen(mysqlinfo_cmd).readlines()

        #peer
        peer_version=''
        peer_svnrevision=''
        peer_compiletime=''
        peer_password=''
        peer_mysqlinfo=''
        if not self.peer_business_ip:
            self.printlog('\033[1;31mpeer_business_ip is null,The machine maybe single or not init!!!\033[0m')
        else:
            peer_install_flag_cmd = 'ssh %s@%s sudo cat %s/DBHA.conf |grep "DBHA_install_flag"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % (self.ssh_user,self.peer_business_ip, config_path)
            self.peer_install_flag = os.popen(peer_install_flag_cmd).readlines()[0].replace('\n', '')

            try:
                peer_uid_cmd = 'ssh %s@%s sudo cat %s/HAConfig.conf |grep "uid"|awk -F "\\"" \'{print $4}\'' % (self.ssh_user, self.peer_business_ip, config_path)
                self.peer_uid = os.popen(peer_uid_cmd).readlines()[0].replace('\n', '')

                peer_cluster_id_cmd = 'ssh %s@%s sudo cat %s/HAConfig.conf |grep "cluster_id"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % (self.ssh_user, self.peer_business_ip, config_path)
                self.peer_cluster_id = os.popen(peer_cluster_id_cmd).readlines()[0].replace('\n', '')
            except:
                self.printlog('\033[1;31mcat file HAConfig.conf error,maybe not exist!!!\033[0m')

            try:
                peer_master_timestamp_cmd = 'ssh %s@%s sudo cat %s/HAState.stat |grep "master_timestamp"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % (self.ssh_user, self.peer_business_ip, config_path)
                self.peer_master_timestamp = os.popen(peer_master_timestamp_cmd).readlines()[0].replace('\n', '')

                peer_master_state_cmd = 'ssh %s@%s sudo cat %s/HAState.stat |grep "used_state"|awk -F ":" \'{print $2}\'' % (self.ssh_user, self.peer_business_ip, config_path)
                self.peer_master_state = os.popen(peer_master_state_cmd).readlines()[0].replace('\n', '')
            except:
                self.printlog('\033[1;31mcat file HAState.stat error,maybe not exist!!!\033[0m')

            peer_user_cmd = 'ssh %s@%s sudo cat %s/Mysql.conf |grep "user_security"|awk -F "\\"" \'{print $4}\' |base64 -d' % (self.ssh_user, self.peer_business_ip, config_path)
            self.peer_user = os.popen(peer_user_cmd).readlines()[0].replace('\n', '')

            peer_password_cmd = 'ssh %s@%s sudo cat %s/Mysql.conf |grep "password_security"|awk -F "\\"" \'{print $4}\'|base64 -d' % (self.ssh_user, self.peer_business_ip, config_path)
            self.peer_password = os.popen(peer_password_cmd).readlines()[0].replace('\n', '')
            # used to print in log
            peer_password_cmd = 'ssh %s@%s sudo cat %s/Mysql.conf |grep "password_security"|awk -F "\\"" \'{print $4}\'' % (self.ssh_user, self.peer_business_ip, config_path)
            peer_password = os.popen(peer_password_cmd).readlines()[0].replace('\n', '')

            peer_password_changed_cmd = 'ssh %s@%s sudo cat %s/Mysql.conf |grep "security_has_changed"|awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' % (self.ssh_user, self.peer_business_ip, config_path)
            peer_pw_changed = os.popen(local_password_changed_cmd).readlines()
            if peer_pw_changed:
                self.peer_password_changed = os.popen(peer_password_changed_cmd).readlines()[0].replace('\n', '')
            else:
                self.printlog('\033[1;31mPeer<%s>,In this version <security_has_changed> do not exist,it is normal!!!\033[0m'%self.peer_business_ip)

            #version
            peer_version_cmd='ssh %s@%s sudo %s/DBHAServer64 -V |grep "version" |awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\''%(self.ssh_user, self.peer_business_ip,bin_path)
            peer_version=os.popen(peer_version_cmd).readlines()[0].replace('\n', '')
            peer_svnrevision_cmd = 'ssh %s@%s sudo %s/DBHAServer64 -V |grep "svn revision" |awk -F ":" \'{print $2}\'|awk -F "," \'{print $1}\'' %(self.ssh_user, self.peer_business_ip,bin_path)
            peer_svnrevision = os.popen(peer_svnrevision_cmd).readlines()[0].replace('\n', '')
            peer_compiletime_cmd = 'ssh %s@%s sudo %s/DBHAServer64 -V |grep "compile time" |awk -F "compile time:" \'{print $2}\'' %(self.ssh_user, self.peer_business_ip,bin_path)
            peer_compiletime = os.popen(peer_compiletime_cmd).readlines()[0].replace('\n', '')
            peer_mysqlinfo_cmd = 'ssh %s@%s sudo mysql -V'%(self.ssh_user,self.peer_business_ip)
            peer_mysqlinfo = os.popen(peer_mysqlinfo_cmd).readlines()

        self.printlog('\n---------------------check base info start--------------------')
        self.printlog('\033[1;31mVersion info\033[0m')
        self.printlog('lcoal MySQL version:%s - peer MySQL version:%s'%(mysqlinfo,peer_mysqlinfo))
        self.printlog('lcoal DBHA version:%s - peer DBHA version:%s'%(version.strip(),peer_version.strip()))
        self.printlog('local svn revision:%s - peer svn revision:%s' %(svnrevision.strip(),peer_svnrevision.strip()))
        self.printlog('local compile time:%s - peer compile time:%s\n' % (compiletime.strip(),peer_compiletime.strip()))
        if svnrevision.strip() != peer_svnrevision.strip() and self.peer_business_ip !='':
            self.printlog('ERROR:local svn revision:%s not equal peer svn revision:%s,ErrorCode:0x0003' % (compiletime.strip(), peer_compiletime.strip()),1)

        self.printlog('\033[1;31mThread info\033[0m')
        self.check_thread()

        self.printlog('\033[1;31mConfig info\033[0m')
        self.printlog('vip:\033[1;31m%s\033[0m,vip_netmask:%s' % (self.vip, self.vip_netmask))
        if self.vip == '' or self.vip_netmask == '':
            self.printlog('\033[1;31mError:vip or vip_netmask is null,please deploy ,ErrorCode:0x0008\033[0m',1)

        self.printlog('local_business_ip:\033[1;31m%s\033[0m,local_heart_ip:%s' % (self.local_business_ip, self.local_heart_ip))
        self.printlog('peer_business_ip:\033[1;31m%s\033[0m,peer_heart_ip:%s' % (self.peer_business_ip, self.peer_heart_ip))
        if self.local_heart_ip == '' or (self.peer_heart_ip == '' and self.peer_business_ip !=''):
            self.printlog('\033[1;31mError:local_heart_ip or peer_heart_ip is null,please deploy heart ip,ErrorCode:0x0009\033[0m',1)

        self.printlog('heartbeart_if_name:%s,if_name:%s,vif_name:\033[1;31m%s\033[0m' % (self.heartbeart_if_name, self.if_name, self.vif_name))
        self.printlog('local_install_flag:%s,peer_install_flag:%s' % (self.local_install_flag, self.peer_install_flag))
        self.printlog('backup_path:%s\n' % self.backup_path)

        self.printlog('local_uid:%s,peer_uid:%s' % (self.local_uid, self.peer_uid))
        self.printlog('local_cluster_id:%s,peer_cluster_id:%s\n' % (self.local_cluster_id, self.peer_cluster_id))
        if self.local_cluster_id != self.peer_cluster_id and self.peer_business_ip !='':
            self.printlog('\033[1;31mError:local_cluster_id not equal peer_cluster_id,ErrorCode:0x0010\033[0m',1)

        local_otherStyleTime=''
        peer_otherStyleTime=''
        if self.local_master_timestamp:
            local_timeArray = time.localtime(float(self.local_master_timestamp))
            local_otherStyleTime = time.strftime("%Y-%m-%d %H:%M:%S", local_timeArray)
        else:
            peer_otherStyleTime=''
            
        if self.peer_master_timestamp:
            peer_timeArray = time.localtime(float(self.peer_master_timestamp))
            peer_otherStyleTime = time.strftime("%Y-%m-%d %H:%M:%S", peer_timeArray)
        else:
            peer_otherStyleTime=''
        self.printlog('local_master_timestamp:%s,peer_master_timestamp:%s' % (local_otherStyleTime, peer_otherStyleTime))
        self.printlog('local_master_state:%s,peer_master_state:%s\n' % (self.local_master_state, self.peer_master_state))

        self.printlog('local_user:%s,peer_user:%s' % (self.local_user, self.peer_user))
        self.printlog('local_password:%s,peer_password:%s' % (local_password, peer_password))
        if self.local_password != self.peer_password and self.peer_business_ip !='':
            self.printlog('\033[1;31mError:local_password not equal peer_password,ErrorCode:0x0011\033[0m',1)
        self.printlog('local_password_changed:%s,peer_password_changed:%s' % (self.local_password_changed, self.peer_password_changed))
        self.printlog('---------------------check base info end--------------------\n')

    def check_vip(self):
        self.printlog('---------------------check vip start--------------------')
        # local
        ifconfig_cmd = 'ifconfig |grep %s' % self.vip
        value = os.popen(ifconfig_cmd).readlines()
        if value:
            self.printlog('\033[1;31mINFO:vip:%s is up on %s!!!\033[0m' % (self.vip, self.local_business_ip),1)
            self.master_ip=self.local_business_ip
            self.master_mysql_user = self.local_user
            self.master_mysql_password = self.local_password
            self.slave_ip=self.peer_business_ip
            self.slave_mysql_user=self.peer_user
            self.slave_mysql_password=self.peer_password
            self.bool_vip=True
        else:
            if self.peer_business_ip:
                ifconfig_cmd = 'ssh %s@%s sudo ifconfig |grep %s' % (self.ssh_user, self.peer_business_ip, self.vip)
                value = os.popen(ifconfig_cmd).readlines()
                if value:
                    self.printlog('\033[1;31mINFO:vip:%s is up on %s!!!\033[0m' % (self.vip, self.peer_business_ip),1)
                    self.master_ip = self.peer_business_ip
                    self.master_mysql_user = self.peer_user
                    self.master_mysql_password = self.peer_password
                    self.slave_ip = self.local_business_ip
                    self.slave_mysql_user = self.local_user
                    self.slave_mysql_password = self.local_password
                    self.bool_vip = True
                else:
                    self.printlog('\033[1;31mError:vip:%s is not exist,ErrorCode:0x0012\033[0m\n' % self.vip,1)
            else:
                self.printlog('\033[1;31mError:vip:%s is not exist,ErrorCode:0x0012\033[0m\n' % self.vip,1)
        self.printlog('---------------------check vip end--------------------\n')

    def check_mysqlstate(self):
        slave_io=''
        slave_sql=''
        master_log_file_name=''
        master_log_file_position=0

        if self.peer_business_ip and self.slave_ip and self.master_ip:
            self.printlog('---------------------check mysql status start--------------------')
            try:
                slave_io_cmd='mysql -u%s -p%s -h%s -e "show slave status\\G" | grep -w "Slave_IO_Running" | awk -F": " \'{print $2}\''%(self.slave_mysql_user,self.slave_mysql_password,self.slave_ip)
                slave_io = os.popen(slave_io_cmd).readlines()[0].replace('\n', '')
                slave_sql_cmd = 'mysql -u%s -p%s -h%s -e "show slave status\\G" | grep -w "Slave_SQL_Running" | awk -F": " \'{print $2}\'' % (self.slave_mysql_user, self.slave_mysql_password, self.slave_ip)
                slave_sql = os.popen(slave_sql_cmd).readlines()[0].replace('\n', '')
            except:
                self.printlog('\033[1;31mError:the MySQL<salve:%s> connect failed,ErrorCode:0x0006\033[0m\n'%self.slave_ip,1)

            try:
                master_log_file_name_cmd = 'mysql -u%s -p%s -h%s -e "show master status\\G" | grep -w "File" | awk -F": " \'{print $2}\'' % (self.master_mysql_user, self.master_mysql_password, self.master_ip)
                master_log_file_name = os.popen(master_log_file_name_cmd).readlines()[0].replace('\n', '')
                master_log_file_position_cmd = 'mysql -u%s -p%s -h%s -e "show master status\\G" | grep -w "Position" | awk -F": " \'{print $2}\'' % (self.master_mysql_user, self.master_mysql_password, self.master_ip)
                master_log_file_position = os.popen(master_log_file_position_cmd).readlines()[0].replace('\n', '')
            except:
                self.printlog('\033[1;31mError:the MySQL<master:%s> connect failed,ErrorCode:0x0006\033[0m\n' % self.master_ip,1)

            if slave_io != 'Yes'or slave_sql != 'Yes':
                slave_sql_cmd = 'mysql -u%s -p%s -h%s -e "show slave status\\G"' % (self.slave_mysql_user, self.slave_mysql_password, self.slave_ip)
                slave_status=os.popen(slave_sql_cmd).readlines()
                self.printlog('\033[1;31mError:the slave status is abnormal,ErrorCode:0x0013\033[0m',1)
                for line in slave_status:
                    line=line.replace('\n', '')
                    self.printlog(line)
            else:
                self.printlog('INFO:the slave status<Slave_IO_Running:%s,Slave_SQL_Running:%s>' % (slave_io, slave_sql),1)
                self.bool_mysqlstate=True

            self.printlog('the master status<File:%s,Position:%s>' % (master_log_file_name, master_log_file_position))

            if self.bool_vip and self.bool_mysqlstate:
                self.printlog('\033[1;31mINFO:The DBHA is runing normal,you can verify this by the follow way,see code:0x0000\033[0m',1)

            self.printlog('---------------------check mysql status end--------------------\n')

    def check_thread(self):
        #local
        if self.local_business_ip:
            self.printlog('Local<%s>'% self.local_business_ip,1)
            keep_thread_cmd = 'ps -ef | grep /bin/sh | grep %s/DBHAKeeper.sh | awk \'{print $2}\''%shell_path
            keep_thread = os.popen(keep_thread_cmd).readlines()
            if keep_thread:
                self.printlog('INFO:the DBHAKeeper is running!!!',1)
            else:
                self.printlog('\033[1;31mError:the DBHAKeeper is not running,ErrorCode:0x0004\033[0m',1)

            mysql_thread_cmd = 'pidof mysqld'
            mysql_thread = os.popen(mysql_thread_cmd).readlines()
            if mysql_thread:
                self.printlog('INFO:the MySQL is running!!!',1)
                try:
                    sql_test_cmd = 'mysql -u%s -p%s -h%s -e "select 1;"' % (self.local_user, self.local_password, self.local_business_ip)
                    sql_test = os.popen(sql_test_cmd).readlines()[0].replace('\n', '')
                    if sql_test == '1':
                        self.printlog('the MySQL connect success!!!\n')
                    #else:
                    #    self.printlog('\033[1;31mError:the MySQL connect failed,ErrorCode:0x0006\033[0m\n',1)
                except:
                    self.printlog('\033[1;31mError:the MySQL connect failed,ErrorCode:0x0006\033[0m\n',1)
                    try:
                        init_passwd_cmd = 'echo ZGFodWFjbG91ZAo= |base64 -d'
                        init_passwd = os.popen(init_passwd_cmd).readlines()[0].replace('\n', '')
                        mysqltest_cmd='mysql -u%s -p%s -h%s -e "select 1;"'%(self.local_user,init_passwd,self.local_business_ip)
                        mysqltest = os.popen(mysqltest_cmd).readlines()[0].replace('\n', '')
                        if mysqltest == '1':
                            self.printlog('\033[1;31mError:the MySQL<%s> password is not match it in the MySql.conf,ErrorCode:0x00014\033[0m\n'%self.local_business_ip,1)
                    except:
                        pass
            else:
                self.printlog('\033[1;31mError:the MySQL is not running,ErrorCode:0x0005\033[0m\n',1)

            dbha_thread_cmd='pidof DBHAServer64'
            dbha_thread=os.popen(dbha_thread_cmd).readlines()
            if dbha_thread:
                self.printlog('INFO:the DBHA is running!!!',1)
            else:
                self.printlog('\033[1;31mError:the DBHA is not running,ErrorCode:0x0007\033[0m',1)

        #peer
        if self.peer_business_ip:
            self.printlog('Peer<%s>'%self.peer_business_ip,1)
            keep_thread_cmd = 'ssh %s@%s sudo ps -ef | grep /bin/sh | grep %s/DBHAKeeper.sh | awk \'{print $2}\''%(self.ssh_user, self.peer_business_ip,shell_path)
            keep_thread = os.popen(keep_thread_cmd).readlines()
            if keep_thread:
                self.printlog('Info:the DBHAKeeper is running!!!',1)
            else:
                self.printlog('\033[1;31mError:the DBHAKeeper is not running,ErrorCode:0x0004\033[0m',1)

            mysql_thread_cmd = 'ssh %s@%s sudo ps -ef |grep mysqld |grep -v grep' %(self.ssh_user,self.peer_business_ip)
            mysql_thread = os.popen(mysql_thread_cmd).readlines()
            if mysql_thread:
                self.printlog('INFO:the MySQL is running!!!',1)
                try:
                    sql_test_cmd = 'mysql -u%s -p%s -h%s -e "select 1;"' % (self.peer_user, self.peer_password, self.peer_business_ip)
                    sql_test = os.popen(sql_test_cmd).readlines()[0].replace('\n', '')
                    if sql_test == '1':
                        self.printlog('the MySQL connect success!!!\n')
                #    else:
                #        self.printlog('\033[1;31mError:the MySQL connect failed,ErrorCode:0x0006\033[0m\n', 1)
                except:
                    self.printlog('\033[1;31mError:the MySQL connect failed,ErrorCode:0x0006\033[0m\n', 1)
                    try:
                        init_passwd_cmd = 'echo ZGFodWFjbG91ZAo= |base64 -d'
                        init_passwd = os.popen(init_passwd_cmd).readlines()[0].replace('\n', '')
                        mysqltest_cmd='mysql -u%s -p%s -h%s -e "select 1;"'%(self.peer_user,init_passwd,self.peer_business_ip)
                        mysqltest = os.popen(mysqltest_cmd).readlines()[0].replace('\n', '')
                        if mysqltest == '1':
                            self.printlog('\033[1;31mError:the MySQL<%s> password is not match it in the MySql.conf,ErrorCode:0x00014\033[0m\n'%self.peer_business_ip,1)
                    except:
                        pass
            else:
                self.printlog('\033[1;31mError:the MySQL is not running,ErrorCode:0x0005\033[0m\n', 1)

            dbha_thread_cmd = 'ssh %s@%s sudo ps -ef |grep DBHAServer64 |grep -v grep'%(self.ssh_user,self.peer_business_ip)
            dbha_thread = os.popen(dbha_thread_cmd).readlines()
            if dbha_thread:
                self.printlog('INFO:the DBHA is running!!!',1)
            else:
                self.printlog('\033[1;31mError:the DBHA is not running,ErrorCode:0x0007\033[0m',1)

    def check_DBHA_install(self):
        self.printlog('---------------------check dbha install start--------------------')
        #local
        local_ip_cmd = 'cat %s/DBHA.conf |grep "local_business_IP"|awk -F "\\"" \'{print $4}\'' % config_path
        self.local_business_ip = os.popen(local_ip_cmd).readlines()[0].replace('\n', '')
        if self.local_business_ip == '':
            self.printlog('Error:local_business_ip is null,please ensure installed the DBHA,ErrorCode:0x0001\n',1)
            return -1
        self.printlog('Local<%s>'%self.local_business_ip)
        mysql_install_cmd='sudo cat /root/installMysql.log |grep "install mysql success"'
        mysql_install=os.popen(mysql_install_cmd).readlines()
        if mysql_install:
            self.printlog('MySQL install success!!!')
        else:
            mysql_install_cmd = 'sudo cat /root/installMysql.log |grep "change mysql data successfully"'
            mysql_install = os.popen(mysql_install_cmd).readlines()
            if mysql_install:
                self.printlog('MySQL install success!!!')
            else:
                self.printlog('\033[1;31mMySQL install failed!!!\033[0m')

        dbha_install_cmd = 'sudo cat /root/installDBHA.log |grep "init DBHA success"'
        dbha_install = os.popen(dbha_install_cmd).readlines()
        if dbha_install:
            self.printlog('DBHA install success!!!')
        else:
            dbha_install_cmd = 'sudo cat /root/installDBHA.log |grep "mkdir service link"'
            dbha_install = os.popen(dbha_install_cmd).readlines()
            if dbha_install:
                self.printlog('DBHA install success!!!')
            else:
                dbha_install_cmd = 'sudo cat /root/installMysql.log |grep "install DBHA failed,configure not exist!"'
                dbha_install = os.popen(dbha_install_cmd).readlines()
                if dbha_install:
                    self.printlog('\033[1;31mDBHA install failed,do not have file: /etc/mysql_private.configure,ErrorCode:0x0002\033[0m',1)
                else:
                    self.printlog('\033[1;31mDBHA install failed!!!\033[0m')

        peer_ip_cmd = 'cat %s/DBHA.conf |grep "peer_business_IP"|awk -F "\\"" \'{print $4}\'' % config_path
        self.peer_business_ip = os.popen(peer_ip_cmd).readlines()[0].replace('\n', '')

        if self.peer_business_ip:
            self.printlog('Peer<%s>' % self.peer_business_ip)
            mysql_install_cmd = 'ssh %s@%s sudo cat /root/installMysql.log |grep "install mysql success"'%(self.ssh_user, self.peer_business_ip)
            mysql_install = os.popen(mysql_install_cmd).readlines()
            if mysql_install:
                self.printlog('MySQL install success!!!')
            else:
                mysql_install_cmd = 'ssh %s@%s sudo cat /root/installMysql.log |grep "change mysql data successfully"' % (self.ssh_user, self.peer_business_ip)
                mysql_install = os.popen(mysql_install_cmd).readlines()
                if mysql_install:
                    self.printlog('MySQL install success!!!')
                else:
                    self.printlog('\033[1;31mMySQL install failed!!!\033[0m')

            dbha_install_cmd = 'ssh %s@%s sudo cat /root/installDBHA.log |grep "init DBHA success"'%(self.ssh_user, self.peer_business_ip)
            dbha_install = os.popen(dbha_install_cmd).readlines()
            if dbha_install:
                self.printlog('DBHA install success!!!')
            else:
                dbha_install_cmd = 'ssh %s@%s sudo cat /root/installDBHA.log |grep "mkdir service link"' %(self.ssh_user, self.peer_business_ip)
                dbha_install = os.popen(dbha_install_cmd).readlines()
                if dbha_install:
                    self.printlog('DBHA install success!!!')
                else:
                    dbha_install_cmd = 'ssh %s@%s sudo cat /root/installDBHA.log |grep "install DBHA failed,configure not exist!"'%(self.ssh_user, self.peer_business_ip)
                    dbha_install = os.popen(dbha_install_cmd).readlines()
                    if dbha_install:
                        self.printlog('\033[1;31mDBHA install failed,do not have file: /etc/mysql_private.configure,ErrorCode:0x0002\033[0m',1)
                    else:
                        self.printlog('\033[1;31mDBHA install failed!!!\033[0m')
        else:
            self.printlog('WARN:peer_business_ip is null,please ensure install the DBHA single,otherwise there are someting wrong',1)

        self.printlog('---------------------check dbha install end--------------------\n')

    def check_election_log(self):
        self.printlog('---------------------check DBHA_election_log_current.log start--------------------')
        #local
        election_log_cmd = 'cat %s/DBHA_election_log_current.log | head -n 30' % log_path
        election_log = os.popen(election_log_cmd).readlines()
        if election_log:
            self.printlog('\033[1;31mLocal<%s>,the election log is:\033[0m' % self.local_business_ip)
            for line in election_log:
                line = line.replace('\n', '')
                self.printlog(line)
            self.printlog("---")

        election_log_cmd = 'cat %s/DBHA_election_log_current.log | tail -n 30' % log_path
        election_log = os.popen(election_log_cmd).readlines()
        if election_log:
            for line in election_log:
                line = line.replace('\n', '')
                self.printlog(line)
        #peer
        if self.peer_business_ip:
            election_log_cmd = 'ssh %s@%s sudo cat %s/DBHA_election_log_current.log | head -n 30' %(self.ssh_user, self.peer_business_ip,log_path)
            election_log = os.popen(election_log_cmd).readlines()
            if election_log:
                self.printlog('\033[1;31mPeer<%s>,the election log is:\033[0m' % self.peer_business_ip)
                for line in election_log:
                    line = line.replace('\n', '')
                    self.printlog(line)
                self.printlog("---")

            election_log_cmd = 'ssh %s@%s sudo cat %s/DBHA_election_log_current.log | tail -n 30' %(self.ssh_user, self.peer_business_ip,log_path)
            election_log = os.popen(election_log_cmd).readlines()
            if election_log:
                for line in election_log:
                    line = line.replace('\n', '')
                    self.printlog(line)
        self.printlog('---------------------check DBHA_election_log_current.log end--------------------')

    def check_dbha_current_log(self):
        self.printlog('---------------------check DBHA_log_current.log start--------------------')
        # local
        dbha_current_log_cmd = 'cat %s/DBHA_log_current.log | head -n 100' % log_path
        dbha_current_log = os.popen(dbha_current_log_cmd).readlines()
        if dbha_current_log:
            self.printlog('\033[1;31mLocal<%s>,the current log is:\033[0m' % self.local_business_ip)
            for line in dbha_current_log:
                line = line.replace('\n', '')
                self.printlog(line)
            self.printlog("---")

        dbha_current_log_cmd = 'cat %s/DBHA_log_current.log | tail -n 100' % log_path
        dbha_current_log = os.popen(dbha_current_log_cmd).readlines()
        if dbha_current_log:
            for line in dbha_current_log:
                line = line.replace('\n', '')
                self.printlog(line)
        # peer
        if self.peer_business_ip:
            dbha_current_log_cmd = 'ssh %s@%s sudo cat %s/DBHA_log_current.log | head -n 100' % (self.ssh_user, self.peer_business_ip, log_path)
            dbha_current_log = os.popen(dbha_current_log_cmd).readlines()
            if dbha_current_log:
                self.printlog('\033[1;31mPeer<%s>,the current log is:\033[0m' % self.peer_business_ip)
                for line in dbha_current_log:
                    line = line.replace('\n', '')
                    self.printlog(line)
                self.printlog("---")

            dbha_current_log_cmd = 'ssh %s@%s sudo cat %s/DBHA_log_current.log | tail -n 100' % (self.ssh_user, self.peer_business_ip, log_path)
            dbha_current_log = os.popen(dbha_current_log_cmd).readlines()
            if dbha_current_log:
                for line in dbha_current_log:
                    line = line.replace('\n', '')
                    self.printlog(line)
        self.printlog('---------------------check DBHA_log_current.log start--------------------')

    def check_shell_log(self):
        self.printlog('---------------------check Shell.log start--------------------')
        #local
        try:
            file_size=os.path.getsize('%s/Shell.log'%log_path)
            if file_size >1024*1024*200:
                self.printlog('\033[1;31mWARN:Local<%s>The size of Shell.log is big than 200MB\033[0m\n'%self.local_business_ip)
                return
        except:
            pass

        start_cmd = 'cat %s/Shell.log |grep \'start server by command\' -A 30 | tail -n 31' % log_path
        local_start = os.popen(start_cmd).readlines()
        if local_start:
            self.printlog('\033[1;31mLocal<%s>,the start log is:\033[0m' %self.local_business_ip)
            for line in local_start:
                line=line.replace('\n', '')
                self.printlog(line)
        else:
            self.printlog('\033[1;31mLocal<%s> not find the <start> in Shell.log\033[0m\n' % self.local_business_ip)

        startup_cmd='cat %s/Shell.log |grep startup -A 30 | tail -n 31'%log_path
        local_startup=os.popen(startup_cmd).readlines()
        if local_startup:
            self.printlog('\033[1;31mLocal<%s>,the startup log is:\033[0m' % self.local_business_ip)
            for line in local_startup:
                line = line.replace('\n', '')
                self.printlog(line)
        else:
            self.printlog('\033[1;31mLocal<%s> not find the <startup> in Shell.log\033[0m\n' %self.local_business_ip)

        install_cmd = 'cat %s/Shell.log |grep \'DBHA is ready to install\' -A 50 | tail -n 51' % log_path
        local_install = os.popen(install_cmd).readlines()
        if local_install:
            self.printlog('\033[1;31mLocal<%s>,the install log is:\033[0m' % self.local_business_ip)
            for line in local_install:
                line = line.replace('\n', '')
                self.printlog(line)
        else:
            self.printlog('\033[1;31mLocal<%s> not find the <DBHA is ready to install> in Shell.log\033[0m\n' % self.local_business_ip)

        #peer
        if self.peer_business_ip:
            try:
                peer_file_size_cmd='ssh %s@%s sudo ls -l %s/Shell.log | awk \'{print $5}\''%(self.ssh_user, self.peer_business_ip,log_path)
                peer_file_size = os.popen(peer_file_size_cmd).readlines()[0].replace('\n', '')
                if peer_file_size > 1024 * 1024 * 200:
                    self.printlog('\033[1;31mWARN:Peer<%s>The size of Shell.log is big than 200MB\033[0m\n'% self.peer_business_ip)
                    return
            except:
                return

            start_cmd = 'ssh %s@%s sudo cat %s/Shell.log |grep \'start server by command\' -A 30 | tail -n 31' %(self.ssh_user, self.peer_business_ip,log_path)
            peer_start = os.popen(start_cmd).readlines()
            if peer_start:
                self.printlog('\033[1;31mPeer<%s>,the start log is:\033[0m' % self.peer_business_ip)
                for line in peer_start:
                    line = line.replace('\n', '')
                    self.printlog(line)
            else:
                self.printlog('\033[1;31mPeer<%s> not find the <start> in Shell.log\033[0m\n' % self.peer_business_ip)

            startup_cmd = 'ssh %s@%s sudo cat %s/Shell.log |grep startup -A 30 | tail -n 31' %(self.ssh_user, self.peer_business_ip,log_path)
            peer_startup = os.popen(startup_cmd).readlines()
            if peer_startup:
                self.printlog('\033[1;31mPeer<%s>,the startup log is:\033[0m' % self.peer_business_ip)
                for line in peer_startup:
                    line = line.replace('\n', '')
                    self.printlog(line)
            else:
                self.printlog('\033[1;31mpeer<%s> not find the <startup> in Shell.log\033[0m\n' % self.peer_business_ip)

            install_cmd = 'ssh %s@%s sudo cat %s/Shell.log |grep \'DBHA is ready to install\' -A 50 | tail -n 51' %(self.ssh_user, self.peer_business_ip,log_path)
            peer_install = os.popen(install_cmd).readlines()
            if peer_install:
                self.printlog('\033[1;31mPeer<%s>,the install log is:\033[0m' % self.peer_business_ip)
                for line in peer_install:
                    line = line.replace('\n', '')
                    self.printlog(line)
            else:
                self.printlog('\033[1;31mpeer<%s> not find the <DBHA is ready to install> in Shell.log\033[0m\n' % self.peer_business_ip)
        self.printlog('---------------------check Shell.log end--------------------\n')

    def printlog(self,message,print_message = 0):
        if self.ssh_user != "root":
            if print_message == 1:
                print(message)
        if self.ssh_user == "root":
            if print_message == 1:
                print(message)
                logging.info(message)
            else:
                logging.info(message)

if __name__ == "__main__":
    check=DBHACheck()
    check.call_tip()
    check.set_log()
    check.printlog('\033[1;31m********************check DBHA start********************\033[0m')
    ret=check.check_DBHA_install()
    if ret == -1:
        exit
    ret=check.read_baseinfo()
    if ret != -1:
        check.check_vip()
        check.check_mysqlstate()
    check.check_shell_log()
    check.check_election_log()
    check.printlog('\033[1;31m********************check DBHA end********************\033[0m')



