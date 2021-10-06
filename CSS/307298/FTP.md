* 配置文件 /etc/vsftpd/vsftpd.conf

  * 设置匿名访问

    ```
    // 匿名访问 默认是YES？
    anonymous_enable=YES
    // 匿名目录设置，默认是在/var/ftp
    anon_root=/home/ftp
    ......
    ```
    
  * 匿名访问：不需要用户名和密码登录

  * 匿名目录设置后，需要设置**权限为755**，否则不能匿名访问（如777也不行）

  * 匿名目录不是默认值时，需要设置 **setenforce 0**（Permissive），否则访问的目录显示不出来

  * 设置完需要重启服务

* /etc/init.d/vsftpd 
  restart  stop  start  status