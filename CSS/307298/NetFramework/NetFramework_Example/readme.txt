###################################################################################
Demo��������ͷ�ļ�Ĭ��Ϊ: Project/Include/ Ŀ¼
Demo����������Ĭ��Ϊ    : Project/Lib/m64x86/ Ŀ¼
###################################################################################

/* ��������
 */
 
make clean; make demo=httpsvr
make clean; make demo=tcpsvr  	# �ڵ�ǰĿ¼������Example�ļ�������ϵͳ���� tcp������
make clean; make demo=tcpcli	# �ڵ�ǰĿ¼������Example�ļ�������ϵͳ���� tcp�ͻ���

make clean; make demo=udpsvr	# �ڵ�ǰĿ¼������test�ļ�������udp����˲���
make clean;make demo=udpsvr ssl=true	# �ڵ�ǰĿ¼������Example�ļ�������dtls����˲���

make clean;make demo=udpcli	# �ڵ�ǰĿ¼������Example�ļ�������udp�ͻ��˲���
make clean;make demo=udpcli ssl=true 	# �ڵ�ǰĿ¼������Example�ļ�������dtls�ͻ��˲���

make clean; make demo=sslclidemo	# ssl ͬ���첽���Կͻ��ˣ�SSL�������ܲ��� (QPS, ָ����������ʱ)
make clean; make demo=sslsvrdemo	# ssl ͬ���첽���Է���ˣ�SSL�������ܲ��� (QPS, ָ����������ʱ)

make clean; make demo=nbsslsvr	# �򵥷�����ssl�����
make clean; make demo=nbsslcli	# �򵥷�����ssl�ͻ���


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


#make clean; make ==>�ڵ�ǰĿ¼������test�ļ�������������ģ��ĵ�����Ľӿڲ���

*/
#####################################################################################
