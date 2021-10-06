# 鸟哥的Linux私房菜

## 计算机概论

* 五大单元：输入、输出、内存、控制单元、算数逻辑单元
* RISC(Reduced Instruction Set Computer)  ARM等
  CISC(Complex ...)  x86

* 32位/64位表示CPU一次数据读写的最大值
* 外频、倍频，总线宽度，超线程
* 北桥整合到CPU内，CPU与内存、显卡沟通的zongxian位系统总线，南桥是IO总线
* DRAM：SDRAM、DDR；SRAM
* 内存多通道设计
* AGP、PCI、PCIe
* 传统硬盘：圆形盘片、机械手臂、主轴马达
  盘片：扇区（sector 512B/4KB）、磁道（track）、柱面（cylinder）
  MSDOS、GPT
* HDD SSD
* 传输接口：SATA、SAS、USB
* IOPS 每秒读写操作次数
* IO位址与IRQ中断信号
* CMOS：纪律主板上的重要参数，如date、CPU电压和频率、I/O位址、IRQ等
  BIOS：写入到主板某一块flash或EEPROM的程序，写死再内存芯片中ROM，没通电也能记录数据
* OS核心功能：系统调用接口、程序管理、内存管理、文件系统管理、设备的驱动

## Linux学习概述

## 磁盘相关

### 设备与文件名

| 设备                | 设备在Linux内的文件名                                        |
| ------------------- | ------------------------------------------------------------ |
| SCSI/SATA/USB硬盘机 | /dev/sd[a-p]\[1-128]                                         |
| USB闪存盘           | /dev/sd[a-p] （与SATA相同）                                  |
| VirtI/O界面         | /dev/vd[a-p] （用于虚拟机内）                                |
| 软盘机              | /dev/fd[0-7]                                                 |
| 打印机              | /dev/lp[0-2] （25针打印机） /dev/usb/lp[0-15] （USB 接口）   |
| 鼠标                | /dev/input/mouse[0-15] （通用） /dev/psaux （PS/2界面） /dev/mouse （当前鼠标） |
| CDROM/DVDROM        | /dev/scd[0-1] （通用） /dev/sr[0-1] （通用，CentOS 较常见） /dev/cdrom （当前 CDROM） |
| 磁带机              | /dev/ht0 （IDE 界面） /dev/st0 （SATA/SCSI界面） /dev/tape （当前磁带） |
| ~~IDE硬盘机~~       | /dev/hd[a-d] （旧式系统才有）                                |

### 磁盘结构

![image-20210817134717820](.\Linux学习.assets\image-20210817134717820.png)

* 扇区（sector）、磁道（track）、盘片、磁头、柱面（cylinder）
  
  * **扇区**：**最小读写单元**（512B或4KB）
    
  * 磁道的周长越大，扇区面积也越大，但扇区的容量都是一样的
* 一个盘片可能有两个盘面，每个盘面有一个磁头
  * **磁头**：连接在同一个磁臂，“共进退”
  * **柱面**：各个盘面相同磁道组成的一个立体的空心圆柱，linux中是按柱面来分配空间。
  
    ```bash
    Disk /dev/sde: 62.7 GB, 62746787840 bytes
    255 heads, 63 sectors/track, 7628 cylinders
    Units = cylinders of 16065 * 512 = 8225280 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00000000
       Device Boot      Start         End      Blocks   Id  System
    /dev/sde1               1        1897    15231999+  ee  GPT
    ```
  
  * 如上，有255个磁头，63扇区/磁道，7628柱面
  * Units（一个柱面的大小） =  磁头数 * 扇区数 * 扇区大小 = 255 * 63 * 512
  * 第1柱面 - 第1897柱面分配给 sde1 分区 （Start和End），15231999+逻辑块
  * **磁盘块**：虚拟出来的，由2^n个扇区组成，是**操作系统对硬盘读写的最小单元**
    （柱面号，盘面号，扇区号）定位一个磁盘块
  * 存储容量 ＝ 磁头数 × 磁道(柱面)数 × 每道扇区数 × 每扇区字节数
  * page：内存与操作系统操作的小小单元

### MBR与GPT分区表

* MBR（MSDOS）
  * 第一个扇区512B：主要开机记录（MBR）446B和分区表64B
  * 分区表仅有64B，仅能写入四组分区信息，称为主要分区或延伸分区
  * 分区最小单位通常为柱面
  * 延伸分区（最多只能有一个）也可以记录分区信息，从而达到分更多分区，成为逻辑分区
* GPT（GUID partition table）
  * LBA逻辑区块位址（默认每个是512B），前34个LBA区块记录分区信息，磁盘最后33个LBA会用来备份分区信息
  * LBA0（MBR相容区块，类似MRB）、LBA1（GPT表头记录）、LBA2-33（实际记录分区信息）
  * 每个LBA可以记录4个分区记录，最多总共可以有4*32个分区记录，每个记录提供64bits记录开始和结束扇区号码，单一分区最大容量`2^64 * 512B = 8T`

### 开机流程

#### BIOS MBR/GPT 

Basic Input Output System

* BIOS -> MBR -> 开机管理程序 boot loader（如Grub） -> 核心文件
  （若为GPT，LBA0 仅提供第一阶段的开机管理程序码，需要额外分区出BIOS boot分区存放grub，2MB左右？ ）
* boot loader
  * 提供菜单
  * 载入核心文件（内核）
  * 转交其它loader

#### UEFI BIOS GPT

Unified Extensible Firmware Interface

| 比较项目               | 传统 BIOS                                                    | UEFI               |
| ---------------------- | ------------------------------------------------------------ | ------------------ |
| 使用程序语言           | 组合语言                                                     | C 语言             |
| 硬件资源控制           | 使用中断 （IRQ） 管理 <br />不可变的内存存取<br />不可变得输入/输出存取 | 使用驱动程序与协定 |
| 处理器运行环境         | 16 位                                                        | CPU 保护模式       |
| 扩充方式               | 通过 IRQ 链接                                                | 直接载入驱动程序   |
| 第三方厂商支持         | 较差                                                         | 较佳且可支持多平台 |
| 图形化能力             | 较差                                                         | 较佳               |
| 内置简化操作系统前环境 | 不支持                                                       | 支持               |

### 磁盘分区

* 目录树结构：根目录 /
* 挂载：利用一个目录作为进入点，将磁盘分区数据放置在该目录
* 文件系统类型：ext2/ext3/ext4、xfs、vfat
  CentOS7默认使用 xfs 作为文件系统  



## Linux 基本命令

* 热键
  * Ctrl+c、Ctrl+d 中断程序、exit
  * Tab 自动补全
  * shift+PageUP/PageDown 翻页
  * ==Ctrl + Alt + F1~F6== 终端切换

* man page 
  * `[cmd] + --help`
  * `man + [cmd]`
  * man搜寻路径配置文件 `/etc/man_db.conf `
    通常目录 `/usr/share/man`

| 代号 | 代表内容                                                     |
| ---- | ------------------------------------------------------------ |
| 1    | **一般账号可用指令或可可执行文件**                           |
| 2    | 系统核心可调用的函数与工具等                                 |
| 3    | 一些常用的函数（function）与函数库（library），大部分为C的函数库（libc） |
| 4    | 设备文件的说明，通常在/dev下的文件                           |
| 5    | **配置文件或者是某些文件的格式**                             |
| 6    | 游戏（games）                                                |
| 7    | 惯例与协定等，例如Linux文件系统、网络协定、ASCII code等等的说明 |
| 8    | **系统管理员常用指令**                                       |
| 9    | 跟kernel有关的文件                                           |

| 代号        | 内容说明                             |
| ----------- | ------------------------------------ |
| NAME        | 简短的指令、数据名称说明             |
| SYNOPSIS    | 简短的指令下达语法（syntax）简介     |
| DESCRIPTION | 较为完整的说明，这部分最好仔细看看！ |
| ...         | ......                               |
| EXAMPLE     | 一些可以参考的范例                   |

| 按键    | 进行工作                                                     |
| ------- | ------------------------------------------------------------ |
| ...     | ......                                                       |
| /string | 向“下”搜寻 string 这个字串，如果要搜寻 vbird 的话，就输入 /vbird |
| ?string | 向“上”搜寻 string 这个字串                                   |
| n, N    | 利用 / 或 ? 来搜寻字串时，用 n 来继续下一个搜寻，N反向搜索   |
| q       | 结束这次的 man page                                          |

* info page
  * 类似man page
  * 独立的页面 node 、 超链接、易读

* documents
  * `/usr/share/doc`

* nano 文本编辑器

* 观察系统使用状态
  * `netstat -a`  网络状态
  * `ps -aux` 查看进程
* `sync`  将buffer中未写入硬盘的数据强制写入硬盘
* `shutdown`、`reboot`、`halt`、`poweroff`
* `systemctl + [cmd] ` 所有服务的管理命令 （CentOS7）

## 文件权限与目录配置

* Owner、Group、Others

### 文件权限

文件属性

```bas
[root @svn307298 ~/EfsTools]# ll
total 8
-rwxr-xr-x. 1 root root  36 Aug  5 09:41 auto_make.sh
drwxr-xr-x. 2 root root  26 Aug 16 09:48 bin
```

* 第一栏：文件类型与权限
  `-rwxr-xr-x.`
  * 第1个字符：`d`目录文件、`-`正规文件、`l`链接文件、`b`块设备（存储）文件、`c`字符设备文件、`s`数据接口文件、`p`数据输送档（FIFO pipe）
  * 234、457、890：分别是文件拥有者、群组、其它账号的可读、可写、可执行权限（`rwx`），没有权限（`-`）
* 第二栏：==i-node==
* 第三栏、第四栏：拥有者账号、所属群组
* 第五栏：文件大小（单位Byte），如果是目录则不是表示该目录的总大小
* 第六栏：修改日期 （`--fulle-time`显示完整时间）
* 第七栏：文件名，文件名前面有`.`代表隐藏文件，需要`-a`显示
  * 文件名颜色：蓝色目录、绿色可执行文件、红色压缩文件、浅蓝链接、黄色设备、白色其它；（红色闪烁链接有问题）

### 文件属性与权限修改

* `chgrp [group] [file]`

  * group必须是在 `/etc/group` 文件里存在
  * `-R`  ：recursive 递归改变目录下所有文件

* `chown [user] [file]`

* `chmod [xyz] [file]`

  * x、y、z的值是  `r:4/w:2/x:1`   的累加值，分别代表owner、group、others的权限

    `chmod 755 file` 表示开启owner可读可写可执行权限，group和others的可读可执行权限

  * u、g、o、a 以及  +、-、=

    `chmod o+w` 表示添加others的可写权限

| 元件 | 内容     | 叠代物件   | r        | w        | x                |
| ---- | -------- | ---------- | -------- | -------- | ---------------- |
| 文件 | 详细数据 | 文件数据夹 | 文件内容 | 文件内容 | 执行文件内容     |
| 目录 | 文件名   | 可分类抽屉 | 文件名   | 文件名   | 进入该目录的权限 |

### 目录配置

FHS Filesystem Hierarchy Standard


|              | shareable                                                    | unshareable                                       |
| :----------: | :----------------------------------------------------------- | ------------------------------------------------- |
|  **static**  | /usr （软件放置处） <br />/opt （第三方协力软件）            | /etc （配置文件）<br />/boot （开机与核心档）     |
| **variable** | /var/mail （使用者邮件信箱） <br />/var/spool/news （新闻群组） | /var/run （程序相关）<br />/var/lock （程序相关） |

* / ：与开机系统有关
* /usr (unix software resource)：与软件安装、执行有关
* /var (variable)：与系统运行过程有关



* 建议：**根目录（/）所在分区应该越小越好， 软件最好不要与根目录放在同一个分区内，保持根目录越小越好。 如此性能较佳且根目录所在的文件系统也较不容易发生问题。**  

* 必须存在的目录：

  * /bin  可执行文件目录
  * /boot  开机相关文件，如kernel （vmlinuz）
  * /dev  设备文件
  * /etc  配置文件    
  * /lib  函数库    /lib/modules  核心相关模块
  *  /media  挂载软盘、光盘、DVD设备
  * /mnt  暂时挂载额外的设备
  * /opt  第三方软件防止目录
  * /run  开机后产生的各项信息
  * /sbin  开机所需要的命令，包括开机、修复、还原系统
  * /srv  服务所需要的数据目录
  * /usr：/usr/bin、/usr/lib、/usr/local、/usr/sbin、/usr/share
  * /var：**/var/log**（messages内核日志、wtmp登录信息）、/var/run（服务或程序启动后，PID存放在此）、/var/spool、/var/lib

* 建议存在的目录

  * /home  使用者的主文件夹
  * /root  系统管理员的主文件夹
  * /proc  该目录本身是虚拟文件系统，放置的数据都是在内存当中，如系统核心、进程、网络状态等。(不占硬盘空间)
  * /sys  也是虚拟的文件系统，类似/proc，记录核心与系统硬件相关的信息，包括已载入的kernel和一些硬件设备信息等

* CentOS7将许多在 / 的目录挪到 /usr，并进行链接设置：

  ```bas
  /bin --> /usr/bin
  /sbin --> /usr/sbin
  /lib --> /usr/lib
  /lib64 --> /usr/lib6
  /var/lock --> /run/lo
  /var/run --> /run
  ```

## 文件与目录管理

### 绝对路径与相对路径

* 绝对路径：从根目录开始 /
* 相对路径：相对于当前路径
  * .  ：当前目录，同 ./
  * .. ：上一层目录，同 ../
* 其它特殊目录
  * \-  ：前一个目录
  * ~  ：使用者的主文件夹
  * ~account：account账号的主文件夹

### 目录相关命令

* `cd`：change directory 变更目录
* `pwd`：print working directory 显示目前所在目录 （-P正确的目录，而不是链接目录）
* `mkdir`：创建新目录  `mkdir dir`
  * -p：上层目录若不存在，则同时创建上层目录 `mkdir -p dir1/dir2`
  * -m：给予相关权限 `mkdir -m 755 dir`
* `rmdir`：删除空目录

### $PATH 可执行文件路径

```bash
[root @svn307298 ~]# echo $PATH
.:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/xbin/:/root/bin
```

* 相同命令在不同目录，且都在`$PATH`中，则PATH中先被查询的先执行
* 添加目录到 `$PATH`

```bash
PATH="${PATH}:/dirname"
```

* 本目录（.）最好不要放在PATH中

### 文件相关命令

（包括目录文件命令）

ls、cp、rm、mv、basename、dirname

* `ls [-aAdfFhilnrRSt] dir`：列出目录下的文件名，按文件名排序

  * **-a**：全部文件
  * **-d**：仅列出目录
  * **-l**：长数据串行出，包含文件属性权限等，同 `ll` 
  * -h：大小以人类易读的方式列出
  * -i：列出 inode 号
  * -n：列出UID与GID而不是使用者和群组名
  * -t：以时间排序
  * -S：以文件大小排序
  * -r：排序结果反向输出
  * -R：子目录内容一起列出
  * --color=never/always/auto：不显示颜色/显示颜色/系统自行判断是否给予颜色
  * --full-time：完整时间
  * --time={atime,ctime}：输出access时间atime或改变权限时间ctime而非内容变更时间mtime

* `cp [-adfilprsu] src dest`：复制文件或目录
  `cp [...] src1 src2 src3 ... dest(dir)`

  * **-a**：同 -dr --presserve=all
  * **-i**：若目标文件已存在，则覆盖时先询问
  * **-p**：文件属性（权限、用户、**时间**）一起复制（备份常用）
  * **-r**：递归复制，用于**目录的复制**
  * **-f**：强制
  * -d：若源文件为链接文件的属性，则复制链接文件属性而非文件本身
  * -s：复制（文件）成为符号链接（软链接）
  * -l：复制（文件）为硬式链接
  * -u：dest 比 src 旧或者dest不存在才更新复制到dest （备份常用）
  * --preserve=all：除了-p外，还加入SELinux属性，links，xattr也复制

* `rm [-fir] file/dir`：删除目录或文件

  * -f：force
  * -i：询问
  * -r：递归删除**目录**

* `mv [-fiu] src dest`：移动文件或目录 、更名
  `mv [...] src1 src2 src3 ... dest(dir)`

  * -f -i：同上
  * -u：dest比src旧，才会更新

* `basename` 和 `dirname ` 获取文件路径的文件名和目录名（脚本常用）

  ```bash
  [root @svn307298 ~/EfsTools]# basename /root/EfsTools/Makefile 
  Makefile
  [root @svn307298 ~/EfsTools]# dirname /root/EfsTools/Makefile 
  /root/EfsTools
  ```

### 文件内容相关命令

* `cat [-AbEnTv] file` ：concatenate（连续），将文件内容连续输出在屏幕
  * **-n**：打印出行号，包括空白行
  * -b：打印行号，仅对非空白行标行号 
  * -A：相当于 -vET
  * -E：将换行字符$显示出来
  * -v：列出看不出来的特殊字符
  * -T：将`[tab]`以 `^I` 显示
* `tac`：同cat，从最后一行到第一行反过来输出
* `nl [-bnw] file`：添加行号的打印内容
  * -b a：列出行号，包括空行，同cat -n
    -b  t：不列出空行(默认)
  * -n ln：行号在最左方
    -n rn：行号在字段最右方，不加0
    -n rz：行号在字段最右方，加0
  * -w：行号字段占用字符数

* `more file`：翻页查看文件

  * [space]：下翻页
  * [Enter]：下翻行
  * /string：向下查找string
  * :f：显示文件名以及当前行号
  * q：退出
  * b：上翻页

* `less file`：翻页查看

  * 翻页同上
  * /string 和 ?string：向下查找和向上查找
  * n 和 N：重复查找和反向重复查找
  * g 和 G：前进到第一行和最后一行

* `head [-n num] file`：取出文件前面num行(空行不算num，但会显示出来)

  * head -n 10 等同于 head -10

  ```bash
  head -n -10 file # 读取倒数第10行之前的内容
  ```

* `tail [-n num] file`：取出文件后面num行

  * -f：持续监测后面几行

  ```bash
  tail -10f file # tail -n 10 -f file
  tail -n +10 file # 读取第10行以后开始的内容
  ```

* `od [-t TYPE] file`：非纯文本文件，如二进制文件

  * ==TYPE==:

    * a：默认字符
    * c：ASCII字符
    * d[size]：十进制输出，每个整数占size Bytes
    * f [size]：浮点数输出，每个整数占size Bytes
    * o[size]：八进制输出，每个整数占size Bytes
    * x[size]：十六进制输出，每个整数占size Bytes

    ```bash
    # 显示八进制和ASCII
    echo password | od -t oCc
    ```

* 文件时间：

  * mtime：文件内容修改时更新(默认)
  * ctime：文件状态（权限、属性）修改时更新
  * atime：文件被取用时更新，如cat文件后就会更新

* `touch [-acdmt] file` 创建空文件，或修改文件时间（atime、mtime）

  * -a：修改access time
  * -c：仅修改文件时间，若文件不存在则不创建新文件
  * -m：修改mtime
  * -d：接想要修改的日期，同--date="日期或时间"
  * -t：接想要修改的时间，格式[YYYYMMDDhhmm]

### 默认权限与隐藏权限

* `umask [-S] [num]`：创建文件或目录时的权限默认值查看、设置

  ```bash
  [root @svn307298 ~/tmp]# umask
  0022     #0022 后三位表示7-0  7-2  7-2，即755
  [root @svn307298 ~/tmp]# umask -S
  u=rwx,g=rx,o=rx
  
  [root @svn307298 ~/tmp]# umask 003
  [root @svn307298 ~/tmp]# umask -S
  u=rwx,g=rwx,o=r
  ```

* `chattr [+-=][ASacdistu] [file/dir]`：设置文件隐藏属性

  * **a：设置后只能增加数据，不能删除或修改数据**
  * **i：不能被删除、改名、设置链接，也无法写入或新增数据，安全性高**
  * A：存取文件时，atime不会被修改，避免I/O较慢机器过度的存取磁盘
  * S：将非同步写入磁盘更改为同步写入磁盘(无缓冲)
  * c：设置后，自动将文件压缩，读取时自动解压缩
  * d：使该文件不会被dump备份
  * s：文件被删除后将会被完全移除出硬盘空间（误删无法补救）
  * u：与s相反，删除后，数据其实还在磁盘中
  * （xfs文件系统仅支持AadiS）

* `lsattr [-adR] [file/dir]`

  * -a：将隐藏文件的属性也列出来
  * -d：如果时目录，只列出目录本身的属性
  * -R：同子目录的一起列出

* SUID/SGID/SBIT权限（==十六章==）

  * SUID/SGID：仅对二进制程序有效，执行者需具备x权限，执行者将具有owner的权限；SUID是 **owner** 的 **x** 权限为 **s** 标志；SGID是 **group** 的 **x** 权限为 **s** 标志

  * SBIT（StickyBit）：仅针对目录有效，当使用者在该目录下创建文件或目录时，仅有自己与root才有权删除该文件，无法删除别人的文件；**others**的 **x** 权限为 **t**

  * SUID 4、SGID 2、SBIT 1

  * 设置权限：

    ```bash
    # 在原先原先的 755 前面
    chmod 4755 file/dir # 表示设置SUID权限
    ```

  * 若user、group、others本身没有x的权限，则用大写代替 S 、T，代表是空的SUID、SGID、SBIT权限

* `file [file]` ：观察文件类型

### 命令和文件搜寻

* `which [-a] [command]` ：寻找可执行文件、命令
  * -a：将PATH目录符合的命令全部列出，而不是只列出第一个找到的
* `whereis [-bmsu] [file/dir]` ：特定的目录中寻找文件文件名
  * **-l**：列出会去查询的几个主要目录
  * -b：只找 binary 格式的文件
  * -m：只找在说明文档 manual 路径下的文件
  * -s：只找source来源文件
  * -u：寻找不在上述三个项目中的其它特殊文件
* `locate [ir] keyword` 
  * **-i**：忽略大小写
  * -c：不输出文件名，仅计算找到的文件数量
  * -l：仅输出几行
  * **-S**：输出所使用的数据库文件相关信息
  * -r：接正则表达式的显示方式
  * 数据库：/var/lib/mlocate，默认每天执行一次更新数据库
  * 手动更新数据库：`updatedb` ，根据/etc/updatedb.conf的设置搜寻并更新上面的数据库
* `find [PATH] [option] [action]` 
  * 与时间有关选项，-atime、-ctime、-mtime
    * -mtime n：列出n天之前的**一天内**被更改过的文件
    * -mtime +n：列出n天之前（不含n天）被更改过内容的文件
    * -mtime -n：列出n天之前含n天被更改过内容的文件
    * -newer file：file为存在的文件，列出比file还要新的文件
  * 与使用者或群组有关：
    * -uid n：寻找使用者id为n的文件
    * -gid n：寻找群组id为n的文件
    * -user name：寻找使用者账号为name的文件
    * -group name：寻找群组为name的文件
    * -nouser：寻找文件拥有者不存在/etc/passwd的人的文件
    * -nogroup：类似nouser
  * 与文件权限和名称有关：
    * **-name filename**
    * -size [+-]SIZE：寻找比 SIZE 大或小的文件，规格有c（Byte）、k（KByte），例子 `-size +50k`
    * -type TYPE：寻找文件类型为TYPE的。类型：正规文件`f`、设备文件`b c`、目录`d`、链接`l`、socket `s`、FIFO `p`等
    * -perm mode：寻找文件权限刚好等于mode的文件，mode 如 4755
    * -perm -mode：寻找文件权限必须全部囊括mode的权限
    * -perm /mode：寻找文件权限包含任一mode的文件
  * 其它：
    * -exec [cmd]：接额外的命令来处理搜索到的结果
      `-exec ls -l {} \  `：命令从-exec到 \ 结束，find的结果会被放到 {}
    * -print：打印输出到屏幕，默认

## 文件系统

### 文件系统基本概念

* **superblock**：记录此 filesystem 的整体信息，包括inode/block的总量、使用量、剩余量， 以及文件系统的格式与相关信息等 
* **inode**：记录文件的属性和权限，一个文件占用一个inode，同时记录此文件的数据所在的 block 号码
* **block**：实际记录文件的内容，若文件太大时，会占用多个 block  



* 索引式文件系统（通过inode索引到block） vs U盘闪存（FAT 无inode）
* 磁盘重组：block过于离散导致性能变差时，可使同一文件所属的blocks汇整在一起，使数据容易读取。（FAT需要，索引式的一般不需要）

### EXT2

![image-20210823215229430](.\Linux学习.assets\image-20210823215229430.png)

#### data block

* 文件系统限制

  * block大小和数量在格式化完不能再改变了（除非再格式化）

  * 每个block内最多只能放一个文件的数据
  * 若文件小于block，则该block剩余容量不能再被使用（空间浪费）

| Block 大小       | 1KB  | 2KB   | 4KB  |
| ---------------- | ---- | ----- | ---- |
| 最大单一文件限制 | 16GB | 256GB | 2TB  |
| 大文件系统总容量 | 2TB  | 8TB   | 16TB |

#### inode table

* 记录的数据：rwx、owner、group、容量、ctime、atime、mtime、flag（如SetUID）、文件内容指针 pointer

* inode 特点：

  * 大小固定为128Byte（ext4 和 xfs 可设置到 256Byte）、每笔block记录4Byte

  * 每个文件仅会占用一个inode

  * 文件系统能创建的文件数量与inode数量有关

  * 读取文件先找到 inode，分析inode所记录的权限和使用者，若符合才能开始读取block内容

  * inode记录block号码的区域：12个直接、1间接、1双间接、1三间接记录区

    以1K block为例，最大总额：

    * 12 * 1K
    * 1K / 4B = 256 * 1K
    * 256 * 256 *1K
    * 256 * 256 *256 * 1K
    * = 16GB

#### Superblock

* 记录的数据：block与inode总量、未使用与已使用的inode/block数量、block与inode大小（block 1/2/4K，inode 128/256B）、filesystem挂在时间、最后一次写入数据时间、最后一次检验磁盘（fsck）时间等、一个valid bit数值（已挂载该值为0，否则为1）
* 1024Byte



FilesystemDescription、block bitmap、inode bitmap、

#### dumpe2fs命令

* `dumpe2fs [-h] 设备文件名`：查询Ext家族 superblock 信息
  * -h：仅列出superblock的数据

### 与目录树关系

* 目录：分配一个inode与至少一块block给一个目录
* `ls -li ...` 查看inode号码
* 目录大小为 1024整数倍
* inode不记录文件名，文件名记录是在block中

### 新增文件过程

* 确定使用者欲新增文件的目录是否有w和x权限，有的话才能新增
* 根据inode bitmap 找未使用的inode号码，并写入权限、属性等
* 根据block bitmap 找未使用的block号码，写入实际数据到block，并更新inode的block指针
* 将inode与block数据同步更新 inode/block bitmap，更新superblock

中介数据：superblock, inode bitmap 及 block bitmap 的数据是经常变动  

### 日志式文件系统

* 数据不一致状态：写入时遇突发状况，未更新中介数据
* 日志记录，简化一致性检查：
  * 预备：先在日志区块记录文件准备要写入的信息
  * 实际写入：开始写入文件权限和数据，更新metadata
  * 结束：完成数据与metadata更新后，在日志记录区块中完成该文件的记录



### XFS

* EXT家族支持度广，但格式化超慢



* xfs数据分布：
  * 数据区（data section）：
    * block大小 512B - 64K，由于linux内存控制，最多 4K
    * inode大小 256B - 2M，默认256B
  * 活动登录区（log section）：日志
  * 实时运行区（realtime section）：extent区块，创建文件先放到此处，4K-1G，默认64K

* `xfs_info 设备文件名/挂载点` ：类似ext的dumpe2fs，观察superblock

### 文件系统相关命令

#### 磁盘与目录容量

* `df [-ahikHTm] [file/dir]` ：文件系统磁盘使用量
  * **-h**：以较易阅读的格式，GB、MB、KB
  * **-i** ：不用磁盘容量，以inode数量显示
  * **-T**：连同分区的文件系统名称也列出
  * -a：列出所有的文件系统，包括特有的 /proc
  * -k、-m：以KB、MB容量显示
  * -H：以 M=1000K 取代 M=1024K 的进位

**/dev/shm 是利用内存虚拟出来的磁盘空间，通常是总实体内存从一半**

* `du [-ahskm] file/dir`：查看文件或目录所占容量，默认是KB
  * **-s**：列出总量，不列出每个目录占用量
  * -h、-k、-m：易读格式
  * -a：列出所有文件与目录
  * -S：不包括目录下的总计，与-s有点差别

#### 链接

* Hard Link 实体链接：`ln src dest`
  * 新增一笔文件名链接到同一个inode号码
  * 同inode的两个文件，除文件名外，其它所有信息都一样
  * 不论修改哪个文件，均能对数据进行修改
  * 若仅删除一个文件名，inode与block都还会存在
  * 不增加inode和block占用
  * **限制：不能跨文件系统，不能link目录**
* **Symbolic Link 符号链接**：`ln -s 源文件/目录 链接文件`
  * -f：链接文件存在时，则先删除再创建
  * 可链接目录
  * 会占用 inode 与 block，链接文件的大小由 src 的英文个数决定，每个英文占用 1Byte
  * 源文件被删除了，链接就会打不开，显示红色

### 分区、格式化、检验、挂载

#### 分区

* `lsblk [-dfimpt] [device]`
  * -d：仅列出磁盘本身，不列出磁盘的分区数据
  * -f：同时列出文件系统名称
  * -i：使用ASCII的线段，不使用复杂编码
  * -m：输出再/dev 下面的权限数据
  * -p：列出完整文件名，即含/dev...
  * -t：列出设备详细数据，包括伫列机制、与读写的数据流等

* `blkid`：列出设备的 uuid
* `parted [dev] print`：列出磁盘分区表类型和分区信息

* `gdisk/fdisk 设备名`：进行磁盘分区，MBR使用fdisk，GPT使用gdisk

* `partprobe -s `: 更新核心分区表信息
  `cat /proc/partitions`

#### 格式化

* `mkfs.xfs [-b bsize] [-d parms] [-i parms] [-L label] [-f] [-r parms] 设备名`
  * -b：block容量，512B-4k
  * -d：与data section的相关参数值
    * agcount=数值：设置几个存储群组，通常与CPU有关
    * agsize=数值：每个AG设置多少容量
    * file：指格式化时文件而不是设备的意思，如虚拟磁盘
    * size=数值：data section的容量
    * su=数值：当有RAID时，stripe数值的意思
    * sw=数值：当有RAID时，用于储存数据的磁盘数量
    * sunit=数值：与su相当，不过单位是使用sector的数量
    * swidth=数值：su*sw，单位是sector的数量
  * -f：如果设备已经有文件系统，需要强制格式化
  * -i：与inode相关的设置
    * size=数值：256Byte - 2k
    * ......
  * -L：后面接文件系统的 Label name 标签名称
  * -r：realtime section相关设置值，常见的有
    * extsize=数值：extent数值，一般不须设置，但又RAID时，最好与swidth相同，4K-1G

* `mkfs.ext4 [-b size] [-L label] 设备名`
* 其它文件系统 mkfs 同上

#### 文件系统检验

* `xfs_repair [-fnd] 设备名` 处理XFS文件系统
  * -f：后面的设备名是文件而不是实体设备
  * -n：仅检查
  * -d：单人维护模式下，针对 / 进行检查修复

* `fsck.ext4 [-pf] [-b superblock] 设备名`
  * -p：修复时，进行询问
  * -f：强制进入细部检查
  * -D：针对文件系统下的目录进行最优化配置
  * -b：接superblock 位置，一般用不到。当superblock损毁时，尝试救援
  * **注意：通常文件系统有问题才使用这个命令，否则可能对系统造成危害**

#### 挂载与卸载

**挂载注意**：

* 单一文件系统不应该被重复挂载在不同的挂载点  
* 单一目录不应该重复挂载多个文件系统  
* 要作为挂载点的目录，理论上应该都是空目录才是  



* **mount**：
  * `mount [-l]` ：显示目前挂载信息
  * `mount -a` ：依照配置文件 **/etc/fstab**   的数据将未挂载的挂载上来
  * `mount 设备名 挂载点`
  * ~~-t：加上文件系统种类指定欲挂载的类型~~
  * -n：默认时，系统会将实际挂载情况实时写入 **/etc/mtab**中，加-n则不会刻意写入
  * -o：额外参数
    * async，sync：非同步、同步内存机制
    * **remount：重新挂载，在系统出错时或更改参数时，很有用**
    * ......
* `umount [-fn] 设备文件或挂载点`
  * -f：强制
  * -l：立即卸载，比-f还强制
  * -n：不更新 **/etc/mtab** 情况下卸载
  * **卸载前需要离开该挂载点**

#### 磁盘、文件系统参数修订

* 设备代码：主要设备代码（Major）、次要设备代码（Minor）

* `mknod 设备文件名 [bcp] [Major] [Minor] ` ：
  * b c p ：设备文件类型
* `xfs_admin [-lu] [-L label] [-U uuid] 设备文件名`：修改XFS文件系统的UUID和Label
  * -l -u：列出Label、UUID

* `tune2fs [-l] [-L Label] [-U uuid] 设备文件名  `：修改 ext4 的 label 与 UUID  



### 开机挂载

#### /etc/fstab 和 /etc/mtab

* **/etc/fstab**：开机时要挂载的设备的配置文件
  * 第四栏：文件系统参数：async/sync、auto/noauto、rw/ro、exec/noexec、user/nouser、suid/nosuid、default（具备rw,suid,dev,exec,auto,nouser,async）
  * 第五栏、第六栏：能否被dump备份、是否以fsck检查扇区
* /etc/mtab：挂载记录

#### 特殊设备loop挂载

* 挂载镜像文件

  `mount -o loop 镜像文件 挂载点`

* 创建大文件制作 loop 设备

  * 创建大文件，如下是创建512个block，每个block 1MB的512M文件
    `dd if=/dev/zero of=/srv/loopdev bs=1M count=512` 

  * 格式化大文件 `mkfs.xfs -f /srv/loopdev  `，-f 强制对文件格式化

  * 挂载该大文件到一个挂载目录
    `blkid /srv/loopdev`

    `/srv/loopdev: UUID="7dd97bd2-4446-48fd-9d23-a8b03ffdd5ee" TYPE="xfs"  `

    `mount -o loop UUID="7dd97bd2-4446-48fd-9d23-a8b03ffdd5ee" /mnt  `

### 内存交换空间 swap

#### 实体分区创建swap

* gdisk 分区
* `mkswap /dev/xxx` 创建swap格式
* `free` 查看内存和swap
* `swapon /dev/xxx` 载入swap
* `swapon -s` ：列出使用swap的设备
* 写入开机配置文件 /etc/fstab ：
  `UUID="6b17e4ab-9bf9-43d6-88a0-73ab47855f9d" swap swap defaults 0 0`

#### 使用文件创建swap

* `dd if=/dev/zero of=/tmp/swap bs=1M count=128  `
* 其它同上
* swapoff xxx 关掉swapfile

### GNU parted分区

所有分区表都支持

* `parted [设备] [指令 [参数]]  `
  * mklabel mbr/gpt
  * mkpart [primary|logical|extended] [ext4|vfat|xfs] [Start] [End]  ：新增分区
  * print：显示分区
  * rm [partition]：删除分区



## 压缩 打包 备份

### 常用压缩命令

*.Z           compress 程序压缩的文件；
*.zip 	   zip 程序压缩的文件；
*.gz         gzip 程序压缩的文件；
*.bz2       bzip2 程序压缩的文件；
*.xz          xz 程序压缩的文件；
*.tar         tar 程序打包的数据，并没有压缩过；
*.tar.gz    tar 程序打包的文件，其中并且经过 gzip 的压缩
*.tar.bz2  tar 程序打包的文件，其中并且经过 bzip2 的压缩
*.tar.xz     tar 程序打包的文件，其中并且经过 xz 的压缩  

#### gzip，zcat/zmore/zless/zgrep

* `gzip [-cdtv#] 文件名  `
  * -c：输出到屏幕
  * -d：解压缩参数
  * -t：检验压缩文件的一致性
  * -v：显示出压缩比
  * -#：#为数字，代表压缩等级，-1最快压缩比最差，-9最慢压缩比最好，-6默认
  * **压缩后原本文件不再存在**
* zcat、zmore、zless、zgrep 读取纯文本文件被压缩后的压缩文件
* egrep



#### bzip2，bzcat/bzmore/bzless/bzgrep

* `bzip2 [-cdkzv#] 文件名`
  * -k：保留原始文件
  * -z：压缩的参数
  * 其它同 gzip

#### xz，xzcat/xzmore/xzless/xzgrep

* `xz [-dtlkc#] 文件名  `
  * -l：列出压缩文件的相关信息
  * 其它同上

#### tar 打包

* `tar [-z|j|J][cv][-f 待创建的新文件名] filename` ：打包压缩

  `tar [-z|j|J][tv][-f 既有的tar文件名]` ：查看文件名

  `tar [-z|j|J][xv][-f 既有的tar文件名] [-C 目录]`：解压缩

  * -c：创建打包文件
  * -t：查看
  * -x：解压缩，配合 -C 解压缩在特定目录
  * -z：gzip压缩、解压缩，文件名最好为 *.tar.gz
  * -j：bzip2压缩、解压缩，文件名最好为 *.tar.bz2
  * -J：xz压缩、解压缩，文件名最好为 *.tar.xz
  * -v：显示文件名
  * -p：保留原本权限和属性
  * -P：保留绝对路径
  * --exclude=FILE：不要将 FILE 打包
  * --newer-time=”xxxx/xx/xx“ ：仅打包比某时刻还要新的文件

* 解开单一文件：
  `tar -ztvf 压缩包 | grep 文件名` 先找到要解开的文件
  `tar -zxvf 压缩包 待解开文件名`

### XFS文件系统的备份、还原

* xfsdump 不支持没有挂载的文件系统备份
* xfsdump 必须使用 root 的权限才能操作 （涉及文件系统的关系）
* xfsdump 只能备份 XFS 文件系统
* xfsdump 备份下来的数据只能让 xfsrestore 解析
* xfsdump 是通过文件系统的 UUID 来分辨各个备份文件的，因此不能备份两个具有相同 UUID 的文件系统喔 

#### 备份

* `xfsdump [-L S_label] [-M M_label] [-l #] [-f 备份文件] 待备份数据  `
  `xfsdump -I`
  * ==-L：记录备份的session标头==
  * ==-M：记录储存媒体的标头==
  * -l：等级，默认0完整备份
  * -I：从/var/lib/xfsdump/inventory列出目前备份的信息状态

* 累积备份 -l 1~9

#### 还原

* `xfsrestore -I` ：查看备份文件数据

  `xfsrestore [-f 备份文件] [-L S_label] [-s] 待复原目录  `：单一文件全系统复原

  `xfsrestore [-f 备份文件] -r 待复原目录`：累积备份文件复原

  `xfsrestore [-f 备份文件] -i 待复原目录`：询问

  * -s：仅复原一个文件或目录



### 光盘写入工具

* `mkisofs [-o 镜像文件] [-Jrv] [-V vol] [-m file] 待备份文件`



### 其它常见压缩与备份工具

#### dd

默认一个个扇区读写，可备份完整的 partition或者磁盘设备

* `dd if=src of=dest`
  `dd if="input_file" of="output_file" bs="block_size" count="number"  `

#### cpio

* 配合find和数据流重导向进行备份

* `cpio -ovcB > [file|device]` ：备份

  * -o：将数据 copy 输出到文件或设备上  
  * -B：默认的Blocks可以增至5120B，默认512B

* `cpio -ivcdu < [file|device]`：还原

  `cpio -ivct < [file|device]`：查看

  * -i：将数据从文件或设备copy到系统中
  * -v：显示在屏幕
  * -d：自动创建目录
  * -u：自动将较新文件覆盖较旧文件
  * -t：配合-i，查看
  * -c：新的 portable format 方式储存



## vi、vim

* 一般命令模式：可以删除、复制、粘贴、光标移动等
* 编辑模式：按下`i I o O a A r R`进入编辑模式，按`Esc`退出编辑模式
* 命令行模式：在一般命令模式，按下` : / ? ` 进入
  可进行搜寻、读取、存盘、替换字符、离开、显示行号等待

### 一般命令模式

#### 移动光标

* `h/j/k/l` ：左下上右，（`30j `向下30行）
* ......
* `Home、End、PageUp、PageDown`
* `G、gg`：最后一行、第一行

#### 搜索、替换

* `/word `和 `?word`：向下或向上找word
* `n、N`：正向、反向重复搜索动作
* `:[start],[end]s/word1/word2/g`
  * 从start到end行寻找word1替换为word2
  * 1,$s 第一行到最后一行
  * 结尾加 `c` 表示询问确认

#### 删除、复制、粘贴

* `x、X`：向后、向前删除一个字符
* `dd`：删除光标所在行
  `ndd`：删除光标所在向下n行
* `yy`：复制光标所在行
  `nyy`：复制光标所在向下n行
* `p、P`：在光标下一行、上一行粘贴
* **u：复原上一个动作**
* **[Ctrl] + r：重做上一个动作**
* `.`：重复前一个动作

### 编辑模式

进入编辑模式

* `i、I`：从光标所在插入，从目前所在行第一个非空白插入
* **a、A：从光标所在的下一个字符插入，从光标所在列最后一个字符插入**
* `o、O`：光标所在下一行插入新一行，光标所在上一行插入新一行
* ~~r、R：~~

退出编辑模式：ESC

### 命令行模式

* `:wq`：w保存、q离开
  * 后面加 ！表示强制
* `ZZ`：文件保存离开
* `:w [filename] `： 另存新文件
* `:r [filename]`：加filename数据追加到光标后面
* `:! command`：执行command
* `:set nu`：显示行号
  `nonu`：取消行号

### 额外功能

#### 区块选择

* `v`：光标经过的地方反白选择
* `V`：列反白选择
* [Ctrl] + v：区块选择
* `y、d、p`：复制反白、删除反白、粘贴

#### 多文件编辑

vim命令可以同时打开多个文件`vim file1 file2 ...`

* `:n`：编辑下一个
* `:N`：编辑上一个
* `:files`：列出目前vim打开的文件

#### 多窗口

* `:sp` [filename]
* [ctrl] + w + j/k ：光标移动到上、下窗口

### 环境参数设置

* `:set all` ：显示目前所有环境参数设置值
* `:set nu/nonu`
* ...
* 配置文件：`/etc/vimrc`



### 常用命令

![](.\Linux学习.assets\image-20210825211125473.png)



### 换行字符

* `dos2unix [-kn] file [newfile]` 
* `unix2dos [-kn] file [newfile] `
  * -k：保留mtime时间格式
  * -n：保留旧文件
* vim编辑器内：
  * :set ff=dos/unix



## BASH

* shells：/bin/sh、/bin/bash、/bin/tcsh、/bin/csh

* ~/.bash_history 记录前一次登录以前所执行的指令

* alias 命令别名设置

  `alias lm ='ls -al'`

* `/etc/passwd`会设置一个给登录用户的shell（一般为bash）



* `type [-tpa] name`：查询命令是否为Bash shell内置

  * 不加参数，显示是外部指令还是内部指令
* -t：显示命令为file（外部命令）、alias（别名）还是builtin（bash内置）
  * -p：显示完整文件名
  * -a：PATH中，所有含name命令都列出，包含alias



* 组合键
  * [ctrl] +u/k：光标向前或向后删除命令串
  * ...

### shell 变量

* 环境变量：`PATH、HOME、MAIL、SHELL`
* `echo $var` 或 `echo ${var}`
* 变量设置规则：
  * `var=xxx` （等号两边不能有空格）
  * 变量名开头不能是数字
  * 双引号内特殊字符如$等，会保留原有的特性，单引号则只是单纯的字符
* 转义字符 `\`
* 命令的结果可以赋值给变量
  var=`command`或 `var=$(command)`
* `export var ` ：使变量转成环境变量，可以被子程序所引用（如makefile）
  * 不加变量，则是显示所有环境变量
* `unset var`：取消变量的设置
* `env`：查看所有环境变量
  * ...
  * `RANDOM`：随机数 0~32767
    `declare -i number=$RANDOM*10/32768 ; echo $number  `：0~9
    `declare` 宣告数值类型
* `set`：查看所有变量
  * `PS1`：提示字符的设置
  * `$`：shell的PID
  * `?`：上个执行命令的返回值，成功为0，失败为非0错误码
  * `OSTYPE、HOSTTYPE、MACHTYPE`：主机硬件与核心等级
* `locale`：语言环境
  * 配置文件：`/etc/locale.conf`

#### read array declare

* `read [-pt] var`：读取键盘输入
  * -p：后接提示字符
  * -t：等待的秒数，不用该参数则会一致等待
* `declare/typeset [-aixr] var`：宣告变量类型
  * -a：var设置成array类型
  * -i：设置成整数
  * -x：同export，设置成环境变量
  * -r：设置成readonly，不能更改也不能unset

* array：`var[index]=xxx`
  * 取array的值：`${var [index]}`，不加括号会取错

#### 文件系统和程序的限制 ulimit

* `ulimit [-SHacdfltu]  [配额]`
  * -a：后面不接参数，列出所有限制额度
  * -H：严格设置，必定不能超过该值
  * -S：警告设置，超过则有警告
  * -c：core文件的最大容量
  * -f：可以创建的最大文件大小，单位KB
  * -d：data seg进程最大的数据段大小，单位KB
  * -l：锁定进程的最大数
  * -t：CPU最大使用时间，单位秒
  * -u：单一用户可使用的最大程序
  * unlimited 和 0 都代表无限制

#### 变量内容删除和替换

* 删除：`#` `##` `%` `%%`
  `${variable#关键字}  `：删除variable中符合关键字内容
  * `#`：符合关键字最短的那一个，向后删除
  * `##`：符合关键字最长的那一个，向后删除
  *  `%` `%%` ：向前删除
* 替换： `/` `//`
  * `${variable/旧字符串/新字符串}`：替换第一个
  * `${variable//旧字符串//新字符串}`：替换全部
* 变量的设置

| 变量设置方式     | str 没有设置       | str 为空字串       | str 已设置非为空字串 |
| ---------------- | ------------------ | ------------------ | -------------------- |
| var=${str-expr}  | var=expr           | var=               | var=$str             |
| var=${str:-expr} | var=expr           | var=expr           | var=$str             |
| var=${str+expr}  | var=               | var=expr           | var=expr             |
| var=${str:+expr} | var=               | var=               | var=expr             |
| var=${str=expr}  | str=expr var=expr  | str 不变 var=      | str 不变 var=$str    |
| var=${str:=expr} | str=expr var=expr  | str=expr var=expr  | str 不变 var=$str    |
| var=${str?expr}  | expr 输出至 stderr | var=               | var=$str             |
| var=${str:?expr} | expr 输出至 stderr | expr 输出至 stderr | var=$str             |



### 命令别名和历史命令

* `alias name='[command]'  `
  alias 后面不接任何东西，则是查看目前有哪些别名
* `unalia name`：取消别名

* `history [n]`：列出最近n笔
  `history [-c]`：所有 history 清除
  `history [-raw] histfiles`
  * -r：将 histfiles 内容读到当前 shell 的 history
  * -a：将当前新增的 history 命令新加入 histfiles 中
  * -w：将目前的 history 写入 histfiles，默认 histfiles 为 `~/.bash_history`
  * HISTFILESIZE：最多记录几笔
  * `!number` ：执行第number笔命令
  * `!!`：执行上一条命令
  * `!command`：执行最近以command开头的指令

### bash shell 操作环境

#### 登录提示和欢迎信息

* `/etc/issue`：登录提示
  * \d   本地端时间的日期；
  *  \l   显示第几个终端机接口； 
  * \m  硬件的等级 （i386/i486/.../x86_64）
  * \n 显示主机的网络名称； 
  * \O  显示 domain name；
  * \r   OS 版本 （相当于 uname -r）
  * \t    显示本地端时间的时间； 
  * \S   OS名称； 
  * \v   OS版本。
* `/etc/motd`：登录后的欢迎信息



#### bash 环境配置文件  

* `/etc/profile`：整体环境配置文件

  * `PATH`：会依据 UID 决定 PATH 变量要不要含有 sbin 的系统指令目录
  * `MAIL、USER、HOSTNAME、HISTSIZE、umask`

  调用外部设置数据

  * `/etc/profile.d/*.sh`
  * `/etc/locale.conf`
  * `/usr/share/bash-completion/completions/*  `：补全

* `~/.bash_profile、~/.bash_login、~/.profile`：个人环境配置文件

* `source 配置文件名` 等同于 `. 配置文件名`：读入环境配置文件
* `~/.bashrc`
  * 配置文件：`/etc/bashrc`
    依据不同的 UID 规范出 umask 的值；
    依据不同的 UID 规范出提示字符 （就是 PS1 变量）；
    调用 /etc/profile.d/*.sh 的设置  

其它配置文件：

* `/etc/man_db.conf`：man page数据库
* `~/.bash_logout`：登出bash后，系统执行完该文件的动作才离开

#### 终端机环境设置 stty，set

* `stty [-a]`：列出所有stty参数
  * `^`代表`[ctrl]`
  * intr：送出interrupt （中断） 的讯号给程序
  * quit : 送出 quit 的讯号给程序
  * ......
  * kill : 删除在目前命令行上的所有文字
  * eof : End of file 的意思，代表“结束输入”  
  * **start : 在某个程序停止后，重新启动他的 output  ^q**
  * **stop : 停止目前屏幕的输出 ^s**
* `stty erase ^h`：修改erase为[ctrl]+h
* `set [-uvCHhmBx]`
  * -u：为定义变量时，显示错误讯息，默认不启动
  * -x：命令执行前，会显示命令内容和++符号（调试），默认不启动
  * ......

#### wildcard 和 特殊字符

* `*`：0到无穷多个任意字符
* `?`：一定有一个任意字符
* `[]`：一定有一个在括号内的某个字符
  * `[abcd]`：一定有a/b/c/d中的一个
  * `[0-9]`：一定有0-9之间的一个
  * `[^abc]`：一定有一个非a/b/c的字符

```bash
#  \  |  ;  ~  $  &  !  /  >  >>  <  <<  ''  ""  ``  ()  {}
```

* &：后台工作
* \`command\`同 `$(command)`：先执行的命令

### 数据流重定向

![image-20210826122504384](.\Linux学习.assets\image-20210826122504384.png)

* 默认输出到屏幕  
* 标准输入 （stdin） ：代码为 0 ，使用 < 或 << ；
* 标准输出 （stdout）：代码为 1 ，使用 > 或 >> ；
* 标准错误输出（stderr）：代码为 2 ，使用 2> 或 2>>  
* \> ：覆盖、\>> ：追加 



* `/dev/null`：黑洞设备
* `> file 2>&1`：标准输出和错误输出都写入到file



### ;  &&  ||

* `cmd1; cmd2`：不考虑命令相关性连续下达命令
* `cmd1 && cmd2`：cmd1 正确执行，才会开始执行cmd2
* `cmd1 || cmd2`： cmd1错误执行，才会开始执行cmd2



### pipe

#### |

* `cmd1 | cmd2`：cmd1的输出，作为cmd2的输入
  * 管线命令仅会处理 stdout，对stderr会忽略
  * 管线后面的命令必须要能够接受 stdin

#### cut  grep 

* `cut -d'分隔字符' -f fields `
  `echo ${PATH} | cut -d ':' -f 3,5`：以冒号为分隔符，取第三与第五段
* `cut -c 字符区间  `
  `cut -c 12-20`：取第 12-20 的字符
  `cut -c 12-`：取第 12-以后的字符



* `grep [-acinv] [--color=auto] '搜寻字串' filename`
  * -a ：将 binary 文件以 text 文件的方式搜寻数据  
  * -c ：计算次数  
  * -i ：忽略大小写 
  * -n ：顺便输出行号  
  * -v ：反向选择，亦即显示出没有 '搜寻字串' 内容的那一行  
  * --color=auto ：关键字显示颜色，默认  

#### sort  wc  uniq

* `sort [-fbMnrtuk] [file or stdin]`：排序
  * -f ：忽略大小写
  * -b ：忽略最前面的空白字符
  * -M ：以月份的名字来排序
  * -n ：使用“纯数字”进行排序
  * -r ：反向排序
  * -u ：uniq ，相同的数据中，仅出现一行
  * -t ：分隔符号，默认是 [tab] 
  * -k ：以区间某字段（field） 来进行排序的意思，跟 -t 配合 
* `uniq [ic]`：重复数据仅列出一行
  * -c：计数
* `wc [-lwm]`：计算字、行、字符数

#### tee

* `tee [-a] file`：同时将数据流分送到文件和屏幕
  * -a：累加

#### 字符转换命令

* `tr [-ds] SET1 ...`：删除文字或替换文字
  * -d：删除 SET1 这个字串
  * -s：替换，默认
* `col [-xb]`：-x将 [tab]转成对应的空格
* `join [-ti12] file1 file2`：两个文件中，有**相同数据**的那一行，加在一起
  * -t：分割字符，默认是空格
  * `join -t ':' /etc/passwd /etc/shadow | head -n 3`
* `paste [-d] file1 file2`：直接按行数贴在一起，且以[tab]隔开
  * -d：接分隔符，默认是 [tab]
* `expand [-t num] file`：[tab]用num个空格取代

#### split 分割

* `split [-bl] file PREFIX`

  * -b：后面接分割成的文件大小，单位 b、k、m 等
  * -l：以行数分割
  * PREFIX：前置字符

  ```shell
  split -b 300k /etc/services services	#分割
  cat services* >> servicesback			#合并
  ```

#### ==xargs== 

#### 减号和管线配合

* `-` 可以代表 `stdin` 和 `stdout`
  `tar -cvf - /home | tar -xvf - -C /tmp/homeback  `
  将/home里的文件打包，传送到 `-` stdout；经过管线，后面解包 `-` stdin 到/tmp/homeback









---

wtmp utmp

blkid

lsblk

fuser

ps

bc 计算器

crontab

/etc/profile