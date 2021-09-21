#
# makefile for x86
#

# 工程目录
TOPDIR = ..

# makefile名字，生成的obj文件会放在.obj_$(NAME)下面
# 所以建议每个脚本取不同的名字，不过静态库和动态库可以共用一个名字
NAME = x86-test
# 交叉编译工具前缀，空着表示x86
CROSS = 
# debug还是release
DEBUG = 1
# 编译出的二进制类型，目前支持静态库static，动态库shared，可执行程序exec
BINARY = exec
# 二进制名字
TARGET ?= Example
# 二进制输出路径
INSTALL_PATH = .

# 源文件目录路径
SRCPATH = ./
# 需要编译的源文件子目录（必须在源文件目录路径之下）

#DHTS库
DHTSLIB =

ifeq ($(demo), tcpsvr)
SRCSUBDIR += TcpServer
CFLAGS += -DTCP_SVR 
else ifeq ($(demo), tcpcli)
SRCSUBDIR += TcpClient
CFLAGS += -DTCP_CLI

else ifeq ($(demo), udpsvr)
ifeq ($(ssl), false)
SRCSUBDIR += UdpServer
CFLAGS += -DUDP_SVR
else ifeq ($(ssl), true)
SRCSUBDIR += UdpSslServer
CFLAGS += -DSSL_UDP_SVR
endif 
else ifeq ($(demo), udpcli)
ifeq ($(ssl), false)
SRCSUBDIR += UdpClient
CFLAGS += -DUDP_CLI
else ifeq ($(ssl), true)
SRCSUBDIR += UdpSslClient
CFLAGS += -DSSL_UDP_CLI
endif 

else ifeq ($(demo), multicast)
SRCSUBDIR += Multicast
CFLAGS += -DMULTICAST

else ifeq ($(demo), packet)
SRCSUBDIR += Packet
CFLAGS += -DPACKET

else ifeq ($(demo), udppacket)
SRCSUBDIR += UdpPacket
CFLAGS += -DUDPPACKET

else ifeq ($(demo), sslsvr)
SRCSUBDIR += SslServer
CFLAGS += -DSSL_SVR -D_USE_OPENSSL

else ifeq ($(demo), sslcli)
SRCSUBDIR += SslClient 
CFLAGS += -DSSL_CLI -D_USE_OPENSSL

else ifeq ($(demo),echosvr)
SRCSUBDIR += EchoServer
CFLAGS += -DECHO_SVR

else ifeq ($(demo), echocli)
SRCSUBDIR += EchoClient
CFLAGS += -DECHO_CLI

else ifeq ($(demo), timer)
SRCSUBDIR += Timer
CFLAGS += -DTIMER

else ifeq ($(demo), message)
SRCSUBDIR += Message
CFLAGS += -DMESSAGE

else ifeq ($(demo), write_event)
SRCSUBDIR += WriteEvent
CFLAGS += -DWRITE_EVENT

else ifeq ($(demo), livesvr)
SRCSUBDIR += LiveServer
CFLAGS += -DLIVE_SVR

else ifeq ($(demo), livecli)
SRCSUBDIR += LiveClient
CFLAGS += -DLIVE_CLI

else ifeq ($(demo), p2p_sender)
SRCSUBDIR += P2pSender
CFLAGS += -DP2P_SENDER

else ifeq ($(demo), str_parser)
SRCSUBDIR += StringParser
CFLAGS += -DSTR_PARSER

else ifeq ($(demo), file_copy)
SRCSUBDIR += FileCopy
CFLAGS += -DFILE_COPY

else ifeq ($(demo), private_pool)
SRCSUBDIR += PrivatePool
CFLAGS += -DPRIVATE_POOL

else ifeq ($(demo), sock_protect_svr)
SRCSUBDIR += SockProtectServer
CFLAGS += -DSOCK_PROTECT_SVR

else ifeq ($(demo), sock_protect_cli)
SRCSUBDIR += SockProtectClient
CFLAGS += -DSOCK_PROTECT_CLI

else ifeq ($(demo), dhtssvr)
DHTSLIB = Dhts
SRCSUBDIR += DhtsServer
CFLAGS += -DDHTS_SVR -D_USE_DHTS

else ifeq ($(demo), dhtscli)
DHTSLIB = Dhts
SRCSUBDIR += DhtsClient
CFLAGS += -DDHTS_CLIENT -D_USE_DHTS

else ifeq ($(demo), nbsslsvr)
SRCSUBDIR += NbSslServer
CFLAGS += -DNB_SSL_SVR -D_USE_OPENSSL

else ifeq ($(demo), nbsslcli)
SRCSUBDIR += NbSslClient
CFLAGS += -DNB_SSL_CLI -D_USE_OPENSSL

else ifeq ($(demo), sslsvrdemo)
SRCSUBDIR += SslSvrDemo
CFLAGS += -DSSL_SVR_DEMO -D_USE_OPENSSL

else ifeq ($(demo), sslclidemo)
SRCSUBDIR += SslCliDemo
CFLAGS += -DSSL_CLI_DEMO -D_USE_OPENSSL

else ifeq ($(demo), httpsvr)
SRCSUBDIR += HttpServer
CFLAGS += -DHTTP_SVR

endif



# 头文件目录
INCDIR = ./ \
		$(SRCSUBDIR) \
		$(TOPDIR)/Include
		#-I../Include -I./ -I../../../include/ -I../
	
# 一些自定义的编译flags（debug和release的一些选项已在common.mk中包含）
CFLAGS += -Wall #-fprofile-arcs  -ftest-coverage
CFLAGS += -g

# 一些自定义的链接flags（静态库不用）
LDFLAGS = -ldl -lrt
ifeq ($(cov), 1)
CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGs += -lgcov
endif

# 链接的库路径（静态库不用）
LIBDIR = \
	$(TOPDIR)/Lib/m64x86/
	#$(TOPDIR)/Test/Lib/m64x86 


#CFLAGS += -DLocalLiveTest	
	
# 链接的库（静态库不用）
#LIBS = gtest StreamSvr Stream NetFramework Infra pthread NetAutoAdaptor Opensource Dhts
#LIBS = Infra Opensource NetFramework_ssl
LIBS = Infra Opensource Dhts

# 用户易改变的库
USER_LIBS = $(TOPDIR)/Lib/m64x86/libNetFramework.a

# 进入实质处理
include common.mk
