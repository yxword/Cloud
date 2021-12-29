#include "CommonFun.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

namespace Dahua {
namespace VideoCloud {
namespace DBHA{

DBHALOG_CLASSNAME( CommonFun );

bool CommonFun::executeScript(std::string scriptPath, std::string para)
{
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "/bin/sh  %s  %s ", scriptPath.c_str(), para.c_str()) < 0 ) {
		DBHALOG_ERROR( "executeScript failed!\n" );
		return false;
	}
	// shell()返回0，代表执行成功
	return 0 == Dahua::EFS::os::shell( NULL, std::string( command ) );
}

/*
 * 不带参数的脚本执行，脚本内状态码返回给上层
 * @return [0...n] 脚本exit $?
 *         -1 os::shell()执行出错
 */
int CommonFun::executeScript(std::string scriptPath)
{
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "/bin/sh  %s", scriptPath.c_str()) < 0 ) {
		DBHALOG_ERROR( "executeScript failed!\n" );
		return false;
	}

	return Dahua::EFS::os::shell( NULL, std::string( command ) );
}

//带参数的脚本执行，echo打印信息返回给上层
bool CommonFun::getExecuteScriptValue(std::string scriptPath, std::string para, std::string& retValue)
{
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "/bin/sh  %s  %s ", scriptPath.c_str(), para.c_str()) < 0 ) {
		DBHALOG_ERROR( "getExecuteScriptValue failed!\n" );
		return false;
	}

	std::stringstream ret_value;
	int32_t ret = Dahua::EFS::os::shell( &ret_value, std::string( command ) );
	if ( ret ) {
		DBHALOG_ERROR( "excute script failed,command: %s!\n", command );
		return false;
	}
	retValue = ret_value.str();
	//DBHALOG_INFO( "excute script succeeded, %s %d command: %s retValue:%s!\n", __FUNCTION__, __LINE__, command,
	//	retValue.c_str());
	return true;
}

int32_t CommonFun::strSplit( const std::string& src, char separator, StringVec& strs )
{
	if( src == "" )
		return 0;
	int32_t begin = -1, end = -1;
	int32_t num = 0;
	do{
		begin = end;
		end = src.find( separator, begin + 1 );
		num++;
		if( end == ( int32_t )std::string::npos ){
			strs.push_back( src.substr( begin + 1 ) );
			break;
		}
		strs.push_back( src.substr( begin + 1, end - begin - 1 ) );
	}while( end != ( int32_t )std::string::npos );
	return num;
}

//获取文件长度
uint64_t CommonFun::getFileLen(std::string fileName)
{
	FILE *fp = NULL;
	fp=fopen(fileName.c_str(),"rb");
	if( fp == NULL ){
		DBHALOG_ERROR( "get file %s length failed!\n", fileName.c_str() );
		return 0;
	}

	fseek(fp,0,SEEK_END);
	uint64_t size=ftell(fp);
	fclose(fp);
	return size;
}

bool CommonFun::setBinlogInfo(std::string binlogInfo, std::string& binlogName, int32_t& binlogPos)
{
	binlogName = "";
	binlogPos = 0;

	/* binglog返回信息格式是filename pos */
	if ( binlogInfo == "" ){
		DBHALOG_ERROR("binloginfo is empty");
		return false;
	}
	std::string bin_log_info = binlogInfo;
	StringVec itemvec;
	strSplit( bin_log_info, ' ', itemvec );
	// itemvec如果不包含两个元素，那后续操作就会越界
	// 根据当前脚本实现，是可能存在越界情况的
	if( itemvec.size() != 2 ){
		DBHALOG_ERROR("binloginfo is invalid");
		return false;
	}
	std::string binlog_filename = itemvec[0];
	std::string pos = itemvec[1];
	int32_t bin_pos = atoi(pos.c_str());
	//DBHALOG_INFO("init bin log file %s, file pos %d",binlog_filename.c_str(),bin_pos);
	binlogName = binlog_filename;
	binlogPos = bin_pos;
	return true;
}

} //DBHA
} //VideoCloud
} //Dahua
