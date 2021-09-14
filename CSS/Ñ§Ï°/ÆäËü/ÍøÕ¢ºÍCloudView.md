* 由于业务段和存储段使用的虚拟ip是 192 网段，本机ping不通，所以使用CloudView需要配置网闸，将其映射到能ping通的 10 网段



* 管理平台-系统管理-配置网闸

  ![image-20210715192631552](C:\Users\307298\AppData\Roaming\Typora\typora-user-images\image-20210715192631552.png)

  第一行是MDS节点，其他是DN存储节点

  ![image-20210715192844731](C:\Users\307298\AppData\Roaming\Typora\typora-user-images\image-20210715192844731.png)

* 使用CloudView前需要先在云存储管理平台添加用户，CloudView登录账号密码为该用户的账号密码
  ip为网闸映射出来的MDS ip，端口为38100





---

CloudView进不去？？？