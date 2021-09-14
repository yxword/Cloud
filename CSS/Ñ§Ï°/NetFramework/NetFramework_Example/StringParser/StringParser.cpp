#include <string.h>
#include "StringParser.h"

using namespace Dahua::NetFramework;

void CStringParser::ParseText()
{
	parse_rtsp_describe();
	parse_rtsp_proxy_url();
}

void CStringParser::parse_rtsp_describe()
{
	int cseq = 0;
	char content[256];
	memset(content, 0 , sizeof(content));
	const char *sample = "DESCRIBE rtsp://10.19.2.80:8554/h264.mp4 RTSP/1.0\r\n"
						 "CSeq: 8\r\n"
						 "Accept: application/sdp\r\n";	
	CStrParser parser;	
	parser.Attach( sample );
	//获取CSeq
	if( parser.LocateStringCase("CSeq:") >= 0 ){
		parser.ConsumeUntilDigit();
		cseq =  parser.ConsumeInt32();
		printf("CSeq: %d \n",cseq);
	}
	parser.ResetAll();
	if( parser.LocateStringCase("rtsp://") > 0 &&
		parser.ConsumeLength(8) > 0 &&
		parser.LocateString("/") > 0 && 
		parser.ConsumeLength(1) > 0 )
	{
		//获取rtsp请求内容"h264.mp4"
		parser.ConsumeWord(content, sizeof(content) - 1 );
		printf("%s\n",content);
	}
}

void CStringParser::parse_rtsp_proxy_url()
{
	char *sample = "rtsp://10.19.2.27/proxy=10.6.5.51:8554?/dev/sda6/00008970.00000000.dav";
	char dest_url[256];
	memset(dest_url, 0 , sizeof(dest_url));
	CStrParser parser(sample);
	char ip_buf[128];
	char port[64];
	memset(ip_buf,0,sizeof(ip_buf));
	memset(port, 0, sizeof(port));
	snprintf(port,sizeof(port),"%d",554);
	if( parser.LocateString("proxy=") >= 0 ){
		parser.ConsumeLength(strlen("proxy="));
		if( parser.ConsumeIpAddr(ip_buf, sizeof(ip_buf)-1) >= 0 )
		{
			if( parser.Expect(':') > 0 ) //如果指定端口号，则解析出端口号
				parser.ConsumeSentence("?",port,sizeof(port)-1);
			parser.Expect('?');
			snprintf( dest_url, sizeof(dest_url)-1, "rtsp://%s:%s/%s", 
								ip_buf, port, parser.GetString()+ parser.GetCurOffset());
			printf("dest_url: %s \n",dest_url);
		}
	}else
		printf("invalid rtsp proxy url \n");		
	
}


int StartStringParserTest()
{
	CStringParser parser;
	parser.ParseText();
}