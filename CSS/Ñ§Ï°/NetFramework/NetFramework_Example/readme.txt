###################################################################################
Demo编译依赖头文件默认为: Project/Include/ 目录
Demo编译依赖库默认为    : Project/Lib/m64x86/ 目录
###################################################################################

/* 编译命令
 */
 
make clean; make demo=httpsvr
make clean; make demo=tcpsvr  	# 在当前目录下生成Example文件，用于系统测试 tcp服务器
make clean; make demo=tcpcli	# 在当前目录下生成Example文件，用于系统测试 tcp客户端

make clean; make demo=udpsvr	# 在当前目录下生成test文件，用于udp服务端测试
make clean;make demo=udpsvr ssl=true	# 在当前目录下生成Example文件，用于dtls服务端测试

make clean;make demo=udpcli	# 在当前目录下生成Example文件，用于udp客户端测试
make clean;make demo=udpcli ssl=true 	# 在当前目录下生成Example文件，用于dtls客户端测试

make clean; make demo=sslclidemo	# ssl 同步异步测试客户端，SSL并发性能测试 (QPS, 指定连接数耗时)
make clean; make demo=sslsvrdemo	# ssl 同步异步测试服务端，SSL并发性能测试 (QPS, 指定连接数耗时)

make clean; make demo=nbsslsvr	# 简单非阻塞ssl服务端
make clean; make demo=nbsslcli	# 简单非阻塞ssl客户端


make clean;make demo=echosvr
make clean;make demo=echocli


make clean;make demo=timer

make clean;make demo=message

make clean;make demo=write_event

make clean;make demo=livesvr
make clean;make demo=livecli


make clean;make demo=p2p_sender
make clean;make demo=str_parser
make clean;make demo=file_copy


make clean;make demo=sock_protect_svr
make clean;make demo=sock_protect_cli


make clean;make demo=dhtssvr
make clean;make demo=dhtscli


#make clean; make ==>在当前目录下生成test文件，用于所有子模块的单个类的接口测试

*/
#####################################################################################
