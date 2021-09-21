### 安装

* [获取 mysql yum 源](https://dev.mysql.com/downloads/repo/yum/)
* 下载 mysql yum 源：`wget https://dev.mysql.com/get/xxxxxx.noarch.rpm`
* 安装 mysql yum 源：`yum -y localinstall xxxxxx.noarch.rpm`
* 使用 `yum` 安装：`yum -y install mysql-community-server`

### 启动

* `systemctl start mysqld`
* 设置开机启动：`systemctl enable mysqld systemctl daemon-reload`

### 初始登录

* 初始密码：`grep 'password' /var/log/mysqld.log`

* `mysql -uroot -p`

* 登录后修改密码

  ```bash
  ALTER USER 'root'@'localhost' IDENTIFIED BY '123456';
  ```

* mysql 配置文件： `/etc/my.cnf`

  ```shell
  [mysqld]
  ......
  datadir=/var/lib/mysql
  socket=/var/lib/mysql/mysql.sock
  # Disabling symbolic-links is recommended to prevent assorted security risks
  symbolic-links=0
  log-error=/var/log/mysqld.log
  pid-file=/var/run/mysqld/mysqld.pid
  ```

### 问题

* 修改密码报错

  ```bash
  ERROR 29 (HY000): File './mysql/user.MYD' not found (Errcode: 13 - Permission denied)
  mysql> flush privileges;
  ERROR 1820 (HY000): You must reset your password using ALTER USER statement before executing this statement.
  ```

* 原因：datadir 目录下文件拥有者和组未设置成mysql

```bash
[root@localhost ~]# ll /var/lib/
drwxr-x--x. 5 root    root    4096 Sep 20 01:08 mysql
```

* 解决
```bash
[root@localhost lib]# chown -R mysql /var/lib/mysql
```

