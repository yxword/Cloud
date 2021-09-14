# Socket

### 主机字节序和网络字节序

字节序分为 `大端字节序`和`小端字节序` 由于大多数PC采用小端字节序(高位存在高地址处), 所以小端字节序又称为主机字节序

为了防止不同机器字节序不同导致的错乱问题. 规定传输的时候统一为 大端字节序(网络字节序). 这样主机会根据自己的情况决定 - 是否转换接收到的数据的字节序

### API

#### 基础连接

```C++
// 主机序和网络字节序转换
#include <netinet/in.h>
unsigned long int htonl (unsigned long int hostlong); // host to network long
unsigned short int htons (unsigned short int hostlong); // host to network short
```
```C++
// IP地址转换函数
#include <arpa/inet.h>
// 将点分十进制字符串的IPv4地址, 转换为网络字节序整数表示的IPv4地址. 失败返回INADDR_NONE
in_addr_t  inet_addr( const char* strptr);

// 功能相同不过转换结果存在 inp指向的结构体中. 成功返回1 反之返回0
int inet_aton( const char* cp, struct in_addr* inp);

// 函数返回一个静态变量地址值, 所以多次调用会导致覆盖
char* inet_ntoa(struct in_addr in); 

// src为 点分十进制字符串的IPv4地址 或 十六进制字符串表示的IPv6地址 存入dst的内存中 af指定地址族
// 可以为 AF_INET AF_INET6 成功返回1 失败返回-1
int inet_pton(int af, const char * src, void* dst);
// 协议名, 需要转换的ip, 存储地址, 长度(有两个常量 INET_ADDRSTRLEN, INET6_ADDRSTRLEN)
const char* inet_ntop(int af, const void*  src, char* dst, socklen_t cnt);
```
```C++
// 创建 命名 监听 socket
# include <sys/types.h>
# include <sys/socket.h>
// domain指定使用那个协议族 PF_INET PF_INET6
// type指定服务类型 SOCK_STREAM (TCP协议) SOCK_DGRAM(UDP协议)
// protocol设置为默认的0
// 成功返回socket文件描述符(linux一切皆文件), 失败返回-1
int socket(int domain, int type, int protocol);

// 关闭连接
#include <unistd.h>
// 参数为保存的socket
// 并非立即关闭, 将socket的引用计数-1, 当fd的引用计数为0, 才能关闭(需要查阅)
int close(int fd);

// 立即关闭
#include <sys/socket.h>
// 第二个参数为可选值 
//	SHUT_RD 关闭读, socket的接收缓冲区的数据全部丢弃
//	SHUT_WR 关闭写 socket的发送缓冲区全部在关闭前发送出去
//	SHUT_RDWR 同时关闭读和写
// 成功返回0 失败为-1 设置errno
int shutdown(int sockfd, int howto)

// ============== 服务器端
// 绑定socket
// socket为socket文件描述符
// my_addr 为地址信息
// addrlen为socket地址长度
// 成功返回0 失败返回 -1
int bind(int socket, const struct sockaddr* my_addr, socklen_t addrlen);

// 监听
// backlog表示队列最大的长度
int listen(int socket, int backlog);

// 接受连接 失败返回-1 成功时返回socket
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)

// ============== 客户端
// 发起连接
// 第三个参数为 地址指定的长度
// 成功返回0 失败返回-1
int connect(int sockfd, const struct sockaddr * serv_addr, socklen_t addrlen);
```

#### TCP发送和接收数据

```C++
#include<sys/socket.h>
#include<sys/types.h>

// 读取sockfd的数据
// buf 指定读缓冲区的位置
// len 指定读缓冲区的大小
// flags 参数较多
// 成功的时候返回读取到的长度, 可能小于预期长度, 需要多次读取.   读取到0 通信对方已经关闭连接, 错误返回-1
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
// 发送
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```
* Flag：

| 选项名        | 含义                                                         | 可用于发送 | 可用于接收 |
| ------------- | ------------------------------------------------------------ | ---------- | ---------- |
| MSG_CONFIRM   | 指示链路层协议持续监听, 直到得到答复.(仅能用于SOCK_DGRAM和SOCK_RAW类型的socket) | Y          | N          |
| MSG_DONTROUTE | 不查看路由表, 直接将数据发送给本地的局域网络的主机(代表发送者知道目标主机就在本地网络中) | Y          | N          |
| MSG_DONTWAIT  | 非阻塞                                                       | Y          | Y          |
| MSG_MORE      | 告知内核有更多的数据要发送, 等到数据写入缓冲区完毕后,一并发送.减少短小的报文提高传输效率 | Y          | N          |
| MSG_WAITALL   | 读操作一直等待到读取到指定字节后才会返回                     | N          | Y          |
| MSG_PEEK      | 看一下内缓存数据, 并不会影响数据                             | N          | Y          |
| MSG_OOB       | 发送或接收紧急数据                                           | Y          | Y          |
| MSG_NOSIGNAL  | 向读关闭的管道或者socket连接中写入数据不会触发SIGPIPE信号    | Y          | N          |

#### UDP发送和接收数据

```C++
#include <sys/types.h>
#include <sys/socket.h>
// 由于UDP不保存状态, 每次发送数据都需要 加入目标地址.
// 不过recvfrom和sendto 也可以用于 面向STREAM的连接, 这样可以省略发送和接收端的socket地址
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
ssize_t sendto(int sockfd, const void* buf, size_t len, ing flags, const struct sockaddr* dest_addr, socklen_t addrlen);
```





# NetFramework

## 概述

网络框架基础库模块 （NetFramework）

* 应用程序和操作系统平台之间的一层
* 两大模块，十项功能，线程模型和网络、内存及磁盘I/O两个方面
* 网络无关：定时器、异步消息、异步磁盘I/O、优化的内存池
  网络相关：IPv4、IPv6、TCP、UDP、Socket、SSL 六个接口简洁的功能模块



## 初始化和线程模型 CNetThread

头文件 NetThread.h

### 初始化和公有线程池 

* Leader/Follower线程模型

  * 一个线程监听注册到框架内的所有事件（网络IO，定时器，异步消息等）
  * 其余线程睡眠，有事件发生则唤醒处理任务
  * 阻塞执行，n个线程数的阻塞事件最多不能超过n-1个

* NetFramework会对线程数量自动调整，以用户指定数量为基础，不少于CPU数量，并自动向上调整为CPU数量的整数倍

* 接口

  * public static int CNetThread::**CreateThreadPool**( uint32_t thread_num, bool use_aio = false );

    * 创建线程
    * 参数：tread_num 线程数量；use_aio 是否使用AIO（异步磁盘IO），默认不开启；
    * 返回值：-1失败，0成功

  * public static int CNetThread::**DestroyThreadPool**();

    * 销毁整个线程池，程序结束运行时调用
    * 返回值：0 成功，无失败情况

  * 示例

    ```C++
    if( CNetThread::CreateThreadPool( 4, true ) < 0 ){
        fprintf( stderr, "创建线程池失败\n" );
        CNetThread::DestroyThreadPool();
        return -1;
    }
    ```



### 初始化和私有线程池

* 在隔离环境中**调试测试使用**，不受同一系统其他模块干扰
* 线程池数量上限63个
* 接口
  * public static int CNetThread::**CreatePrivatePool**(uint32_t thread_num, bool use_aio = false);
    * 同共有线程池
    * 返回值：-1失败；成功返回创建成功的私有线程池的索引号（1~63），供后续使用



### 线程池相关参数设置

* static int **SetThreadPriority**(int priority, int policy = policyNormal, int stackSize = 0);

  *  功能：设置网络框架线程优先级。其在调用CreateThreadPool前调用

  * 参数：

    * priority 线程优先级，值越小优先级越高。枚举量，priorDefault为缺省值

      ```c++
      enum Priority{
          priorTop = 1,
          priorBottom = 127,
          priorDefault = 64
      };
      ```

    * policy 线程调度策略。枚举量如下

      ```C++
      enum Policy{
          policyNormal = 0,   // 普通线程
          policyRealtime = 1  // 实时线程
      };
      ```

    * stackSize 线程指定堆栈大小

  * ~~返回值~~

* static void **SetRcvPriority**( int level );

  * 功能：设置网络框架在接受流媒体数据时的接收策略（流媒体接收工具CMediaStreamReceiver）
  
  * 框架内部已设置了合理策略，无特殊需要，不要随意调用该接口
  
  * 在初始化线程池前调用
  
  * 参数：level，指定具体策略。枚举量
  
    ```C++
    enum{
        HIGH_REALTIME = 0, // 高实时性，尽快接收数据，牺牲性能为代价
    	REALTIME = 1,      // 实时性，保证绝大部分无延迟接收
    	BALANCE = 2,       // 平衡
    	PERFORMANCE = 3    // 性能优先，牺牲实时性为代价
    };
    ```



## 建立对象 CNetHandler

头文件 NetHandler.h

* public long CNetHandler::**GetID**();
  * 功能：获取对象的ID号
  * 返回值：long类型的表示ID号的负数
  * 利用这个接口，一个CNetHandler对象可以获取利用另一个对象的ID号，并向对方发送消息
* public void CNetHandler::**Close**();
  * 功能：结束一个CNetHandler对象
  * 自动向NetFramework注销关于对象的所有注册信息，做完相关清理工作后，会回调CNetHandler对象的handle_close函数
* public **virtual** int CNetHandler::**handle_close**( CNetHandler* myself )
  * 功能：Close关闭对象后回调，用户重新实现完成一些相关清理工作（类似析构函数）。
  * 参数：myself相当于this指针。
  * 返回值 -1 无定义
  * 示例 见定时器



## 建立定时器

### 定时器的使用

头文件 NetHandler.h

* protected long CNetHandler::**SetTimer**( int64_t usec );
  * 功能：建立指定超时时间的定时器
  * 参数：usec 表示从当前时间开始多少微秒后本定时器超时
  * 返回值：定时器的 ID 号，为负整数，需要保存下来，通过ID号判断哪个定时器超时
  * 注意：该接口精度是微秒，但NetFramework定时器是毫秒级，**若usec<1000，相当于0，马上超时**
  * 超时后框架会回调handle_timeout虚函数
* public **virtual**  int64_t CNetHandler::**handle_timeout**(long id);
  * 功能：定时器超时后的回调函数，用户重新实现完成超时后需处理的一些事件
  * 参数：id 发生超时的定时器的 ID
  * 返回值：
    * <0 (通常为-1)，销毁该定时器，定时器的ID也被回收
    * =0 触发下一个超时任务，超时时间和上一个超时任务时间相等
    * \>0 触发下一个超时任务，超时时间为返回值
* protected void CNetHandler::**DestroyTimer**( long id );
  * 功能：销毁一个定时器 （定时器还未超时时，不希望产生超时事件时使用）
  * 参数：要销毁的定时器 ID 号



回调函数都是网络框架的线程在后台处理

### 多线程给定时器带来的问题

* 使用DestroyTimer销毁一个定时器的时候，该定时器的一个超时事件可能已经发生，对用户来讲，所看到的现象就是在调用DestroyTimer并成功返回之后，handle_timeout又被调用了一次

  （需要避免DestroyTimer后直接删掉了某些资源，而handle_timeout又不对这些资源的有效性进行检测，而直接访问）

* Close 和 handle_close 的问题同上

* 同一对象多个定时器，同一时间点同时触发超时事件，调用同一个handle_timeout，需要注意访问的时同一个资源时，需做好资源保护工作。

### 示例

见Example中的Timer.h和Timer.cpp



## 多个CNetHandler间通信

### 异步消息的使用 

NetHandler.h

* protected void CNetHandler::**Notify**(long receiver id, MSG_TYPE type, long info);

  * 功能：向CNetHandler对象发送消息
  * 参数：
    * receiver_id  接收对象 ID
    * type  发送的消息类型
      typedef unsigned int MSG_TYPE;
      NetFramework可以**自定义**消息类型，只要**发送者和接收者保持一致**即可
    * info  消息的附带信息
  * 消息发出后，接收对象会回调 handle_message 虚函数，用户自己实现消息处理工作

* public **virtual** int CNetHandler::**handle_message**( long from_id, MSG_TYPE type, long attach );

  * 功能：接收者的回调函数，处理消息
  * 参数：
    * from_id  发送对象 ID
    * type  同上，由发送者和接收者自定义
    * attach  同info，含义由双方协商
  * 返回 -1 无定义
  * 

* public void CNetHandler::**SetDefaultRcer**( long receiver_id );

  * 功能：为对象设置一个异步消息的默认接收者
  * receiver_id  默认接收者 ID

  protected void CNetHandler::**Notify**( MSG_TYPE type, long info );

  * 功能：向默认接收者发送消息

* protected void CNetHandler::**NotifyIO**( long receiver_id, MSG_TYPE type, long info );
  protected void CNetHandler::**NotifyIO**( MSG_TYPE, long info );

  * 功能：和Notify相同，但不占用正常的网络框架工作线程，而是通过异步磁盘I/O线程处理消息，即I/O专用线程

### 多线程给异步消息带来的问题

* 同定时器问题，需要做好对资源的保护工作
* 消息发送时，接收者可能不存在了，或者接收者不一定能收到消息；消息接收时，发送者可能不存在了。NetFramework都已做相应的处理，不会引起异常，只是消息会被丢弃

### 示例

见Example 的 Message.h 和 Message.cpp



## 使用异步磁盘 I/O

磁盘 I/O 比较耗时，多个磁盘 I/O 操作容易引起整个系统无响应

### 建立存储设备对象 CNStorDev_IF

NStorDev_IF.h

* public **virtual** int CNStorDev_IF::**Open**( const char* path, const char* mode ) = 0;

  * 功能：打开设备上的文件或者其它类型的数据集合，由用户存的储设备本身定义
  * 参数：
    * path  打开的数据集的标识符，即文件的路径名
    * mode  打开的方式，读、写、追加或其它
  * 返回 -1失败 0成功
  * 用户自己实现该接口

* public **virtual** void CNStorDev_IF::**Close**() = 0;

  * 功能：关闭文件或类似的数据集合，由用户本身的存储设备定义

* public **virtual** int CNStorDev_IF::**Write**( const char* buf, const uint32_t len ) = 0;

  * 功能：向已打开的CNStorDev_IF对象写入数据
  * 参数：
    * buf  待写入数据
    * len  写入数据的长度
  * 返回值：
    * \>=0 实际写入的数据量
    * -1 失败

* public **virtual** int CNStorDev_IF::**Read**( char* buf, const uint32_t len ) = 0;  

  * 功能：读数据
  * 参数：
    * buf  [OUT] 存放读到的数据的缓冲区
    * len  缓冲区长度
  * 返回值：同写入

* public **virtual** int CNStorDev_IF::**Seek**( int64_t offset, int whence ) = 0;

  * 功能：在已打开的文件或数据集合中移动读写指针

  * 参数：

    * offset  偏移量，单位字节

    * whence  从何处开始偏移，取值如下

      ```C++
      enum{
          FILE_HEAD = 0, // 文件起始
          FILE_CUR,  // 当前位置
      };
      ```

  * 返回  -1失败 0成功

上述接口定义了 **开、关、读、写、移动五个操作，都需要用户自定义实现**，此外还有两个接口：

* public virtual int CNStorDev_IF::**Stat**( struct NFile_Info* state ) = 0;

  * 功能：获取已打开文件或数据集合的一些基本属性信息

  * 参数：stat [OUT] 存储获取到的信息的一个结构体

    ```C++
    struct NFile_Info
    {	
    	uint64_t	time_create;	///< 文件创建时间
    	uint64_t	time_access;	///< 文件访问时间
    	uint64_t	time_modify;	///< 文件修改时间
    	uint64_t	file_size;		///< 文件大小
    	uint32_t	attrib;		///< 文件属性
    	char	name[256];		///< 文件名
    }; 
    // attrib
    enum{
    	NORMAL		= 	0x00,
    	READ_ONLY 	= 	0x01,
    	HIDDEN 		= 	0x02,
    	SYSTEM 		= 	0x04,
    	VOLUME 	= 	0x08,
    	DIRECTORY 	= 	0x10,
    	ARCHIVE 	=   0x20
    };
    ```

    若用户的存储设备的文件只拥有部分信息，只填一部分，没有的置0

  * 返回：-1失败 0成功

* ~~public virtual bool CNStorDev_IF::**NeedAIO**() {return true;}~~

  * 功能：设置用户的存储设备是否需要NetFramework提供的AIO功能
  * 返回：默认true，即需要，用户可以重载该接口，返回false来关闭
  * 注意Linux下，默认需要，即不必重现该接口，NetFramework内部作了支持

### 虚拟文件对象的文件读写 CNFile

NFile.h

CNFile类，虚拟文件对象，只对用户要求读写的数据缓冲，最终提交给CNStorDev_IF

* public static CNFile* CNFile::**Open**( long recvr_id, const char* path,  const char* mode, CNStorDev_IF* dev = NULL );
  * 打开一个虚拟文件对象
  * 参数：
    * recvr_id  异步消息的接收者ID
    * path，mode 同CNStor_Dev_IF，Linux下同 fopen 的参数
    * dev  用户自己实现的存储设备对象，默认是Linux标准存储设备
  * 返回：成功返回CNFile对象指针，失败返回NULL
* public void CNFile::**Close**();
  * 功能：关闭CNFile，关闭后CNFIle对象指针立即失效，不能自己再去delete该指针
* public int CNFile::**Read**( char* buf, uint32_t len, bool& is_EOF );
  * 参数：~~buf、len~~
    * is_EOF  [OUT] 指示符，若读取到达文件为，则为true，否则为false
  * 返回： \>=0 实际读取到的量
  * 若某次返回0，但is_EOF为false，表示未读完，只是暂时读不到数据；此时，CNFile会向存储设备发请求，读入存储设备自己的缓冲区；然后会向用户发送MSG_FILE_READ，表示数据准备好可以读了，即可再去调用该Read接口，保证读到数据
  * 发生错误，会收到MSG_FILE_EXCPTION消息
* public int CNFile::**Write**( const char* buf, const uint32_t len );
  * ~~参数、返回~~
  * 若某次写入小于指定的len，表示暂时不能写，用户应等待CNFile对象的消息MSG_FILE_WRITE，表示可以继续写入
  * 发生错误，同Read
* public int CNFile::**Seek**( int64_t offset, int whence );
  * 功能、参数和返回值：同CNStorDev_IF::Seek
  * whence不允许从文件末尾开始偏移
* CNFile::Stat 同 CNStorDev_IF::Stat
* public int CNFile::**SetBufferSize**( uint32_t size_per_buffer = DEFAULT_BUFFER_SIZE, uint32_t buffer_num = DEFAULT_BUFFER_NUM );
  * 功能：设置CNFile内部的缓冲区的数量和大小
  * 参数：
    * ~~size_per_buffer  每块缓冲区的大小，目前已无效，系统内部自动优化，随意传值即可~~
    * buffer_num  缓冲区数量，DEFAULT_BUFFER_NUM=4，可自定义，一般读文件设为2，写文件设置为8+。

### 异步磁盘I/O需要注意的问题

* Read和Wirte接口会发送异步消息，分别为MSG_FILE_READ，MSG_FILE_WRITE，MSG_FILE_EXCEPTION消息
* 极端下，磁盘处理非常快时，Read还未返回 0，而MSG_FILE_READ已经到达用户的接收函数，此时需要做好相关资源保护，保证执行逻辑。

### 示例

FileCopy.h和FileCopy.cpp

代码中针对上面的问题，对copy_read和copy_write进行锁保护



## 优化过的Malloc和Free

* protected: inline void* CNetHandler::**TsMalloc**( uint32_t size );

  * 功能：申请分配一定内存
  * 参数：size 要分配的内存长度
  * 返回：指向分配成功的内存的首地址的指针

* protected: inline void CNetHandler::**TsFree**( void* ptr );

  * 功能：释放由TsMalloc 申请分配的内存
  * 参数：待释放的内存的指针

* 上面量函数是内联函数，实际是调用CTsMemory类的同名函数，**TsFree只能释放由TsMalloc申请的内存**，**TsMalloc/TsFree只能在网络框架的线程中被调用**（即handle_*回调函数中，以及其子函数），否则会造成崩溃

* **建议分配小于224K的内存块**，TsMemory只支持对小于224K内存的访问进行优化

* 使用非线程安全版本的Malloc和Free，实际应用中以**避免使用第三方库造成的问题**

  * protected: inline void* CNetHandler::**Malloc**( uint32_t size );
  * protected: inline void CNetHandler::**Free**( void* ptr );
  * 不是线程缓冲的，这两个函数的执行可能会产生局部的锁竞争，效率会低于TsMalloc/TsFree

  

## IPv4地址的使用 CSockAddrIPv4

SockAddr.h

CSockAddr  地址的基类

* public: uint32_t CSockAddr::**GetType**() const;

  * 功能：获取当前地址对象
  * 返回：
    * CSockAddr::SOCKADDR_TYPE_UNKNOW   未知地址类型
    * CSockAddr::SOCKADDR_TYPE_IPV4   IPv6地址类型
    * CSockAddr::SOCKADDR_TYPE_IPV6   IPv6地址类型
    * CSockAddr::SOCKADDR_TYPE   地址存储类型，本身没有意义，其存储的地址类型为枚举的几个类型之一
      * CSockAddr::SOCKADDR_TYPE_UN   UNIX域的地址类型
      * CSockAddr::SOCKADDR_TYPE_IPV4
      * CSockAddr::SOCKADDR_TYPE_IPV6
      * CSockAddr::SOCKADDR_TYPE_STORAGE

* public: **virtual** bool CSockaddr::IsValid()const **= 0**;

  * 功能：判断当前地址对象是否有效，为纯虚函数，派生类必须实现

* 针对Linux的socket系统调用，如connect函数：

  int **connect**(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);

  CSockAddr提供如下接口来获取地址结构体的指针和地址结构体长度，它们都是纯虚函数，由派生类实现：

  * public: virtual struct sockaddr * CSockaddr::**GetAddr**( struct sockaddr * addr ) const = 0;

  * public: virtual uint32_t CSockAddr::**GetAddrLen**() const = 0;

  * 参数：  addr 是由用户提供的struct sockaddr结构体的地址，函数会在该addr中填充取得的地址，并把指针原样返回，避免不必要的内存分配问题

  * 使用示例：

    ```C++
    // remote 表示对端地址的 CSockAddr对象
    struct sockaddr tmp_addr;
    int retval = connect( fd, remote.GetAddr(&tmp_addr), remote.GetAddrLen() );
    ```

  * 后面章节提供更多可用的设施，无需用户关心connect这样的细节。

* 提供端口检测接口，使用临时端口时，避免可能产生临时端口和保留端口冲突的问题
  public bool CSockAddr::**IsReserved**();

  * 功能：检测当前地址端口是否为保留端口号
  * 返回：true 保留端口号，false 非保留端口号

* CSockAddrIPv4类是IPv4地址的真正实现，提供各种数据类型的IP地址和端口的设置和获取，方便用户的使用，甚至可以**把域名作为IP地址**传入。接口简单，不一一说明。？

* 所有IP地址和端口号，和所有获取到的IP地址和端口号，都是主机字节序的，用户根本接触不到网络字节序的数据。



## Ipv6地址的使用 CSockAddrIPv6

同IPv4地址类，CSockAddrIPv6也继承自CSockAddr



## Storage地址的使用

SockAddrStorage.h

* 作为CSockAddr的子类，其多数接口和意义，和IPv4和IPv6类同。

* 注意设置IP地址时，仅支持设置**字符串格式的IP地址或者域名**，而不像IPv4允许输入unsigned int型地址

* 本类作为地址存储类，允许存储IPv4和IPv6的地址格式，并通过接口方便使用
* 存在IPv4映射的IPv6地址，本质是IPv4，但形式上是IPv6
  如IPv4地址：12.13.14.15，其IPv4映射的IPv6地址为::ffff:12.13.14.15
* 由于IP地址不确定是IPv4还是IPv6，存在不确定性，提供如下接口，得到其实质类型：
  uint32_t **GetRealType**()const; 
  * 功能：获取地址存储的本质类型
  * 返回：CSockAddr::SOCKADDR_TYPE_IPV4、CSockAddr::SOCKADDR_TYPE_IPV6或者CSockAddr::SOCKADDR_TYPE_UNKNOWN之一
* bool **GetRealSockAddr**( CSockAddr* addr );  
  * 功能：将存储地址转换为特定类型的地址。如GetRealType()返回的是CSockAddr::SOCKADDR_TYPE_IPV4，则可以通过此接口，传入一个CSockAddrIPv4对象指针，由存储地址类型填充，完成从地址存储类型到实际地址类型的转换。



## Socket对象CSock

Sock.h

CSock对象是所有套接字对象的基类，定义了一组基本的接口

* public uint32_t CSock::**GetType**();
  * 功能：获取套接字类型
  * 返回：
    * CSock::SOCK_TYPE_RAW  原始套接字
    * CSock::SOCK_TYPE_STREAM  面向流的套接字(TCP)
    * CSock::SOCK_TYPE_DGRAM  数据报套接字(UDP)
    * CSock::SOCK_TYPE_SSL_STREAMC  数据加密的TCP流套接字
    * CSock::SOCK_TYPE_SIMU_STREAM  测试用的仿真TCP流套接字
    * Sock::SOCK_TYPE_OTHER  其它类型的套接字
    * CSock::SOCK_TYPE_UNKNOW  未知类型的套接字
* ~~public int CSock::**Close**();~~
  * 功能：关闭套接字对象的句柄，但不销毁CSock对象本身
  * 返回：-1失败，0成功
  * 对象生命周期结束后，自己会关闭句柄，所以，除非特殊情况，一般不需要调用Close
* public void CSock::**SetBlockOpt**( bool isblock );
  * 功能：设置套接字对象的阻塞模式
  * 参数：isblock，true阻塞，false非阻塞
  * NetFramework使用套接字要求是非阻塞的，框架自动完成对读写事件的调度，所以默认是非阻塞的，若用户需要，可以设置成阻塞的
*  bool **GetBlockOpt**();
  * 功能：获取套接字对象当前的阻塞模式
  * 返回：true 阻塞，false非阻塞
* public: int CSock::**GetLocalAddr**( CSockAddr *local );
  public: int CSock::**GetRemoteAddr**( CSockAddr *remote );
  * 功能：获取当前工作状态的本地地址和对端地址
  * 返回：0成功，-1失败
* public bool CSock::**IsValid**();
  * 功能：检测套接字是否有效
  * 返回：true有效，false无效
* public int CSock::**GetHandle**();
  * 功能：获取套接字对象中的句柄
  * 返回：套接字句柄
* public: int CSock::**Attach**( int sockfd );
  public: int CSock::**Detach**();
  * 功能：把句柄sockfd联接到对象上或解除对象的句柄联接
  * 返回：-1失败，0成功 ；被解除联接的句柄
  * 该接口适用于：用户通过其它方式建立的网络连接，得到的套接字句柄fd，将其联接到套接字对象上，从而帮助完成在此网络连接上的所有操作。解除联接同理。



## TCP CSockStream和CSockAcceptor

### CSockStream类

SockStream.h

* 流套接字CSockStream继承自CSock，其类型为CSock::SOCK_TYPE_STREAM，额外提供TCP流 I/O的接口和设置套接字属性的接口

* public: int CSockStream::**Connect**(const CSockAddr & remote, CSockAddr* local = NULL);

  * 功能：与对端TCP连接；也可以通过local，指定绑定本地的特定IP地址和端口
  * 返回：-1失败；  0成功（阻塞模式下），或者连接进行中（非阻塞模式）
  * 返回0不能看作是成功，也可能是连接进行中，可通过下面接口判断

* public: int CSockStream::**GetConnectStatus**();

  * 功能：获取当前连接状态
  * 返回：
    * CSockStream::STATUS_CONNECTED  连接已成功
    * CSockStream::STATUS_NOTCONNECTED  还未连接成功，如果此时已调用过Connect，请继续等待
    * CSockStream::STATUS_ERROR  连接失败

* TCP套接字的属性设置接口：
  1）public: int CSockStream::**SetSockRecvBuf**( uint32_t len );
  2）public: int CSockStream::**SetSockSendBuf**( uint32_t len );

  * 功能：设置 TCP 发送/接收缓冲
  * 参数：len 同系统调用 setsockopt 的参数
  * 返回：-1失败  0成功

  3）public: int CSockStream::**GetSockRecvBuf**( );

  4）public: int CSockStream::**GetSockSendBuf**( );

  * 功能：获取 TCP 的接收/发送缓冲区的大小
  * 返回：实际的缓冲区大小

  5）public: int CSockStream::**SetSockKeepAlive**( bool keepalive );

  * 功能：设置 TCP 保活能力
  * 参数：keepalive 为true表示保活，false表示不保活
  * 返回：-1失败  0成功

  6）public: int CSockStream::**SetSockNoDelay**( bool nodelay );

  * 功能：设置 TCP 无延迟发送
  * 参数：nodelay  true表示设置无延迟，false表示不设置
  * 返回：-1失败  0成功

* 两组收发接口：

  * 不指定时间：
    public: int CSockStream::**Send**( const char * buf, uint32_t len );

    * 功能：发送 TCP 数据
    * 参数：buf待发送数据，len待发送长度
    * 返回：-1失败，0未发送，\>0 发送出去的字节数

    public: int CSockStream::**WriteV**( const struct iovec *vector, int count );

    * 功能：发送 TCP 数据
    * 参数：同 Linux 调用 writev
    * 返回：-1发送失败，\>0 发送出去的字节数

    public: int CSockStream::**Recv**( char * buf, uint32_t len );

    * 功能：接收 TCP 数据
    * 参数：buf [OUT] 接收缓冲区，len缓冲区长度
    * 返回：-1失败（errno==ECONNRESET表示对方已断开），0未收到数据，\>0为收到的字节数

  * 指定时间内完成收发操作，若超时，不管是否收发完，都会返回：
    public: int CSockStream::**Send_n**( const char * buf, uint32_t len, int timeout = 1000000 );
    public: int CSockStream::**Recv_n**( char * buf, uint32_t len, int timeout = 1000000 );

    * 参数：timeout 超时时间，单位μs（1/1000000s)，默认超时时间是1s
    * 若要使用这俩接口，套接字对象必须设置为非阻塞模式，否则不会超时

### CSockAcceptor类

   SockAcceptor.h

* 建立TCP侦听队列，接受来自远端的TCP连接请求，并为请求建立一个新的连接；成功建立产生一个新的CSockStream对象，它的工作便宣告完成。（ 类似listen和accept ）

* 它不处理任何应用层的TCP流数据。它的类型是：CSock::SOCK_TYPE_OTHER

* TCP服务器会先建立CSockAcceptor对象，并通过Open接口打开

  public int CSockAcceptor::**Open**( const CSockAddr & local );

  * 功能：建立 TCP 侦听队列（ 同 listen ）
  * 参数：local 本地的侦听地址
  * 返回：-1失败，0成功

  public: CSockStream* **Accept**( CSockAddr * remote = NULL );

  * 功能：若侦听到连接请求，则接受远端 TCP 连接请求 （ 同 accept ）
  * 参数：remote[OUT]  用于获取远端地址，也可为NULL
  * 返回：NULL失败；成功则返回CSockStream对象指针，可以用于处理该连接上的I/O事件
  * **返回值需要用户自己管理，不需要时需要delete防止内存泄漏**

  

## UDP CSockDgram

SockDgram.h

* CSockDgram类的类型是CSock::SOCK_TYPE_DGRAM，主要完成UDP数据报的单播及多播的I/O功能

* public: int CSockDgram::**Open**(const CSockAddr * addr = NULL);

  * 功能：打开一个UDP套接字
  * 参数：addr用于绑定一个本地地址（选定一块网卡和一个端口号），用于单播；也可用绑定一个组播地址，加入组播组；或者不指定，系统随机选择一个端口用于单播
  * 返回：-1失败，0成功

* 收发：
  public: int CSockDgram::**Send**( const char * buf, uint32_t len, const CSockAddr & remote);
  public: int CSockDgram::**Recv**(char * buf, uint32_t len, CSockAddr* remote = NULL);

  * ~~同CSockStream~~

* ==设置组播相关属性==
  public: int CSockDgram::**SetMulticastIF**( uint32_t ip_h );
  public: int CSockDgram::**SetMulticastIF**( const char * ipstr );

  * 功能：设置组播发送的本地接口，即在多网卡环境下，指定一块网卡。
  * 返回：-1失败，0成功
  * 注意第一个接口不支持IPv6，要设置IPv6的本地接口，只能使用第二个接口。

  public: int CSockDgram::**SetMulticastTTL**( uint8_t TTL );

  * 功能：设置组播发送报文的TTL
  * 参数：TTL  （TimeToLive 生存事件）
  * 返回：-1失败，0成功

  public: int CSockDgram::**SetMulticastLoop**( bool isloop );

  * 功能：设置是否允许组播数据回送，即是否允许自己收到组播数据
  * 参数：isloop  true表示会送，false表示不会送
  * 返回：-1失败，0成功
  * 

## Raw CSockPacket

SockPacket.h

* 原始套接字允许用户进行一些底层的网络编程，比如，需要发送ARP，RARP协议数据报，或者ICMP协议数据报，或者完全自己组包发送UDP协议数据报；

* CSockPacket 根据用户的设置，替用户完成这些组包工作，而使用户完全不需要了解这些协议本身的底层细节

* CSockPacket 只针对IPv4的一个原始套接字对象，IPv6的原始套接字对象将另外单独提供

* public: int CSockPacket::**Open**( const char* ifname, uint16_t protocol = ETH_P_IP, uint8_t ip_protocol = IPPROTO_UDP );

  * 功能：打开一个原始套接字
  * 参数：
    * ifname：网络接口的名字，Linux下如eth0，eth1，lo等，字符串形式
    * protocol：协议名称，支持的协议名称如下
      * ETH_P_IP  发送ip类型的数据帧，同时只接收发往本机mac的ip类型的数据帧，为缺省值
      * ETH_P_ARP  发送arp类型的数据帧，只接受发往本机mac的arp类型的数据帧
      * ETH_P_RARP 发送rarp类型的数据帧，只接受发往本机mac的rarp类型的数据帧
      * ETH_P_ALL  接收发往本机mac的所有类型ip arp rarp的数据帧，接收从本机发出的所有类型的数据帧。(混杂模式打开的情况下，会接收到非发往本地mac的数据帧)，**目前还不完善，请尽量避免使用**
    * ip_protocol：**当protocol为默认值ETH_P_IP时，该参数才有意义**，表示网络层或传输层的协议名，支持的协议名和含义如下：
      * IPPROTO_UDP UDP协议，缺省值
      * IPPROTO_ICMP  ICMP协议
      * l IPPROTO_IGMP IGMP协议
  * 返回：-1失败，0成功
  * CSockPacket允许多次调用Open，每次调用都会使前一次的打开失效，套接字以新打开的协议执行后续的操作。

* Open后，收发操作之前，需要CSockPacket提供的接口进行设置，否则无法生成以下首部和报文

  * 传输层协议首部（CSockPacket中只支持UDP）
  * 网络层协议的报文格式（ICMP报文，IGMP报文）
  * 网络层协议首部(IP首部)
  * 数据链路层首部（以太网首部）
  * 数据链路层协议报文（ARP,RARP报文）

  public: int CSockPacket::**SetLocalMac**( unsigned char * mac_addr );

  public: int CSockPacket::**SetRemoteMac**( unsigned char * mac_addr );

  * 功能：设置本地和对端mac地址，供对象构建以太网首部，ARP和RARP报文（若需要）
  * 参数：mac_addr 以太网地址，unsigned char 数组，长度为ETH_ALEN(6)
  * 返回：-1失败，0成功

  public: int CSockPacket::**SetLocalIP**( CSockAddrIPv4 &local );

  public: int CSockPacket::**SetRemoteIP**( CSockAddrIPv4 &remote );

  * 功能：设置本地和对端IP地址，供对象构建IP首部，ARP和RARP报文（若需要）
  * 返回：-1失败，0成功

  上面的接口设置后，对象具备了：以太网首部，ARP，RARP报文（如果需要），IP首部（如果需要），UDP首部（如果需要）。之后用户发送数据的话，只要提供：应用层数据（UDP协议），ICMP和IGMP报文。大大简化工作

* 原始套接字的参数获取接口

  public: int CSockPacket::**GetIFIndex**();

  public: int CSockPacket::**GetLocalMac**( unsigned char *buf, uint32_t len );

  public: int CSockPacket::**GetRemoteMac**( unsigned char *buf, uint32_t len );

  public: int CSockPacket::**GetLocalIP**( CSockAddrIPv4 &local );

  public: int CSockPacket::**GetRemoteIP**( CSockAddrIPv4 &remote );

  * 功能：分别用于获取套接字上的网络接口索引号，本地，对端MAC地址，本地，对端IP地址。详见SockPacket.h中的说明。
  * 返回：-1失败，GetIFIndex返回大于0的值即为索引号，其它0表示成功

* public: int CSockPacket::**Send**( const char * buf, uint32_t len );
  * 功能：发送原始数据报
  * 返回：-1失败，\>=0 表示发送成功的数据字节数
  * Send需要发送的是应用层数据或者ICMP/IGMP报文，其它都由CSockPacket自动加上，总数据包长度不得大于目标系统的MTU值（最大传输单元）
* public: int CSockPacket::**Recv**( char * buf, uint32_t& len, CSockAddr* local = NULL, CSockAddr* remote = NULL, unsigned char* local_mac = NULL, unsigned char* remote_mac = NULL );
  * 功能：接收原始数据报
  * 返回：-1失败，大于0表示接收到的数据报的协议类型：
    ETH_P_ARP，ETH_P_RARP，IPPROTO_UDP，IPPROTO_ICMP，IPPROTO_IGMP。
  * 参数：
    * buf  接收缓冲
    * len  接收缓冲区的长度，返回后会被填充成接收到的数据长度
    * local、remote  本地和对端ip地址
    * local_mac、remote_mac  本地和对端的mac地址
  * 注意Recv收到的数据是**去除了以太网首部后的数据**，即Recv可能收到一个ARP或者RARP报文，或者一个IP首部起始的IP协议数据报，而最后len的值也是去除了以太网首部后的数据长度。

## CNetHandler对象的网络I/O接口

### 网络I/O接口-注册和回调处理

NetHandler.h

* 创建套接字对象后，要向框架进行注册，告知框架，关于该套接字的存在，以及该套接字需要框架帮助监视哪些事件

* public: int CNetHandler::RegisterSock( CSock & socket, SOCK_MASK type, int timout = 0 );

  * 功能：向框架注册一个套接字对象
  * 参数：
    * socket：套接字对象
    * type：需要注册的事件，包含READ_MASK、WRITE_MASK和（READ_MASK|WRITE_MASK）
    * timeout：超市时间，0表示不超时
  * 返回：0成功，不会失败
  * socket注册READ_MASK后，如果套接字上有数据到达，框架会立即通过回调函数通知拥有该套接字的CNetHandler类对象进行接收操作，如果指定了timeout，到达超时时间后，若还没有数据到达，框架同样会通过CNetHandler类对象进行相关的出错处理操作，WRITE_MASK也类似。
  * 若同时注册了(READ_MASK|WRITE_MASK)，timeout是读写共同拥有的

* public: **virtual** int CNetHandler::**handle_input**( int handle );

  * 功能：注册READ_MASK后由数据到达时，被回调的函数，由用户实现数据接收操作
  * 参数：handle  套接字的句柄，需要通过socket->GetHandle()的值对比，确定是哪个套接字上由数据到达
  * 返回：
    * -1表示如果事先通过timeout设置了超时时间，则取消超时设置，以后等待接收数据不再会有超时；
    * 0表示如果事先通过timeout设置了超时时间，则维护这个超时时间不变
    * 大于0表示设置一个新的超时时间，单位为微秒

* public: **virtual** int CNetHandler::**handle_input_timeout**( int handle );

  * 功能：当注册的套接字到达超时时间后，还是没有数据到达时，被回调的函数，由用户实现出错处理操作
  * 参数、返回：同handle_input

* 一般不会注册WRITE_MASK事件，需要发送数据时会直接发送数据。但是若发送数据发不出去，或者只发出一部分（可能是网络忙，也可能发太多了），就需要注册一个WRITE_MASK事件，让框架在监视到套接字不忙了，可发数据了，再来通知我们。这样做，可以避免因发不出数据而阻塞等待，影响系统的效率。

* public: **virtual** int CNetHandler::**handle_output**( int handle );

  public: **virtual** int CNetHandler::**handle_output_timeout**( int handle );

  * 功能、返回值、参数：类似handle_input和handle_input_timeout

* CNetHandler还停工一组接口用于在发现socket上有异常时进行回调

  public: **virtual** int CNetHandler::**handle_exception**( int handle );

  public: **virtual** int CNetHandler::**handle_exception_timeout**( int handle );

  * 一般在这两个回调函数中，需要关闭socket，并进行相应的清理工作，或者也有可能，读一下socket上的异常数据就能恢复
  * 若框架监视到socket上有异常，但用户没有实现这俩函数，框架会回调CNetHandler基类中的相应函数，只是简单地返回-1，会导致socket上的异常数据没有被读掉，框架就会不停地调用CNetHandler:: handle_exception，导致系统响应变慢，所以，**一般建议实现handle_exception函数**，并作一些简单的处理。

* 如果用户暂时不想处理socket上的数据了，可以通过UnregisterSock接口暂时注销某类事件，过会儿又想处理socket上的数据了，可通过RegisterSock再恢复
  public: int CNetHandler::**UnregisterSock**( CSock & socket, SOCK_MASK type );

  * 功能：注销socket的一个或多个事件
  * 返回：0成功，不会失败

* public: int CNetHandler::**RemoveSock**( CSock & socket );

  * 功能：删除注册的socket，会删除其在框架中的注册信息，但不会销毁socket对象本身
  * 返回：0成功，不会失败



### 多线程给网络I/O接口带来的问题

* **注册的Socket事件调度是串行**，一个事件一个事件地处理回调函数
  （对于一个注册在框架中的CSock对象A来说，它的所有的**事件调度是串行**的，即，如果当前正在handle类的回调函数中处理A的某个读或写，或超时事件，那么此时，即使A上有新的事件发生，也是不会被调度的，只有当本次事件处理从回调函数返回之后，下一个事件才会被监视到，并产生事件调度。所以，对于某一个CSock对象来说，它的回调处理中是不需要进行多线程的保护措施的）
* **同一对象多个已注册的socket访问统一资源时，需要对该资源进行相应的保护处理**
* UnregisterSock注销了CSock对象A上的某类事件，或者用RemoveSock删除了A的注册，这时，恰恰已有一个A上的事件被监视到，并已产生了调度，那么这个调度还是会发生，但是这样的调度至多只能发生一次，此时，在相应的回调函数中不能做不会被调用的假设
* 在对象调用了Close接口，但handle_close还没有被回调到的时候，也有可能产生至多一次的这样的事件调度。但要强调的是，在handle_close函数被回调后，就不会有任何的事件被调度，**handle_close肯定是一个CNetHandler类对象的生命周期中最后一个被回调的函数**。



## 基于框架的网络通信程序示例

### TCP 示例

* TcpCli.h和TcpCli.cpp实现了一个TCP 客户端
* 需注意，Connect调用未失败，则需马上为套接字注册带超时时间的写事件，如果连接成功了，或者失败了，handle_output会被回调，这时可通过GetConnectStatus来判断连接是否完成。或者超时了，则handle_output_timeout会被回调，进行相应的出错处理
* TcpSvr.h和TcpSvr.cpp实现了一个TCP服务端
* CSockAcceptor对象Open之后，要马上注册一个读事件，有连接请求到达，handle_input就会被回调，可马上接受一个新的连接。

### UDP 示例

* UdpSvr.h和UdpSvr.cpp实现了一个UDP服务端；
* Multicast.h和Multicast.cpp实现了一个组播发送接收程序。

### RAW 示例

* ping.h和ping.cpp实现了一个简单的ping程序，它首先通过发送ARP包获取对端的mac地址，然后发送icmp回显请求。
* UdpPacket.h和UdpPacket.cpp实现了自己组包发送UDP数据报。



## 数据的加密传输

* 接口说明可以参考CSockStream和CSockAcceptor的说明，它们几乎保持了完全一致的接口形式

### CSslStream和CSslAcceptor

CSslStream.h  ==CSslAcceptor.h==

* 由于**SSL初始化的过程极其复杂**，所以CSslStream::**Connect接口是阻塞**的，它也不提供类似CsockStream的GetConnectStatus的接口
* public: int CSslStream::**Connect**(const CSockAddr & remote);
  * 功能：向远端地址建立SSL安全连接
  * 参数：remote 远端地址
  * 返回：-1失败，0成功
* CSslAcceptor::Accept接口，在建立新的连接后，需要跟对端进行密钥协商等初始化工作，所以可以认为Accept接口也是阻塞的
* public: int CSslStream::**SetPemPath**( const char* path );
  public: int CSslAcceptor::**SetPemPath**( const char* path );
  * 功能：设置ssl密钥文件cacert.pem和privkey.pem的路径
  * 参数：path  系统密钥文件cacert.pem和privkey.pem的绝对路径，需要将cacert.pem和privkey.pem放在同一目录下，路径的字符串长度不能超过114个字符
  * 返回：==-1成功，0失败==
  * 示例：见SslSvr.cpp和SslSvr.h ,SslCli.cpp和SslCli.h



## 附录A 基于框架的实用工具

### 阻塞管道CBlockPipe

BlockPipe.h

* public: static CBlockPipe * CBlockPipe::Create( int64_t timeout );
  * CBlockPipe对象的创建必须使用其静态成员方法Create进行，否则不能正常工作
* public: int CBlockPipe::Read( char * buf, uint32_t len );
* public: int CBlockPipe::Write( const char * buf, uint32_t len );
* public: int CBlockPipe::Close();
  * 对象使用结束之后，必须调用其成员方法Close进行释放，不能使用C++的delete操作
* 示例：MyBlockPipe.h和MyBlockPipe.cpp
* 成员方法CallFunc会返回成员数据m_value，但m_value值为负数被认为是无效的，只有在正数时才能将其返回。m_value的值在2秒钟后被设置为正数86，初始值是-1，所以，调用者就调用CallFunc的话，会阻塞2秒钟，直到m_value变正为止。

### TCP 流数据发送工具CStreamSender

StreamSender.h

* 该工具解决的问题

  * 对暂时未发送成功的数据进行有效地管理，确保能尽快发送出去
  * 当应用逻辑因为各种原因，出现多个线程同时向socket写数据时，保证数据的一致性。
  * 合理选择数据发送的时机，有效控制每次数据的发送量，避免系统效率受到影响

* public: static CStreamSender * CStreamSender::**Create**()

  * 功能：创建一个CStreamSender工具
  * 返回：成功返回CStreamSender指针，失败返回NULL
  * CStreamSender对象的删除，必须调用Close进行，不能使用delete

* public: int CStreamSender::**Attach**( CNetHandler *owner, CSock * stream );

  * 功能：把一个TCP socket绑定到发送工具上，之后，真正的数据发送将通过这个socket走。
  * 参数：owner  TCP socket的真正属主，stream是TCP socket
  * 返回：-1 0

* public: CSock * CStreamSender::**Detach**();

  * 功能：解绑
  * 返回：被解绑的TCP socket的指针
  * Attch和Detach操作的对象可以是CSockStream，CSslStream，CSimulatorStream三个中的任意一个，也就是说，CStreamSender可以接受普通的TCP socket，也可以接受加密socket，还可以进行限流量的测试

* public: int CStreamSender::**Put**( const char * buf, uint32_t len );

  * 功能：发送TCP数据

  * ~~参数、返回值略~~

  * 发送数据，光Put不够，因为CStreamSender并不具备向框架注册网络事件的能力，对于CStreamSender的属主来说，它还要为CStreamSender实现handle_ouput的回调功能，即，在用户的应用逻辑的代码中，还有实现这样一个函数

  	```C++
  int MyApp::handle_output( int handle ){
      if( /*其它各种情况*/ ){
          //deal with.
      }else if( handle == m_steam->GetHandle() && /*确实是有数据需要写*/ ){
          return CStreamSenderObj->handle_output( handle ) //关键是这一句。
      }
  }
  	```
  	
  * Put方法不像一般的数据发送接口，它**只返回成功或失败**，也就是说它不存在只发送部分成功的情况，失败了就要调用者等一会再进行尝试。

  * 如果调用者在某次调用Put发送n字节数据失败，那么就要调用下面这个接口，告诉CStreamSender：你什么时候能够发送n字节数据了，就请第一时间告诉我！

* public: void CStreamSender::**WaitBufferSize**( long receiver_id, uint32_t size_in_byte );

  * 功能：等待CStreamSender的缓冲区有size_in_byte字节的空闲，向这个ID发送消息MSG_BUFFER_SIZE，消息的附带信息attach中保存的是当前缓冲区的实际空闲大小，肯定大于等于size_in_byte
  * 参数：
    * receiver_id是调用者的对象ID
    * size_in_byte 等待的缓冲区空闲字节
  * 用户应该实现handle_message回调函数，处理MSG_BUFFER_SIZE消息
  * 注意：收到消息后，再调用Put仍可能失败，需要再次等待；最多收到一次MSG_BUFFER_SIZE，随后需重新调用

* 当CStreamSender中的TCP socket在发送数据的时候出现了异常的话，因为CStreamSender不是该socket的属主，不能越权处理，所以需要向属主进行报告，属主可以在Attach后就设置
  public: void CStreamSender::**WaitException**( long receiver_id );

  * 功能：设置socket出错时的异常报告对象，当CStreamSender中的socket出错时，向这个ID发送消息MSG_SOCK_EXCEPTION，消息无附带内容。
  * 参数：receiver_id为接收者的ID号
  * 接口只调用一次即可，若被多次调用，以最后一次为准。

* public: int StreamSender::**SetBufferSize**( uint32_t size_in_32K );

  * 功能：设置发送缓冲区的大小，
  * 参数：size_in_32K指定缓冲区大小，单位为32K。即 size_in_32K=2 即表示是64K
  * 返回：0成功，-1失败

* public: uint32_t StreamSender::**GetBufferSize**();

  * 功能：获取发送缓冲区的大小
  * 返回：以32K为单位的缓冲区大小

* public: int StreamSender::**Clear**();

  * 功能：清空发送缓冲区中目前还未发送出去的数据

  * 返回：0

    

示例：P2PSender.h和P2PSender.cpp

* 流媒体数据有其自身的特殊性，并不适合使用CStreamSender发送，流媒体数据见下文

  

###      流媒体数据发送工具CMediaBuffer和CMediaStreamSender

MeidaStreamSender.h

CMediaStreamSender类继承自CNetHandler

CStreamSender并不适合用于流媒体数据的发送，尤其是**直播流媒体**，当直播频道向多个接收者进行数据分发的时候，若使用CStreamSender，就会在网络不好的时候，**产生大量的数据拷贝**（数据发不出去，拷贝到CStreamSender的缓冲区中）；即使我们通过对缓冲区作出改进，解决了数据拷贝的问题，那一份数据在多个CStreamSender之间的一致性维护也会是一个很大的问题。另外，因为流媒体有自己固定的**发送频率**，在每一个时间点上触发数据的发送，如果再加上CStreamSender又触发一个发送点，势必造成相互间的竞争。

* 为解决数据拷贝和发送时机选择的两个问题，设计CMediaBuffer和CMediaStreamSender工具，专门用于流媒体数据的发送

* public: int CMediaBuffer::**Put**(CMediaPacket* packet, int len, int mark, int level );

  * 功能：向缓冲区输入流媒体数据
  * 参数：
    * packet  待输入的流媒体数据
    * len  本次输入的长度
    * mark  标记位，如果当前输入的数据包是当前帧的最后一个包，那么mark位置1，否则置0
    * level  当前数据的重要性，级别从LEVEL_HIGH到LEVEL_LOW，总共LEVEL_NUM个级别，目前头文件中定义了5个级别
  * level是以帧为单位，而不是以数据包为单位的，即当mark=1的那一个数据包被Put进来时指定的level为最后有效的当前帧的level
  * 当网络发生故障，或带宽不足时，发送端会依据level对部分帧进行丢弃处理，以尽量适应网络的状况，所以，对这个值的设置不当，会在这时严重影响流媒体的发送质量。
  * CMediaPacket是在MeidaStreamSender.h定义的虚基类，由用户继承和实现，建议对于packet，使用new和delete管理的堆内存即可，任何复杂的实现都不会带来丝毫额外的好处。

* public: void CMediaBuffer::**AddSender**( CMediaStreamSender * Sender );

  * 功能：向流媒体发送缓冲区添加一个发送者
  * 参数：Sender  发送者的指针

* public: CMediaStreamSender* CMediaBuffer::**DelSender**( CMediaStreamSender * Sender );

  * 功能：从流媒体发送缓冲区中删除一个发送者，参数Sender即为发送者的指针
  * 返回：被删除的发送者的指针。若未找到指定的发送者，返回NULL。

* public: static CMediaStreamSender * CMediaStreamSender::**Create**();

  * 功能：创建一个CMediaStreamSender对象
  * 返回：创建成功，返回一个CMediaStreamSender对象的指针，否则返回NULL。
  * 对CMediaStreamSender对象的删除，也必须通过调用它的方法Close进行，而不能使用C++的delete操作

* public: int CMediaStreamSender::**Attach**( CSock * stream );

  * 功能：为流媒体发送者绑定一个socket对象
  * 参数：stream即为待绑定的socket对象
  * 返回：0成功，-1失败

* public: CSock * CMediaStreamSender::**Detach**();

  * 功能：接绑定socket对象
  * 返回：被接绑定的socket对象指针

* Attach和Detach操作的TCP socket对象可以是CSockStream，CSslStream，CSimulatorStream三个中的任意一个，也就是说CStreamSender可以接受普通的TCP socket，也可以接受加密socket，还可以进行限流量的测试。同时，Attach和Detach操作的socket也可以是UDP socket，即CSockDgram。

* public: void CMediaStreamSender::**WaitException**( long receiver_id );

  * 同 CStreamSender::WaitException

* public: void CMediaStreamSender::**SendOtherData**( const char* data, int len );

  * 功能：在流媒体发送的socket上发送一些其它的数据，但不会对流媒体数据本身造成任何影响。
  * 参数：data为指向待发送数据的指针，len为数据的长度。

  

* CMediaBuffer中的流媒体数据除了通过CMediaStreamSender发向网络，还可能发向其它目的地，如本地磁盘，本地解码卡，本地播放器，及其它目前我们还未知的介质等等，统一接口如下，用户需要时自己实现：

  ```C++
  class CLevel2Buffer_IF
  {
  public:
      CLevel2Buffer_IF(): m_next(NULL){}
      virtual ~CLevel2Buffer_IF(){}
  public:
  virtual int Put( CMediaPacket* packet, int len, int mark, int level ) = 0;
  private:
      friend class CMediaBuffer; 
      CLevel2Buffer_IF* m_next;
  };
  ```

  用户只要继承该类，并实现Put函数，就可在Put函数中获得来自CMediaBuffer的流媒体数据，下面定义了添加和删除接口

* public void CMediaBuffer::**AddLevel2Buffer**( CLevel2Buffer_IF * buffer );

  * 功能：向CMediaBuffer缓冲区添加一个用户自定义的二级缓冲区
  * 参数：buffer  用户自定义的二级缓冲区的指针，添加完毕后，buffer即可获取流媒体数据

* public void CMediaBuffer::**DelLevel2Buffer**( CLevel2Buffer_IF * buffer );

  * 从CMediaBuffer缓冲区中删除一个用户自定义的二级缓冲区



* 示例：LiveSvr.h和LiveSvr.cpp是使用该工具编写的一个简单的直播流媒体服务器的例子。接口StartSvr启动一个本地侦听端口，提供流媒体服务，同时流媒体数据也马上开始产生（因为是直播，不会等到有客户端连接上来才会产生流媒体数据），每次产生的流媒体数据在hanndle_timeout中向CMediaBuffer进行Put即可。如果有连接请求进来，则在handle_input中接受连接，并生成一个CMediaStreamSender加入到CMediaBuffer即可。
* 问题：
  极端应用场景：某路流媒体数据，其帧率为1fps，而每帧的大小是1M甚至更大，它会每隔1秒调用一次CMediaBuffer的Put接口，一次Put进来的数据是1M甚至更多。根据上文的描述，我们知道CMediaBuffer的工作原理是每Put进来1帧进行一次发送，但是操作系统提供的协议栈的发送缓冲区是有限的，并且网络的带宽也是有限的，CMediaBuffer每次必然不能发送完，而在接下来的1秒钟时间内却处于休眠状态，未能将余下的数据发送出去，这样必然造成数据的不断堆积，直至崩溃。
* 解决上述问题，在未能完整发送完一帧数据的时候，在下一帧到来之前，能自动尝试将当前帧发送完成。为此，添加了如下接口
  public void CMediaBuffer::**EnableAutoSend**();
  * 功能：启动CMediaBuffer的自动发送功能，即在第n帧Put进来后，若未能一次发送出去，则在第n+1帧到来之前，CMediaBuffer会自动尝试以固定的周期将第n帧分多次发送出去，以不致造成数据的堆积
  * 该功能适用于帧率较低，而码流较大的特殊流媒体数据，会带来性能上的牺牲，所以一定要在仔细分析实际应用场景后，谨慎使用
  * 这是CMediaBuffer的一个属性，打开后无法关闭，故不提供相应的Disable接口。



### 流媒体数据接收工具CMediaStreamReceiver

MediaStreamReceiver.h

CMediaStreamReceiver提供对主动源（如网络接收，其数据是主动发送过来的）和被动源（如文件读取，其数据是需要使用者去读取的）的支持，新版本还增加了对流媒体数据的磁盘写入的驱动支持。

* select的方式来非阻塞地调用Recv，在流媒体的接收过程中其固定的传输频率，和较大的数据量，容易造成**频繁地大量调用Recv，而每次Recv收到的数据量却不大，这会大大增加CPU的负荷，进而影响整个系统的效率**。

* **对于被动源，需要解决一个读取时间点的问题，即不能读太快也不能读太慢**

* 流媒体写入磁盘时，如果基于某些原因，获得的数据不能立即写入磁盘，而是暂时存放于一个缓冲区中，由另一个线程把缓冲区中的数据写入磁盘。因此**执行写磁盘操作的时机和频率对整个系统的性能影响非常关键**

* CMediaStreamReceiver解决上述的问题：既不会霸占线程，影响整个系统的合理调度，也能大大降低CPU的负荷，提高整个系统的运行效率。对于文件等被动源，它让应用开发者从复杂的流控机制中摆脱出来。对于流媒体写磁盘，它帮助应用开发者选择合理的时间点去执行写操作，以降低系统负荷。

* public: **virtual** int CMediaStreamReceiver::**handle_stream_input**();

  * 功能：（针对主动源）由派生类具体实现，在函数中完成对socket上流媒体数据的接收
  * 返回：异常时需要返回小于0，之后回调不会再被调用，并且用户会接收到异常消息，否则返回大于等于0
  * 用户的socket必须是非阻塞的

* public: **virtual** int CMediaStreamReceiver::**handle_passive_input**();

  * 功能：（针对被动源）由派生类具体实现，在函数中完成对文件的流媒体数据的读取。
  * 返回：异常时需要返回小于0，之后回调不会再被调用，并且用户会接收到异常消息，否则需要返回当前媒体帧的pts，以供内部调度使用。

* public: **virtual** int CMediaStreamReceiver::**handle_stream_write**();

  * 功能：由派生类具体实现，在函数中完成对流媒体数据的磁盘写入操作
  * 返回：异常时需要返回小于0，之后回调不会再被调用，并且用户会接收到异常消息，否则返回大于等于0。

* public: void CMediaStreamReceiver::**SetReceiveMode**( receiver_mode mode );

  * 功能：设置本工具的工作模式

  * 参数：mode

    ```C++
    typedef enum{
        INITIATIVE_RECEIVER = 0,
        PASSIVE_RECEIVER = 1,
        DISK_WRITE = 2
    }receiver_mode;
    ```

* ~~public: void CMediaStreamReceiver::SetFrameRate( uint32_t fps );   已弃用~~

* public: void CMediaStreamReceiver::**WaitException**( long receiver_id );

  * 功能：设置异常消息的接收者，在socket接收数据发生错误或者读取文件错误时，会向这个接收者发送MSG_SOCK_EXCEPTION消息。参数id为异常消息的接收者

* public: void CMediaStreamReceiver::**StartReceive**(int64_t fast_start_ts = 0, float rate = 1.0);

  * 功能：开始对数据源的接收。
  * 参数：fast_start_ts和rate仅对被动源有效，即设置码流的快速启动时间和播放倍数

* public: void CMediaStreamReceiver::**StopReceive**();

  * 功能：停止接收装置。
  * 调用此接口后，handle_stream_input（）还可能被调用一次，用户需要等待并接收到消息MSG_TIMER_STOP，才能认为已经完成停止，在这之前的再次StartReceive是不合法的。

### 流媒体测试工具CSimulatorStream

SimulatorStream.h

* 提供了测试工具CSimulatorStream，它的使用其实是非常简单的，用于仿真某个固定带宽的网络环境，以便于在各种不同条件的网络环境下进行测试

* public: int CSimulatorStream::**SetBandWidth**( uint32_t mbps );

  * 功能：设置仿真的网络带宽
  * 参数：mbps  要设置的带宽值，单位是Mb/s。
  * 返回：0
  * 设置后，发送数据就会被限制在该带宽下

* public: int CSimulatorStream::**Send**( const char * buf, uint32_t len );

  public: int CSimulatorStream:: **WriteV**( const struct iovec *vector, int count );

  * 功能：发送流媒体数据
  * 参数：其中WriteV方法的调用参数同Linux的wirtev。
  * 返回：-1失败，0未发送需要下次再试，大于0表示发出去的长度

* public: void CSimulatorStream::**SetRealSocket**( CSockStream *stream );

  * 功能：对stream进行流量限制
  * 参数：stream是一个合法的已经建立了连接的tcp套接字
  * 需要特别注意的是，这是一个**用于测试用途**的工具，即使不作流量限制，它的**效率也低于普通**的CSockStream对象，所以千万**不要在正式**的程序中使用它。

### 定时器工具CNTimer

NTimer.h

* 在高并发网络服务应用，连接数多造成定时器也多，会频发触发一个定时器超时事件，这对应用产生的负荷还是相当大的，甚至有可能成为系统性能的瓶颈。
* 解决上述问题可以只开启一个定时器，超时后统一执行一个保活操作，这对网络框架来讲，只需要管理一个定时器，且每隔一定时间才触发一次，可以大大降低系统负荷

class CNTimer和class CNTimerEvent

* public: static CNTimer* CNTimer::**Create**( int64_t usec );
  * 功能：创建一个定时器容器。参数usec的单位为微秒，表示该容器的超时时间
  * 返回：一个指向已创建容器的指针
* public: void CNTimer::Close();
  * 功能：关闭通过Create创建的定时器容器
* public: int CNTimer::**PushTimerEvent**( CNTimerEvent* event );
  * 功能：往一个定时器容器中添加一个定时器事件对象。
  * 参数：event  一个事件对象，具体描述见下文
  * 返回：固定为0，表示成功
* CNTimerEvent是虚基类，需用户继承并实现自己的handle_timer_event函数
  public: **virtual** int CNTimerEvent::**handle_timer_event**();
  * 功能：由用户继承实现的虚函数，在定时器超时后被回调
  * 返回：无意义
* public: void CNTimerEvent::**Start**();
  * 功能：启动事件，让其可以被容器回调，否则容器超时后是不会回调
* public: void CNTimerEvent::**Stop**();
  * 功能：暂停一个事件，让其暂时不会被容器回调
* public: void CNTimerEvent::**Close**();
  * 功能：关闭一个定时器事件



## 附录B 基于NetFramework的功能组件的消息分配

| 组件名称             | 注释                   | 分配的消息范围  |
| -------------------- | ---------------------- | --------------- |
| NetFramework         | 网络框架基础库         | 0x0000——0x0FFFF |
| StreamSvr            | Rtsp/Rtp流媒体服务组件 | 0x1000——0x17FF  |
| StreamApp            | Rtsp/Rtp流媒体应用组件 | 0x1800——0x1FFF  |
| RPCClient            |                        | 0x2000——0x2FFF  |
| RPCServer            |                        | 0x3000——0x3FFF  |
| NetProtocol          | 网络通用协议组件       | 0x4000——0x8FFF  |
| WebServer            | Http服务器             | 0x9000——0x902F  |
| WebApp               | Http应用               | 0x9030——0x90FF  |
| CenterManagerClient  | 中心管理客户端模块     | 0x9100——0x91FF  |
| CenterManagerSupport | 中心管理本地支持模块   | 0x9200——0x92FF  |
| CenterManagerServer  | 中心管理服务端模块     | 0x9300——0x93FF  |
| SS、MTS              | 平台软件SS、MTS服务    | 0x9400——0x94FF  |



## 附录C 常见问题及解决办法

### 注册写事件，在handle_output中写是否合理

……

写事件只在两种情况下需要被注册：第一种情况是异步的Connect后，注册写事件用于判断Connect是否成功，第二种情况是当Send调用因为网络拥塞而不成功或部分成功时，把未发送出去的数据先作一个缓冲，再注册一个写事件，当网络恢复正常而可写时，写事件被触发，当把数据发出去后，立即注销写事件。正常情况下，有数据需要写时，立即调用Send发送出去，而不要注册写事件!

### handle_input要占用很长时间，如何保证后续响应

作为事件响应者的handle_input，响应完后（recv完数据），为了响应后续到达的数据，已经没有时间再去作进一步处理了，那就把事情交给别人处理，自己不管了就行了。只要在handle_input的实现中，在recv完数据后，使用异步消息，通知自己所在的对象（或者通知其它对象也可，看业务逻辑需要）有数据需要处理即可

`Notify( GetID()，MSG_PROCESS_REQUEST, (long)(bufPtr) );`

GetID()获取自己所在对象的ID号，MSG_PROCESS_REQUEST为自定义消息类型，bufPtr为指向接收到的数据的缓冲区。然后只要实现handle_message对消息进行处理即可

```C++
int MyObj::handle_message( long from_id, MSG_TYPE type, long attach )
{
	if( type == MSG_PROCESS_REQUEST ){
		unsigned char* bufPtr = (unsigned char *)attach;
		//process with bufPtr;
	}
}
```

这样，handle_input就能立即返回



### handle_input等套接字相关回调函数重入问题

handle_input、handle_output，对于一个套接字资源，是不允许并发操作的，一般是需要在编程时避免的。出于这个考虑，框架内已经将一个套接字的所有事件，进行串行化处理，即框架不会并发回调这个套接字的回调函数，使用者无需考虑并发情况。

下面情况是可能出现回调函数被并发调用，需要避免：

```C++
int MyObj::handle_input( int handle )
{
	…
	RemoveSock( sock );
	…
	RegisterSock( sock, XXX_MASK );
	…
}
```

即在回调中，RemoveSock删除了这个套接字相关的注册事件，然后又进行事件注册。在完成RegisterSock之后，若注册的事件条件满足，则回调函数可能马上被框架调用，此时会出现回调函数并发重入问题。若有类似需求，为避免重入，RemoveSock应修改为UnregisterSock 

### 何时删除注册的套接字问题

RemoveSock则会将套接字句柄等信息从框架中清空。所以，对一个即将关闭的套接字而言，应该在关闭前，使用RemoveSock清空框架内保存的关于此套接字的所有信息，而不能使用UnregisterSock。若只在关闭套接字前进行了UnregisterSock，则在进程内下一个套接字创建时，使用了相同的句柄，而导致逻辑混乱，需要避免。





---



