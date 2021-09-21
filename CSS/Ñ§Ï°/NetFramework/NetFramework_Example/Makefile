#
# makefile for x86
#

# ����Ŀ¼
TOPDIR = ..

# makefile���֣����ɵ�obj�ļ������.obj_$(NAME)����
# ���Խ���ÿ���ű�ȡ��ͬ�����֣�������̬��Ͷ�̬����Թ���һ������
NAME = x86-test
# ������빤��ǰ׺�����ű�ʾx86
CROSS = 
# debug����release
DEBUG = 1
# ������Ķ��������ͣ�Ŀǰ֧�־�̬��static����̬��shared����ִ�г���exec
BINARY = exec
# ����������
TARGET ?= Example
# ���������·��
INSTALL_PATH = .

# Դ�ļ�Ŀ¼·��
SRCPATH = ./
# ��Ҫ�����Դ�ļ���Ŀ¼��������Դ�ļ�Ŀ¼·��֮�£�

#DHTS��
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



# ͷ�ļ�Ŀ¼
INCDIR = ./ \
		$(SRCSUBDIR)/ \
		./Include
		#-I../Include -I./ -I../../../include/ -I../
	
# һЩ�Զ���ı���flags��debug��release��һЩѡ������common.mk�а�����
CFLAGS += -Wall #-fprofile-arcs  -ftest-coverage
CFLAGS += -g

# һЩ�Զ��������flags����̬�ⲻ�ã�
LDFLAGS = -ldl -lrt
ifeq ($(cov), 1)
CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGs += -lgcov
endif

# ���ӵĿ�·������̬�ⲻ�ã�
# LIBDIR = \
# 	$(TOPDIR)/Lib/m64x86/
# 	#$(TOPDIR)/Test/Lib/m64x86 
LIBDIR = \
	./Lib
	#$(TOPDIR)/Test/Lib/m64x86 



#CFLAGS += -DLocalLiveTest	
	
# ���ӵĿ⣨��̬�ⲻ�ã�
#LIBS = gtest StreamSvr Stream NetFramework Infra pthread NetAutoAdaptor Opensource Dhts
#LIBS = Infra Opensource NetFramework_ssl
# LIBS = Infra Opensource Dhts
LIBS = Infra64 pthread

# �û��׸ı�Ŀ�
# USER_LIBS = $(TOPDIR)/Lib/m64x86/libNetFramework.a
USER_LIBS = ./Lib/libNetFramework64.a

# ����ʵ�ʴ���
include common.mk
