//
//  "$Id: StrParser.h 34542 2011-09-08 01:53:01Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7		huan_xiaojin	Create

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_STR_PARSER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_STR_PARSER_H__

#include "Infra/IntTypes.h"
#include <stdio.h>
#include <ctype.h>
#include "NDefs.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

//字符串分析工具，主要用于基于文本的网络协议的分析。
class NETFRAMEWORK_API CStrParser{
	CStrParser& operator=( CStrParser const& other );
	CStrParser( CStrParser const& other );
public:
	//返回值，类的所有成员的出错返回值，都是下列枚举量之一。
	enum{
		STR_NOERR = 0,								//无错误，正确返回。
		STR_ERR = -1,									//发生错误。
		STR_TOO_LONG = -2,						//字符串太长，超出能够处理的范围。
		STR_NO_ENOUGH_SPACE = -3,			//缓冲区空间不够，不能把要求的文本串拷出来。
		STR_NOT_FOUND = -4,						//未发现要求的文本。
		STR_REACH_END = -5,						//到达文本串的末尾。
		STR_INVALID_TYPE = -6					//在获取指定类型的数据时，发现格式不符。
	};
public:
	//在strstr的二进制兼容版本。在一个内存串中寻找子内存串。
	//参数：w_buf 母内存串地址， w_len 母内存串大小， sub_buf 子内存串地址，sub_len 子内存串大小
	static unsigned char* MemMem( unsigned char* w_buf, int w_len, 
											unsigned char* sub_buf, int sub_len);
	//MemMemCase为MemMem的区分大小写版本
	static unsigned char* MemMemCase( unsigned char* w_buf, int w_len, 
											unsigned char* sub_buf, int sub_len );
public: //构造及初始化函数。
	//构造一个空的字符串分析器。
	CStrParser();
	//构造一个字符串分析器。
	//参数： text: 传入的文本，以'\0‘作为文本的结束符。
	CStrParser(const char *text);
	//构造一个字符串分析器。
	//参数：text，指向传入的文本的指针，len为text的有效长度。
	CStrParser(const char *text, uint32_t len );
	//为分析器传入一个待分析文本。
	//参数：text，传入的文本，以'\0'作为文本的结束符。
	//返回值：0表示成功，-1表示失败。
	int32_t Attach(const char *text);
	//为分析器传入一个待分析文本。
	//参数：text，指向传入的文本的指针，len为text的有效长度。
	//返回值：0表示成功，-1表示失败。
	int32_t Attach(const char *text, uint32_t len );
	~CStrParser();
	
	//获取当前的分析器所拥有的整个文本串。
	//返回值： 指向文本串的指针。
	const char * GetString();
	//获取当前的整个文本串的长度。
	//返回值： 文本串的长度。
	int32_t GetStringLen(); 
	//获取出错状态
	//返回值：STR_XX的枚举类型。
	int     GetStatus();
public:
	//返回文本串中当前指针所在处的文本行的长度。
	//用处：在文本分析或生成的过程中，需要往当前正在处理的行再添加内容或修改，删除内容，
	//此时，需要当前行的长度。
	//返回值：当前文本行的长度。
	uint32_t GetCurLinelength();
	//获取整个文本串的文本行数。
	//返回值： 文本行数。
	int32_t GetLineNum();
	//获取当前指针所在位置的偏移。
	//返回值：指针的偏移位置。 
	int32_t GetCurOffset();
	//获取指针当前所在处的字符。
	char    GetCurChar();
	//复位整个文本串。
	//指针，当前行等变量均回到行首第一个字符。
	void ResetAll(); 
	//复位指针到当前行首。
	void ResetToLineHead(); 
public:
	//定位字符串，区分大小写
	//返回值：定位到的偏移量。 返回 < 0 表示出错
	int32_t LocateString(const char *subString);
	//定位字符串，不区分大小写
	//返回值：定位到的偏移量。 返回 < 0 表示出错	
	int32_t LocateStringCase(const char *subString); 
	//行内定位字符串，区分大小写
	//返回值：定位到的偏移量。 返回 < 0 表示出错	
	int32_t LocateStringInLine(const char *subString); 
	//行内定位字符串，不区分大小写
	//返回值：定位到的偏移量。 返回 < 0 表示出错		
	int32_t LocateStringInLineCase(const char *subString);
	//定位当前指针到指定偏移处
	//返回值：定位到的偏移量。没有出错情况，超过有效范围的，自动调整到有效范围内
	int32_t LocateOffset(int32_t offset);
public:
	//在文本的当前位置，与subString进行比较。
	//返回值：0 相等， 非0 不相等
	int32_t Compare(const char *subString);
	//在文本的当前位置，与subString进行比较，不区分大小写
	//返回值：0 相等， 非0 不相等
	int32_t CompareCase(const char *subString);
public:
	//期望遇见字符stopChar，并消费掉该字符
	//返回值：指针的偏移量。 返回 < 0 表非期望字符，指针偏移不变
	int32_t Expect(char stopChar); 
	//期望遇见行属结束符，并消费掉该字符
	//返回值：指针的偏移量。 返回 < 0 表非期望字符，指针偏移不变	
	int32_t ExpectEOL(); 
public:
	//从当前位置获取一个数据类型。值超过范围的，直接做强制类型转化。
	//返回值：根据GetStatus()判断调用是否成功。为STR_NOERR则为正确。其他值视为失败。
	int16_t ConsumeInt16();
	int32_t ConsumeInt32();
	int64_t ConsumeInt64();
	uint16_t ConsumeUint16(); 
	uint32_t ConsumeUint32(); 
	uint64_t ConsumeUint64();
	uint32_t ConsumeHexUint32();
	float ConsumeFloat();
	double ConsumeDouble();
public:
enum {
	ADDR_UNKNOWN = -1,
	ADDR_IPV4 = 4,
	ADDR_IPV6 = 6,
};
	//从当前位置获取一个地址。如果addr_type不为NULL，将返回相应的地址类型。
	//addr_type: 返回 ADDR_IPV4 表示IPv4地址类型，返回 ADDR_IPV6 表示IPv6地址类型。其他值表示非法地址。
	//返回值： < 0 失败， > 0 地址长度
	int32_t ConsumeIpAddr(char *buf, uint32_t max_len,int* addr_type = NULL);
public:	
	//消费，直到碰到一个非可见字符。
	//返回值： < 0 失败， > 0 消费字符的长度
	int32_t ConsumeWord(char *buf=NULL, uint32_t max_len=0);
	//消费掉从当前位置开始的所有空白字符。
	//返回值：>=0 消费字符长度
	int32_t ConsumeWhitespace();
	//消费掉行内从当前位置开始的所有空白字符，但不包括本行换行符
	//返回值：>=0 消费字符长度	
	int32_t ConsumeWhitespaceInLine();
	//消费掉指定长度字符
	//返回值：>= 0 消费字符长度
	int32_t ConsumeLength(uint32_t numBytes, char *buf=NULL, uint32_t max_len=0); 
	
	//消费掉以end_masks中的字符结尾的句子。
	//其中end_marks为以'\0'结尾的数组。如 ['\t','\n','\0']。
	//则函数消费到'\t'或'\n'的下一个字符。
	//返回的字符不包含end_marks字符
	//返回< 0: 失败； 
	//返回>=0: 消费的句子的长度
	int32_t ConsumeSentence(const char *end_marks, char *buf = NULL, uint32_t max_len = 0);
	
	//返回当前位置前一个字符。指针偏移不变
	char CheckPrevChar();
public:
	//以下一组函数功能：一直消费，直到某个位置为止。
	//返回< 0: 失败， 指针偏移不变
	//返回>=0: 消费的字符数 
	//消费直到数字为止
	int32_t ConsumeUntilDigit(); 
	//消费直到16进制数字为止	
	int32_t ConsumeUntilHexDigit();
	//消费直到下一行
	int32_t ConsumeUntilNextLine();
	//消费直到指定字符StopChar
	int32_t ConsumeUntil(char StopChar);
	//消费直到遇见以字符为下标的inMask表内值为非0的字符
	int32_t ConsumeUntil(const uint8_t *inMask);
	//行内消费直到指定字符StopChar	
	int32_t ConsumeUntilinLine(char StopChar);
	//行内消费直到遇见以字符为下标的inMask表内值为非0的字符	
	int32_t ConsumeUntilinLine(const uint8_t *inMask);
	
	//消费直到StopChar字符，并把这个字符也消费掉。
	//返回< 0: 失败， 指针偏移不变
	//返回>=0: 消费的字符数 	
	int32_t GetThru(char StopChar);
	//消费到EOL字符，并把这个字符也消费掉。
	//返回< 0: 失败， 指针偏移不变
	//返回>=0: 消费的字符数 		
	int32_t GetThruEOL();
	//获取以head开始的行，不包含换行符
	//返回< 0: 失败， 指针偏移不变
	//返回>0: 该行长度，不包含换行符 
	int32_t GetLineWithHead(const char *head, char *buf = NULL, uint32_t max_len = 0);
	//获取以head开始的行的数量
	//返回>=0: 符合条件行的数量 	
	int32_t GetLineNumWithHead(const char *head);
	
public:
	//替换。当前位置指针不变。以下几个函数可能会重新申请内存，故外部如果持有数组指针，需更新
	//返回< 0: 失败
	//返回>=0: 成功 
	int32_t ReplaceInt16(int16_t newVal);
	int32_t ReplaceInt32(int32_t newVal);
	int32_t ReplaceInt64(int64_t newVal);
	int32_t ReplaceUint16(uint16_t newVal);
	int32_t ReplaceUint32(uint32_t newVal);
	int32_t ReplaceUint64(uint64_t newVal);
	int32_t ReplaceFloat(float newVal);
	//将当前double字符替换为指定double数字。如果当前偏移非合法double字符，视为失败
	//返回< 0: 失败
	//返回>=0: 成功
	int32_t ReplaceDouble(double newVal); 
	//将当前IP地址替换为指定字符串。如果当前偏移非合法IP地址，视为失败
	//返回< 0: 失败
	//返回>=0: 成功
	int32_t ReplaceIpAddr(char * newVal); 
	//从当前位置替换指定长度len个字符。val不指定，或长度< len时，以空格填充。
	//返回值：替换后指针偏移
	int32_t ReplaceLength(int32_t len, char *val = NULL);
	
	//往CStrParser内追加字符串
	//返回值：> 0 成功，追加后字符串长度
	//        < 0 失败
	int32_t Append(const char *val);
	
private:
	struct Internal;    
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_STR_PARSER_H__
