//
//  "$Id: StringParser.h 7672 2010-8-31 02:28:18Z huang_xiaojin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-8-31        huang_xiaojin 	Create
//									2010-8-31	
//
#ifndef __INCLUDED_USE_STRPARSER_H__
#define __INCLUDED_USE_STRPARSER_H__
#include "NetFramework/StrParser.h" 
class CStringParser
{
public:
	void ParseText();
private:
	void parse_rtsp_describe();
	void parse_rtsp_proxy_url();			
};

int StartStringParserTest();

#endif
