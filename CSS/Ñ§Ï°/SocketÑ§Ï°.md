# Socket

## 字节序

字节序分为 `大端字节序`和`小端字节序` 由于大多数PC采用小端字节序(高位存在高地址处), 所以小端字节序又称为主机字节序

为了防止不同机器字节序不同导致的错乱问题. 规定传输的时候统一为 大端字节序(网络字节序). 这样主机会根据自己的情况决定 - 是否转换接收到的数据的字节序

* x86系列CPU都是little-endian的字节序

* 网络字节顺序采用big-endian排序方式

### 示例

```C++
主机字节序与网络字节序转换
第一步  127   .     0     .     0     .    1         把IP地址每一部分转换为8位的二进制数。
第二步 01111111   00000000   00000000   00000001   =  2130706433  （主机字节序）
然后把上面的四部分二进制数从右往左按部分重新排列，那就变为：
第三步 00000001   00000000   00000000  01111111    =  16777343    （网络字节序）
```

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

## API

### 基础连接

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

### TCP发送和接收数据

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

### UDP发送和接收数据

```C++
#include <sys/types.h>
#include <sys/socket.h>
// 由于UDP不保存状态, 每次发送数据都需要 加入目标地址.
// 不过recvfrom和sendto 也可以用于 面向STREAM的连接, 这样可以省略发送和接收端的socket地址
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
ssize_t sendto(int sockfd, const void* buf, size_t len, ing flags, const struct sockaddr* dest_addr, socklen_t addrlen);
```



## 练习

 [SocketEcho.rar](SocketEcho.rar) 

* 编译

  make clean && make all

  rm -rf Socket_Demo/* && scp -P 7298 -r 10.35.30.5:/root/Socket_Demo .



* Todo 

  输入缓冲区清空之前输入的字符







