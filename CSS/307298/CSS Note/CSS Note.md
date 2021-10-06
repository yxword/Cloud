

### storage overview V1 [2013.12.21]

##### 存储技术沙盘
![image-20210719141005064](.\云存储文档学习.assets\image-20210719141005064.png)

##### 存储硬件架构
<img src=".\云存储文档学习.assets\image-20210719092901162.png" alt="image-20210719092901162" style="zoom: 67%;" />

##### **存储相关概念**

* [HBA （Host Bus Adapter 主机总线适配器）](https://blog.csdn.net/weixin_34242819/article/details/92617656)，是一个在服务器和存储装置间提供输入/输出(I/O)处理和物理连接的电路板或集成电路适配器；因为HBA**减轻了主处理器在数据存储和检索任务的负担**，它能够**提高服务器的性能**。一个HBA和与之相连的磁盘子系统有时一起被称作一个磁盘通道；**HBA是服务器内部的I/O通道与存储系统的I/O通道之间的物理连接**。最常用的服务器内部I/O通道是`PCI`和`Sbus`，它们是连接服务器CPU和外围设备的通讯协议.

* [RAID （ Redundant Array of Independent Disks ）](https://zhuanlan.zhihu.com/p/51170719)即独立磁盘冗余阵列，简称为「磁盘阵列」，其实就是用多个独立的磁盘组成在一起形成一个大的磁盘系统，从而实现比单块磁盘更好的存储性能和更高的可靠性.

  * RAID0，简单，将多块磁盘组合形成大容量存储。写数据时，将数据分为N份，以独立的方式实现N块磁盘的读写，这N份数据会同时并发的写到磁盘中，执行**性能非常高**，读写性能理论上是单块磁盘的N倍；五数据校验和冗余备份，**可靠性差**。
  * RAID1，**成本最高**，写数据时将同一份数据无差别写两份分别到工作磁盘和镜像磁盘，空间使用率50%，即两盘当一盘用，数显冗余备份，**可靠性强，但性能差**。
  * RAID5，使用**最多**，**兼顾存储性能、数据安全、存储成本**，是对RAID3的改进（N块盘，N-1块采用RAID0，第N快写**奇偶校验码**，一块盘坏掉，用N-1块恢复数据，第N块盘读写频繁，容易损坏），**数据和对应的校验码分开存储在不同的磁盘**上。至少需要三块组成磁盘阵列，允许最多同时坏一盘。
    ==（RAID5校验位算法原理：P = D1 xor D2 xor D3 … xor Dn （D1,D2,D3 … Dn为数据块，P为校验，xor为异或运算）==
  * RAID6，使用**双重校验**允许两块磁盘同时损坏，也能保证数据恢复，每块磁盘除了有同级数据XOR校验区外，还有针对每个数据块的XOR校验区，数据**冗余性更高**，但**复杂度变高**；**读取效率比较高，写数据性能差**。
  * RAID10，RAID0和RAID1的合体，兼备RAID1和RAID0的优点，有一半磁盘空间冗余，浪费。

  ![img](.\云存储文档学习.assets\v2-5cfded757c5569ebed65829e4d5f1c80_720w.jpg)

* HBA卡基于IOC，直连卡；RAID卡基于ROC带缓存

* GE/10GE  万兆以太网接口
* SAS（Serial Attached SCSI 串行SCSI） 比SCSI接口小很多，点对点序列式传输，速率可达3Gbps

* 两种存储基本架构

  * [SAN](https://wiki.dzsc.com/3403.html)，即存储区域网络。它是一种通过光纤集线器、光纤路由器、光纤交换机等连接设备将磁盘阵列、磁带等存储设备与相关服务器连接起来的高速专用子网。

    * 组成构成：接口（SCSI、光纤通道等）、连接设备（交换设备、网关、路由器等）、通信控制协议（IP和SCSI等）和存储设备
    * 提供专用、高可靠性的基于光通道的存储网络
    * 基本功能
      * RAID将N块物理硬盘虚拟化为1块大逻辑盘
      * 卷管理再将这个大逻辑盘拆分为M块小逻辑盘
      * 小逻辑盘再被映射给各个Host
    * 产品：IBM DS8000、EMC VMAX、HP 3PAR

  * [NAS](https://baike.baidu.com/item/NAS/3465615)，网络附属存储，连接在网络上，具备资料存储功能的装置，也称“网络存储器”。

    * 专用数据存储服务器，以数据为中心，分离存储设备和服务器，集中管理数据，从而释放带宽、提高性能、降低成本等。
    * 基本功能
      * RAID将N块物理硬盘虚拟化为1块大逻辑盘
      * 卷管理再将这个大逻辑盘拆分为M块小逻辑盘
      * 通过文件系统组织LUN， 然后再将文件系统export出去。
    * 产品：IBM SoNAS、EMC Isilon、Panasas

  * [两者区别](https://zhuanlan.zhihu.com/p/50897964)
  * SAN存储设备通过光纤连接，而NAS存储设备通过TCP/IP连接
    
  * SAN存储设备访问数据块，而NAS存储设备访问单个文件
    
  * SAN存储设备连接多个存储设备，而NAS存储设备作为单个专用设备运行
    
  * 选择：
    
    * SAN更快，特别是在传输大量数据时，具有可扩展性和可扩展性，在非常大的工作负载下也能保持其性能；
      * NAS存储解决方案通常更快、更便宜、更容易，频繁地用于个人用户、小企业主和更直接的数据应用程序。
      * 对于大多数个人和组织来说，NAS解决方案是更合适的；然而，对于高级性能，Cloud SAN通常提供出色的性能，而不需要进行不必要的维护和监视
  * 目前市场上也已相继出现采用**对象存储技术**的 统一存储。 它结合了SAN和NAS双重优点，既可以以块形式(SAN侧重块)访问，又可以以文件形式(NAS侧重文件系统)访问

* [Scale Up 和 Scale Out](https://blog.csdn.net/truong/article/details/73056934)

  * Scale Up 纵向扩展，利用现有的存储系统，通过不断增加存储容量来满足数据增长的需求。但这种方式只增加容量，带宽和计算能力并没有相应的增加。
  * Scale Out 横向扩展架构的升级通常是以**节点**为单位，每个节点往往将包含**容量、处理能力和I / O带宽**。一个节点被添加到存储系统，系统中的三种资源将同时升级。而且扩展之后从用户的视角看起来仍然是一个单一的系统。scale out方式使得存储系统升级工作大大简化，用户能够真正实现**按需**购买。
  
* SCSI  [硬盘基础知识 HDD、SSD、IDE、PATA、SATA、SCSI、SAS、FC](https://www.cnblogs.com/xiaodoujiaohome/p/11732687.html)

  * 硬盘接口类型：有IDE=ATA=PATA<SATA, SCSI<SAS,FC
  * 硬盘材质类型：分为机械和固态:HDD SSD
  * 硬盘的组合方式：单块或者raid:raid用raid控制器代替磁盘控制器
  * 硬盘机柜网络：有FC网络的和普通网线的网络
  * 硬盘+服务器的存储架构：DAS=server+raid SAN=server--raid存储器  NAS=server+rais存储器+文件系统
  * 厂家：希捷Seagate、西数Western Digital、日立Hitachi

* 硬盘性能
  提升系统性能的关键，在于如何减小磁头的寻道时间和增加硬盘的并发度：

  * IO尽量连续
  * 通常 1MB 的IO大小为最佳性能
  * 尽量使所有硬盘工作起来

* RAID

  * 软RAID：通用CPU + IOC(HBA)，RAID通过纯软件实现
  * 硬RAID：RoC（RAID on Chip）通过纯硬件（FPGA或ASIC）实现，或者RAID通过RoC上的通用CPU结合XOR加速引擎实现

* ==块设备虚拟化==？？

  * 通过块设备虚拟化技术，可大幅提升存储性能，尤其是随机读写的并发性，同时大大降低了硬盘重构时间。
  * 风险： 一定程度上降低了系统可靠性。  需通过更高冗余度的可靠性算法来提升系统整体可靠性。

* ==高可靠性算法==？？

  * ==RAID6==
    * ==EVENODD 奇偶码  二进制1的个数是奇数还是偶数==
    * ==RDP==
    * ==Reed-Solomon Code(RS Code) / Cauchy Reed-Solomon Code(CRS Code)==
  * ==Erasure Code 纠删码==
    * ==Reed-Solomon Code(RS Code) / Cauchy Reed-Solomon Code(CRS Code)==
    * ==Local Reconstruction Code (LRC)  局部校验码==
    * ==Grid-Code..==
  * ==结合高可靠性的算法来提高系统性能和可靠性，是现在研究的主要热点。==
  * ==通过一个伽罗德华域矩阵 乘以 源数据 得出 校验数据。缺点： 乘法的效率低。== 
    ==改进方法：==
    * ==通过芯片来提速， 即P+Q引擎==
    * ==将乘法转化为位图矩阵，异或得以实现。==

* **卷管理**
  ![image-20210719152428500](.\云存储文档学习.assets\image-20210719152428500.png)

  * 适用场景

    * 通过允许用户添加、更换磁盘，无需停机即可在磁盘间复制、共享内容来管理大容量磁盘设备
    * 小型系统上，用户可以通过LVM轻松地重新划分磁盘分区大小
    * 通过“快照”实现备份；
    * 可为多个物理卷或整个磁盘创建单一逻辑分区（某种程度类似于RAID 0，更与JBOD相似），允许动态更改卷容量。

  * **[LVM（Logic Volue Manager）](https://www.cnblogs.com/gaojun/archive/2012/08/22/2650229.html)**

    * LVM将一个或多个硬盘的分区在逻辑上集合，相当于一个大硬盘来使用，当硬盘的空间不够使用的时候，可以继续将其它的硬盘的分区加入其中，这样可以实现磁盘空间的动态管理

    * LVM为计算机提供了更高层次的磁盘存储，使系统管理员可以更方便的为应用与用户分配存储空间，可以按需要随时改变大小与移除

    * 允许按用户组对存储卷进行管理，允许管理员用更直观的名称(如"sales'、 'development')代替物理磁盘名(如'sda'、'sdb')来标识存储卷。

    * LVM术语

      * 物理存储介质（The physical media）：这里指系统的存储设备：硬盘，如：/dev/hda1、/dev/sda等等，是存储系统最低层的存储单元。

      * 物理卷（physical volume）：物理卷就是指硬盘分区或从逻辑上与磁盘分区具有同样功能的设备(如RAID)，是LVM的基本存储逻辑块，但和物理存储介质（如分区、磁盘等）比较，却包含有与LVM相关的管理参数。

      * 卷组（Volume Group）：LVM卷组类似于非LVM系统中的物理硬盘，其由物理卷组成。可以在卷组上创建一个或多个“LVM分区”（逻辑卷），LVM卷组由一个或多个物理卷组成。

      * 逻辑卷（logical volume）：LVM的逻辑卷类似于非LVM系统中的硬盘分区，在逻辑卷之上可以建立文件系统(比如/home或者/usr等)。

      * PE（physical extent）：每一个物理卷被划分为称为PE(Physical Extents)的基本单元，具有唯一编号的PE是可以被LVM寻址的最小单元。PE的大小是可配置的，默认为4MB。

      * LE（logical extent）：逻辑卷也被划分为被称为LE(Logical Extents) 的可被寻址的基本单位。在同一个卷组中，LE的大小和PE是相同的，并且一一对应。

      * 总结：PV是物理的磁盘分区；VG是LVM中的PV，也就是PV必须加入VG，可以将VG理解为一个仓库或者是几个大的硬盘；LV是从VG中划分的逻辑分区。

        <img src=".\云存储文档学习.assets\image-20210719154227493.png" alt="image-20210719154227493" style="zoom:67%;" />

      * 创建LVM系统步骤：

        * 创建分区(fdisk) fdisk /dev/hdx，指定分区格式为LVM分区格式 8e 
        * 创建完执行partprobe使其生效
        * 创建PV  pvcreate，pvdisplay查看
        * 创建VG  vgcreate，vgdisplay查看
        * 创建LV  lvcreate，lvdisplay查看
        * LV格式化及挂载
          - mkfs格式化
          - mount挂载
          - 将格式化后的LV分区挂载到指定的目录下，就可以像普通目录一样存储数据了；挂载之后，df -T 可以看到此LV的容量；
          - 如果要在系统启动的时候启动LV，最好是将lvData写入fstable 文件中，使用Vim，打开/etc/fstab，根据fstable表添加，LV、挂载点、文件格式

* ==缓冲算法==

  * LRU：丢弃近段时间内最少被使用的数据
  * ARC（Adjustable Replacement Cache）/ SARC：自适应替换算法，解决在随机和连续之间转换
  * AMP：管理连续列表的内容，为连续负载达到最大化吞吐能力
  * IWC：主要改善写cache。对于随机写cache提升幅度明显

* 文件系统：操作系统用于明确磁盘或分区上的文件的方法和数据结构，即磁盘上组织文件的方法

  * 常见文件系统：FAT、NTFS、ext3、XFS（XFS比ext3快）
    XFS设计思想只有一个，“考虑大东西”。XFS也是一种日志文件系统
    * 分配组与可伸缩性
    * B+二叉树
    * 延迟分配
    * 日志记录
  * **分布式文件系统：NFS、GFS、HDFS、Gluster、Ceph**
    * NFS: 老牌文件系统，网络文件系统
    * GFS: Google File System， 主要应用为海量搜索业务场景
    * HDFS: Hadoop Distributed File System
      - 高吞吐，高容错。主要适用于大数据的1次写，多次读的场景
    * Gluster: 采用Key-Value方式的弹性哈希算法来直接避免元数据。主要应用于云存储
    * CePH: linux PB级文件系统。 应用对象存储，数据和控制分离。 主要应用场景为云存储

* ==IP SAN 的 IO 流程==

  * 主机发送SCSI读请求
  * GE/10GE将TCP/IP包解析后，将iSCSI包给iSCSI target模块
  * iSCSI Target模块解析出SCSI CDB(SCSI命令描述块，指定需要target处理的具体操作),并确定LUN号(逻辑单元号)
  * 逻辑卷检查是否有cache 命中，没有则计算出所要操作的IO地址为哪个RAID，并将IO下发到RAID
  * RAID根据RAID类型，LBA(Logical Block Address 逻辑区块地址)地址计算出是哪个硬盘
  * 硬盘驱动再将IO请求转为SCSI CDB
  * SAS、HBA将IO 送给SAS Expander(扩展器，在SAS中类似路由器/交换机功能)
  * Expander 直接将命令送给硬盘

  双控：Cache需要镜像，并需要处理一致性问题；逻辑卷和RAID，两个控制器需处理一致性问题。

  ![image-20210719162224854](.\云存储文档学习.assets\image-20210719162224854.png)

  <img src=".\云存储文档学习.assets\image-20210719162252192.png" alt="image-20210719162252192" style="zoom:67%;" />

* ==IP NAS 的 IO流程==

  - 主机发送文件读请求
  - GE/10GE将TCP/IP包解析后给NFS。NFS解析文件的元数据，并确定文件所在位置
  - 然后操作本地文件系统，读取本地文件
  - 本地文件系统先转到VFS（Virtual File System）,  确定具体文件系统
  - XFS根据文件操作==确定逻辑卷==
  - 逻辑卷检查是否有cache 命中，没有则计算出所要操作的IO地址为哪个RAID，并将IO下发到RAID
  - RAID根据RAID类型，LBA地址计算出是哪个硬盘
  - 硬盘驱动再将IO请求转为SCSI CDB
  - SAS HBA将IO 送给SAS Expander
  - Expander 直接将命令送给硬盘

  双控：大量的文件操作一致性问题；Cache镜像一致性；逻辑卷和RAID的一致性问题。

  ![image-20210719162334636](.\云存储文档学习.assets\image-20210719162334636.png)

  

  <img src=".\云存储文档学习.assets\image-20210719162322249.png" alt="image-20210719162322249" style="zoom:67%;" />

##### 存储技术趋势

  * 背景：大数据时代，数据量每两年翻一番
  * 关注点：可靠性、性能、容量、成本、安全
  * ==热点技术==
    SSD相关：ECC，磨损均衡，Flash Cache, 分级存储， SSD文件系统，其他新型存储介质
    云存储相关：虚拟化，分布式文件系统，SSD元数据加速，纠删码，基于地理的全球数据中心，对象存储，容灾备份，高速互连。

### 大数据研究院介绍

##### 整体架构

- PaaS  大数据、云识别

- IaaS  云计算、云存储，服务器、存储、网络

- Apps  DSS行业平台、乐橙、第三方公司依赖于上述架构

##### **开发流程**

<img src=".\云存储文档学习.assets\image-20210716175447527.png" alt="image-20210716175447527" style="zoom: 67%;" />

- 明确需求规格和架构设计、敏捷开发迭代版本、注重每个阶段交付件
- 重要交付件
  + 需求包： 主要功能、use case
  + 架构概要设计：不仅仅是功能
  + 迭代：迭代story拆解、详细设计和迭代总结
  + 过程分析： 项目详细记录人力、问题、特性实现情况
- 开发活动
  + 迭代特性分析
    * 项目经理需在迭代开始前必须明确版本的迭代特性需求，系分/PL需在迭代开始前完成工作量评估
  + 迭代开始后，针对story进行详细的串讲反串讲，形成详细设计再进入开发
  + 结对编程， 提升代码的可读性和健壮性
  + 代码评审， 人人都是质量经理
  + 迭代总结， 发现教训提升能力，发现优势固化能力
  + 培训传承， 详细专业的技术文档

##### **善于总结分享**

- 项目经理要对项目的每个迭代都作总结，明确经验和可改进点，提出计划
- 开发要对攻克的难点，做文档总结，并分享给所有人
- 鼓励实战性的分享，每次分享的内容可以少，但是必须实用，并且鼓励习惯性分享
- 鼓励不仅仅是成功经验分享，更加鼓励分享走过的弯路
- 文档规范，**redmine**详细记录任务的处理过程，明确的**SVN上库关联**



---



### 云存储培训[2013.12.28]

+ 行业趋势 ：高清化、智能化(ITC IVS)、网络化(IPC NVR)、相辅相成
+ 挑战 ：海量存储需求、视频数据分析需求、海量结构化数据分析需求、事后事中事前监控需求
+ 监控云组成 ：存储云、分析计算云、分布式数据库、云管理平台、业务云、监控中心、前端采集设备
+ 传统存储（DAS、SAN、NAS）局限性 ： 
  + 存储空间有限；难以形成聚合IO[^1]和负载均衡、无法统一命名空间、应用复杂；
  + 仅支持单系统内数据冗余、存储可用性无法保证（？）；
  + 无法动态在线扩容，不灵活

<img src=".\云存储文档学习.assets\image-20210708141546306.png" alt="image-20210708141546306" style="zoom: 80%;" />

- DSS数字监控系统存储现状 ：
  + SS+ESS 静态组成一个SAN存储系统，SS和ESS绑定，难以适应业务的灵活性；
  + SS间存储空间不共享，负载难以均衡；
  + 无法动态、在线扩容以及实现系统级的数据冗余[^2]，无法保证存储的高可用性[^3]。

<img src=".\云存储文档学习.assets\image-20210708150016072.png" alt="image-20210708150016072" style="zoom:67%;" />



+ 云存储优点 ：
  + 统一管理存储，全局负载均衡，最大化利用集群IO能力和存储能力；
  + 存储和业务无耦合，弹性、在线扩容，ScaleOut[^4]线性扩展，==支持业务灵活扩展，业务调整不会带来拓扑结构复杂化（？）；==
  + 更高级别的存储可用性，数据可读和系统可写的可靠性；
  + 数据可共享，具备NAS优点[^5]，具备SAN高性能优点。

<img src=".\云存储文档学习.assets\image-20210708152323095.png" alt="image-20210708152323095" style="zoom:67%;" />



+ DSS结合云存储架构 ：

  - SS和云存储间以网络连接，无设备静态耦合关系，拓扑清晰；

  + 拆分为SS集群和存储集群，各自负载均衡、在线扩展，隔离ESS、SS故障对整体系统的冲击，更容易达成整体系统最优；
  + SS充分利用云存储的数据共享能力、高可用性和数据高可靠性。

+ 视频监控存储业务特点：
  + 音视频作为主要存储对象，大文件并发数据、海量数据、稳定流式不中断写入；（有小文件但非主要内容）
  + 存储成本敏感（使用高存储密度设备，单设备存储能力巨大而存储IO能力、CPU计算能力有限）；
  + 存储数据循环覆盖（频繁创建删除文件，设备高存储量状态）；
  + 数据读写差异（流式写入，很少并发，读取概率相对较低）。一写多读、长期保存。

* 存储架构
  非对称云存储架构（智能存储服务器集群+元数据服务器集群）、对称云存储架构、 
  EFS架构（DN + MDS两台，实现HA高可用、数据分布逻辑......（？）

[^1]: 多个缓冲区写入单个数据流
[^2]: 备份便于数据恢复？分布式？
[^3]: 任一存储设备出现故障时，另一台存储阵列能够继续为前端主机提供完整的数据服务，故障转移切换自动完成无需人工干预，整个过程不会影响前端应用对后端数据的读写，实现数据零丢失，保证业务的连续性。-- 百度百科
[^4]: 横向扩展
[^5]: 高容量、高效能、高可靠，易安装、易维护，扩展性好等



---

### 公司业务介绍

| **常见缩写** | **英文**                                   | **中文**           |
| ------------ | ------------------------------------------ | ------------------ |
| DVR          | Digital Video Recorder                     | 数字硬盘录像机     |
| HDVR         | Hybrid Digital Video Recorder              | 混合数字硬盘录像机 |
| NVR          | Network Video Recorder                     | 网络录像机         |
| EVS          | Embedded(Ethernet) Video Server            | 网络视频服务器     |
| ESS          | Embedded (Ethernet) Storage Server         | 网络存储服务器     |
| M            | Matrix                                     | 视频矩阵           |
| IPC          | Internet Protocol Camera                   | 网络摄像机         |
| ITC          | Intelligent Traffic Camera                 | 智能交通相机       |
| SD           | Speed  Dome                                | (网络)球机         |
| ISP          | Image Signal Process                       | 图像信号处理       |
| HDCVI        | High Definition Composite Video  Interface | 高清复合视频接口   |

| **常见缩写** | **英文**                                            | **中文**           |
| ------------ | --------------------------------------------------- | ------------------ |
| DSS          | Digital  Surveillance  System                       | 数字监控系统       |
| CMS          | Center Management Server                            | 中心管理服务器     |
| MTS          | Media  Transfer Server                              | 流媒体服务器       |
| SS           | Storage Server                                      | 存储服务器         |
| DMS          | Device Management Server                            | 设备管理服务器     |
| PSS          | Professional Surveillance System                    | 专业监控系统       |
| ICP          | Intelligent Combat Platform                         | 视频实战图侦平台   |
| NMS          | Network Management System                           | 网络管理系统       |
| IVS          | Intelligent Video Server                            | 智能视频服务器     |
| HS           | Hua Shi                                             | 华视微讯设备或平台 |
| EFS          | ExaByte,Elastic,ErasureCoding,Extreme  File  System | 云文件系统         |
| CSS          | Cloud  Storage Server                               | 云存储服务器       |
| CCS          | Cloud  Computing Server                             | 云计算服务器       |
| CDB          | Cloud  Database                                     | 云数据库服务器     |



* 数据流和云存储方案...



### 云存储网络部署与交换机配置

##### 云存储网络拓扑

* DSS结合云存储架构
  ![image-20210719165731459](.\云存储文档学习.assets\image-20210719165731459.png)
  * SS集群和云存储之间以网络连接，无设备间静态耦合关系，拓扑非常清晰；
  * 系统逻辑上拆分为SS集群和存储集群，各自负载均衡、在线扩展等成为可能
  * SS可充分利用云存储的数据共享能力，以及高可用性和数据的高可靠性。
* 转存
  <img src=".\云存储文档学习.assets\image-20210719171656758.png" alt="image-20210719171656758" style="zoom:67%;" />
* 直存
  <img src=".\云存储文档学习.assets\image-20210719171718552.png" alt="image-20210719171718552" style="zoom:67%;" />
* 查看网络状况  运维平台
  * 概况 ---  H、B为绿色表示，心跳和业务为正常在线，红色离线
    ![image-20210719172014789](.\云存储文档学习.assets\image-20210719172014789.png)
  * 存储节点 -- 状态
  * 日志
    上线：`DNUid:2,ip:172.5.4.43,port:60010,register success`
    下线：`DNUid:2 is offline`

##### 主机网卡绑定

* linux其中网卡绑定模式：

  0. load balancing （round-robin）

  1. **fault-tolerance （active-backup）**
     主备模式，对应交换机端口不可绑定
  2. **load balancing （xor）**
     xor负载均衡模式，对应交换机端口可绑可不绑，但绑定的性能更高
  3. fault-tolerance（broadcast）
  4. lacp  动态链接聚合
  5. transmit load balancing
  6. adaptive load balancing

* 我们只使用1和2模式

  * MDS / CS：心跳网卡绑定使用模式1，业务网卡使用模式2

  * SS / DN：使用模式2

* MDS/CS 业务网卡

  * /etc/sysconfig/network-scripts/ifcfg-bond0

    ```shell
    DEVICE=bond0
    BOOTPROTO=static
    IPADDR=172.5.4.10
    NETMASK=255.255.0.0
    ONBOOT=yes
    TYPE=Ethernet
    GATEWAY=172.5.0.1
    BONDING_OPTS="miimon=100 mode=2 xmit_hash_policy=1“
    ```

  * /etc/sysconfig/network-scripts/ifcfg-eth0

    ```shell
    DEVICE=eth0
    BOOTPROTO=none
    ONBOOT=yes
    MASTER=bond0
    SLAVE=yes
    ```

  * /etc/sysconfig/network-scripts/ifcfg-eth1

    ```shell
    DEVICE=eth1
    BOOTPROTO=none
    ONBOOT=yes
    MASTER=bond0
    SLAVE=yes
    ```

  * /etc/sysconfig/network-scripts/ifcfg-bond1

    ```shell
    DEVICE=bond1
    BOOTPROTO=static
    IPADDR=192.168.1.10
    NETMASK=255.255.255.0
    ONBOOT=yes
    TYPE=Ethernet
    BONDING_OPTS="miimon=100 mode=1“
    ```

  * /etc/sysconfig/network-scripts/ifcfg-eth2

    ```shell
    DEVICE=eth2
    BOOTPROTO=none
    ONBOOT=yes
    MASTER=bond1
    SLAVE=yes
    ```

  * /etc/sysconfig/network-scripts/ifcfg-eth3

    ```shell
    DEVICE=eth3
    BOOTPROTO=none
    ONBOOT=yes
    MASTER=bond1
    SLAVE=yes
    ```

  * service network restart 重启网络

  * 查看绑定状态  cat /proc/net/bonding/bonx

##### 交换机配置

###### 登录（设置账户、IP）

* 方式：Console、Web、Telnet、SSH

* 空配的交换机没有IP，只能通过Console方式登录；需配置IP和账户并开启对应服务后，才能使用另外三种方式登录。

* Console登录：

  * 配置要点
    * 准备一个专门的串口线（一端是接电脑COM口的九针接口，一端是接交换机console口的普通网线接口）
    * 登录软件：SecureCRT或Xshell，无需用户名密码
  * 配置步骤
    1. Console线链接电脑COM口（设备管理器查看是哪个串口）和交换机console口
    2. SecureCRT新建快速链接，协议选择Serial，端口选择COM1，波特率设置9600，RTS/CTS不打勾（关闭流控功能）
    3. 连接，交换机出现主机名提示符表示成功连接
       无提示符，恢复方法......

* WEB登录：

  * 配置要点

    - 需开启web服务；
    -  需要给交换机配置一个管理IP，如果PC与交换机不是同一个网段，需要给交换机配置一个默认网关；
    - 需要配置一个登录的用户名和密码以及特权密码

  * 配置步骤

    1. 使用console登录交换机，开启web服务功能

       ```shell
       Ruijie>enable				------>进入特权模式
       Ruijie#configure			------>进入全局配置模式
       Ruijie(config)#enable service web-server 		------>开启ssh服务
       ```

    2. 配置交换机管理IP

       ```shell
       Ruijie(config)#interface vlan 1    	------>进入vlan 1接口
       Ruijie(config-if)#ip address 192.168.1.1 255.255.255.0	------>为vlan 1接口上设置管理ip
       Ruijie(config-if)#exit              ------>退回到全局配置模式
       Ruijie(config)#ip route 0.0.0.0 0.0.0.0 192.168.1.254   	------>设置默认网关
       ```

    3. 配置用户名和密码

       ```shell
       Ruijie(config)#username admin password admin   ------>配置远程登入的用户名为admin，密码为admin
       Ruijie(config)#enable password  admin                   ------>配置进入特权模式的密码为admin
       Ruijie(config)#username admin privilege 15           ------>配置web界面登入的用户名的等级权限为15，默认用户名和密码的等级权限为1，web管理需要15级的权限
       Ruijie(config)#ip http authentication local               ------>配置web登入使用用户名和密码
       Ruijie(config)#end      ------>退出到特权模式
       Ruijie#write            ------>确认配置正确，保存配置
       ```

    4. 保存配置

       ```shell
       Ruijie(config)#end     ------>退出到特权模式
       Ruijie#write           ------>确认配置正确，保存配置
       ```

    5. 使用管理ip以及用户名密码，登录web页面

* Telnet登录

  * 配置要点

    * 需要开启telnet服务
    * 需要给交换机配置一个管理IP，如果PC与交换机不是同一个网段，需要给交换机配置一个默认网关
    * 需要配置一个登录的用户名和密码以及特权密码

  * 配置步骤

    1. 使用console登录交换机，开启telnet服务功能

       ```shell
       Ruijie>enable			------>进入特权模式
       Ruijie#configure		------>进入全局配置模式
       Ruijie(config)#enable service telnet-server------>开启telnet服务
       ```

    2. 配置交换机管理IP（参考WEB）

    3. 配置用户名和密码

       ```shell
       Ruijie(config)#line vty 0 4      ------>进入telnet密码配置模式，0 4表示允许共5个用户同时telnet登入到交换机
       Ruijie(config-line)#login local  ------>启用telnet时使用本地用户和密码功能
       Ruijie(config-line)#exit         ------>回到全局配置模式
       Ruijie(config)#username admin password admin   ------>配置远程登入的用户名为admin，密码为admin
       Ruijie(config)#enable password  admin  ------>配置进入特权模式的密码为admin
       ```

    4. 保存配置（参考WEB）

  * 使用配置的ip和用户名密码，telnet登陆     `telnet 192.168.1.1`

* SSH登录

  * 配置要点

    * 需要开启ssh服务
    * 需要手工生成Key
    * 需要交换机配置一个管理IP，如果PC与交换机不是同一个网段，需要给交换机配置一个默认网关
    * 需要配置一个登录的用户名和密码以及特权密码

  * 配置步骤

    1. 使用console登录交换机，开机ssh服务功能

       ```shell
       Ruijie>enable			------>进入特权模式
       Ruijie#configure		------>进入全局配置模式
       Ruijie(config)#enable servicessh-server  	------>开启ssh服务
       ```

    2. 生成加密密钥

       ```shell
       Ruijie(config)#crypto key generate dsa    ------>加密方式有两种：DSA和RSA,可以随意选择
       Choose the size of the key modulus in the range of 360 to 2048 for your Signature Keys. Choosing a key modulus greater than 512 may take a few minutes.
       How many bits in the modulus [512]:      ------>直接敲回车
       % Generating 512 bit DSA keys ...[ok]
       ```

    3. 配置交换机管理IP，配置用户名和密码，保存配置（参考telnet）

  * 使用配置的ip和用户名密码，ssh登陆

###### 端口绑定

* 标准叫法：链路聚合（Link Aggregation）
  其他叫法：端口聚合、端口汇聚、Eth-Trunk
* 聚合端口：
  是由多个物理成员端口聚合而成，通过把多个物理链接聚合在一起形成一个简单的逻辑链接，从而实现增加链路带宽的目的。这个逻辑链接我们称之为一个聚合端口。
* [为什么要链路聚合](https://blog.csdn.net/weixin_39868248/article/details/111861531)：增加带宽、冗余备份、负载分担
* 锐捷，华为，华三的链路聚合配置

###### 级联

* 作用：增加端口数量，延长连接距离
* 方法：使用网线连接普通网口
* 不同型号的交换机都可以级联，如果两台交换机之间使用多条网线级联，必须进行端口绑定
  <img src=".\云存储文档学习.assets\image-20210720095631733.png" alt="image-20210720095631733" style="zoom:67%;" />
* 错误连法（形成环路）
  <img src=".\云存储文档学习.assets\image-20210720095746435.png" alt="image-20210720095746435" style="zoom: 67%;" />

###### 堆叠

![image-20210720100125709](.\云存储文档学习.assets\image-20210720100125709.png)

* 堆叠：将多台设备连接在一起，**虚拟化**成一台设备

* 作用：简化管理、增加端口数量、提高链路冗余

* 方法：使用专用堆叠模块和线缆、使用万兆模块和光纤

* 相同品牌的交换机才可以堆叠，各品牌有各自特有的方式

  * 华为：iStack（Intelligent Stack，智能堆叠）
  * 华三：IRF （Intelligent Resilient Framework，智能弹性架构）
  * 锐捷：VSU（Virtual Switching Unit，虚拟交换单元）

* 成员角色
  堆叠中所有的单台交换机都称为成员交换机，都对外提供相同的服务。按照功能不同，可以分为三种角色：
  ![image-20210720100632871](.\云存储文档学习.assets\image-20210720100632871.png)

  * 主交换机：**Master **负责管理整个堆叠。堆叠中只有**一台**主交换机
  * 备交换机：**Standby **是主交换机的备份交换机。当主交换机故障时，备交换机会接替原主交换机的所有业务。堆叠中只有**一台**备交换机。
  * 从交换机：**Slave **主要用于业务转发，从交换机数量越多，堆叠系统的转发能力越强。除主交换机和备交换机外，堆叠中其他所有的成员交换机都是从交换机。

* 华为堆叠：iStack（Intelligent Stack，智能堆叠）
  <img src=".\云存储文档学习.assets\image-20210720100918287.png" alt="image-20210720100918287" style="zoom: 67%;" />
  ![image-20210720100939073](.\云存储文档学习.assets\image-20210720100939073.png)
  ![image-20210720100951325](.\云存储文档学习.assets\image-20210720100951325.png)
  无需配置，插上堆叠模块，连上堆叠线缆，重启后自动堆叠

* 华三堆叠：IRF（Intelligent Resilient Framework，智能弹性架构）
  ![image-20210720101103685](.\云存储文档学习.assets\image-20210720101103685.png)
  ![image-20210720101109939](.\云存储文档学习.assets\image-20210720101109939.png)

  * 配置：Web平台
    ...

* 锐捷堆叠：VSU（Virtual Switching Unit，虚拟交换单元）

  ![image-20210720101308273](.\云存储文档学习.assets\image-20210720101308273.png)

  * 配置：后台配置
    ...

* 交换机级联和堆叠
  如果主机的多张网卡进行了绑定，可以分插在堆叠的多台交换机上，但不能分插在级联或相互独立的多台交换机上！

  ![image-20210720101815243](.\云存储文档学习.assets\image-20210720101815243.png)

###### 升级（略）

##### 网络性能测试

* 工具：iperf
  * 例子
    * 172.5.5.151作为服务端运用ipfer： `iperf -s –i 1`
    * 172.5.5.152作为客户端运行iperf： `iperf –t 6000 –i 1 –c 172.5.5.151 –P 32`
  * 参数说明
    * -s 以服务端方式运行  |  -c 以客户端方式运行（参数为需要连接的服务端IP地址）
    * -i 报告的时间间隔（秒）|  -t 需要运行的时间（秒）|  -P 线程数
* 针对云存储特性进行测试
  * 云存储业务的主要读写是在客户端和多台DataNode之间进行，以3+1模式为例，则是1台客户端和4台DataNode之间传输数据，所以测试时要以一打多、多打一和多打多的模式进行。
  * 比如我们拿5台主机进行测试，每台四网卡绑定。假设这5台主机的IP地址为：`172.5.4.31、172.5.4.32、172.5.4.33、172.5.4.34、172.5.4.35`。
  * 单向多打一
    * 把31作为服务端：`iperf –s –i 1`
    * 把其他4台机器作为客户端：`iperf –t 6000 –i 1 –c 172.5.4.31 –P 32`
    * 这种情况31的接收速度接近4Gbps比较理想。
  * 单向一打多
    * 把31作为客户端，其他4台机器作为服务端。
    * 首先在4个服务端开启服务：`iperf –s –i 1`。
    * 然后在31上开启4个终端，每个终端都去连其中一个服务器端：
      `iperf –t 6000 –i 1 –c 172.5.4.32 –P 32
      iperf –t 6000 –i 1 –c 172.5.4.33 –P 32
      iperf –t 6000 –i 1 –c 172.5.4.34 –P 32
      iperf –t 6000 –i 1 –c 172.5.4.35 –P 32`
    * 这种情况31的发送速度接近4Gbps比较理想
  * 双向多打一
    * 这种方式是把以上两种单向方式组合在了一起，即31即作为客户端也作为服务端。作为客户端时去连其他每一个服务器，作为服务端时其他客户端都要连过来。
    * 这种情况31的发送和接收速度都接近4Gbps比较理想
  * 双向多打多
    * 每台即作为服务端，又作为客户端
    * 这种情况每台的发送和接收速度都接近4Gbps比较理想

##### 网络速度统计

* 统计工具：`sar`
  * 用法:   `sar [ 选项 ] [ <时间间隔> [ <次数> ] `
    例子：`sar -n DEV 1   # 统计网卡流量，1表示1秒汇报一次`
  * 统计脚本：`/opt/dss/Util/net_statistic.sh`    ==DSS系统独有==
  * 用法： `/opt/dss/Util/net_statistic.sh 网卡名称`
    例子： `/opt/dss/Util/net_statistic.sh eth0`

### EFS结构介绍[2015.3.28]

##### 背景

* 存储规模决定存储系统架构
  规模的发展带来的问题：数据使用困难、稳定性下降、管理复杂、成本增加
* 发展进程
  * 本地存储：DVR、NVS、NVR
  * 集中存储：NVR集群、iSCSI集群、管理服务器
  * 云存储：元数据节点、数据节点、网关服务器

##### 目标

* 具备云存储优势：高可靠性、高可用性、高性能、动态扩容、易维护
* 适用监控行业：成本要求高磁盘利用率，数据实时可读，设备存储容量、IO性能、网络性能受限，读写比率低、要求不一致，循环覆盖、高存储利用率
* 可控：自主研发

##### 物理集群

![image-20210720110510686](.\云存储文档学习.assets\image-20210720110510686.png)

##### 逻辑架构

<img src=".\云存储文档学习.assets\image-20210720112219517.png" alt="image-20210720112219517" style="zoom:80%;" />

##### 命名空间

* Bucket
  1-63字节长度字符串、全局唯一、权限控制最小粒度、最大10W个文件
* 文件
  一定属于某个bucket、Bucket内唯一、bucket总长不超过255字节、存在大小文件之分

##### IO流程

- EFSClient(文件层)向元数据管理中的CS进行 目录服务请求 (简要流程，具体看文件层写入和读取流程)
  + EFSClient 通过和 EOSClient 交互，得到数据存储对象(大文件时创建objectCollection、小文件时为container)
  + 写入数据成功后要释放通过EOSClient得到的数据存储对象
    * 释放完对象，需要更新信息到 CatalogServer 服务(管理节点上的服务)，更新时是要同时更新到主备两台(串行)
  + 读取文件时，向 CatalogServer 请求获取文件名到数据存储对象的映射关系，然后通过 EOSClient读取数据
- EOSClient(对象层)向元数据管理中的MDS进行 数据分布请求(数据分布信息由DN和MDS的心跳定期维护)
  + 和DataNode的数据流交互也是通过EOSClient
  + EOSClient的写入，向主MDS请求，过程中主MDS和备MDS会进行信息同步
    * 请求时，MDS返回关于DN节点的可用信息，等EOSClient操作完DN后，DN向MDS汇报
  + 读取，向主MDS获取objectLocated位置信息，然后根据位置向对应DN节点读取数据

##### 数据分布逻辑
  - 文件ID：`FileID`，唯一标识一个文件
  - 对象ID：`ObjectID`，标识一个文件内的一个对象，仅在文件内唯一
    + 每个对象由分布于*多个*存储节点上的Object block有序数组(对象块有序数组)组成
    + `Object block` 对象块
      * 多个对象块组成一个对象
      * 由于纠删码需要将数据分片分发到多个OSD上，所以文件的一个对象由分布到一组数据节点上的一组Object block有序组成
    + `Piece` 数据分片
      * 客户端在特定分布策略下，会按固定的piece大小进行数据分片，并在分片数达到计算冗余条件时(需要达到配置的可用冗余分数来实现高可用)进行冗余计算
      * 并将这组分片的各个piece分发至多个DN，即`条带单位`，其大小等于`条带大小`
        - `条带大小`的选择需要折中考虑*客户端缓存量*、*数据实时性*、*磁盘IO性能*几个方面
  - `OSD`：对象存储节点
    + 提供按 `FileID + ObjectID + BlockID` 为索引的数据对象访问接口
##### 可靠性设计
  - 元数据可靠性
    + Editlog(类似mysql binlog?代码待看)
    + MDS HA
    + Fsimage+editlog备份(类似redis的rdb+aof?)
    + CS mysql双写(串行 or 并行)
    + 定时备份mysql
  - 数据可靠性
    + Erasure coding 纠删码
    + 异常及时(即时？)检测
    + 快速数据恢复
##### 高性能设计
  - 数据控制分离
  - 并发数据流形成IO聚合效应
  - 内存架构
  - SSD加速
  - Datanode分担元数据管理
##### 不足之处
  - 文件规模支持量不足
  - IOPS性能不高
  - 元数据可靠性进一步提升
  - MDS主备异步同步元数据风险
  - 元数据硬件成本
  - 多域、多用户、异地灾备等特性不支持