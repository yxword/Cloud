//
//  "$Id$"
//
//  Copyright ( c )1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	数据库监控程序主线程
//	Revisions:		2016-12-3 #
//

#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <signal.h>
#include <time.h>

#include "Infra/PrintLog.h"
#include "Infra/Semaphore.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "Common/InterruptHandler.h"
#include "DBHAConfig.h"
#include "DBHAServer.h"
#include "HALog.h"
#include "../version.h"

using namespace Dahua::VideoCloud;

DBHALOG_CLASSNAME( DBHAMain );

const std::string DBHACfgPath = "../Config/DBHA.conf";// 数据库监控程序配置文件路径

Dahua::Infra::CSemaphore gExitSem;
bool g_need_restart = false;
bool g_exit=false;
// 打印帮助信息
void displayHelp( void )
{
    Dahua::Infra::infof( "Usage: " );
    Dahua::Infra::infof( " <-<H,h> program help info>" );
    Dahua::Infra::infof( " <-<V,v> program version> " );
}

// 显示软件版本号、SVN号和编译时间
void displayVersion( void )
{
	
    std::cout << "bin name:         " << VIDEOCLOUD_DBHA_VER_NAME << std::endl;
    std::cout << "version:          " << VIDEOCLOUD_DBHA_VER_PRODUCT_VISION << std::endl;
    std::cout << "svn revision:     " << VIDEOCLOUD_DBHA_VER_SVN << std::endl;
    std::cout << "compile time:     " << __DATE__ << " " << __TIME__ << std::endl;
    return;
	
}

// 返回值为0表示程序继续运行，非0表示程序停止。
int dealArg( int argc, char **argv )
{
    int ch = getopt( argc, argv, "HhVv" );
    if( ch != -1 ) {
        switch( ch ) {
            case 'H':
            case 'h': {
                // 显示帮助信息
                displayHelp();
                return -1;
            }
            case 'V':
            case 'v': {
                // 显示版本信息
                displayVersion();
                return -1;
            }
            default: {
                // 显示帮助信息
                displayHelp();
                return -1;
            }
        }
    }
    return 0;
}

void handleSignal( int sig )
{
    if( (SIGQUIT == sig) || (SIGINT == sig) || (SIGTERM == sig) )
    {
		g_exit=true;
       //DBHALOG_INFO( "the interrupt sig is :%d",sig );
        gExitSem.post();
    }

	if( SIGUSR1 == sig ){
		g_need_restart = true;
		gExitSem.post();
	}

}

void waitExit()
{
    gExitSem.pend();
}

void setSignalHandler()
{
	struct sigaction act;
	act.sa_handler = handleSignal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL); // 为了实现动态加载配置文件
}


int main( int argc, char **argv )
{

	g_exit=false;
    // 打印版本信息
    // 处理程序的参数
    if( dealArg( argc, argv ) != 0 ) {
        return 0;
    }
    //load 配置文件
	Dahua::VideoCloud::DBHA::CDBHAConfig::instance( DBHACfgPath );
    //设置日志级别
    Dahua::EFS::CLog* log = Dahua::VideoCloud::vcsLogInstance();
#ifdef NDEBUG
	log->init("../Log/", "DBHA", DBHA::g_DBHAConfig->getLogLevel(), false, true, 30, 30);
#else
	log->init("../Log/", "DBHA", DBHA::g_DBHAConfig->getLogLevel(), false, true, 50, 30);
#endif
	log->setNoCache();

	// 选举日志独立放在一个文件中,日志级别必须为Info
	Dahua::EFS::CLog* elect_log = Dahua::VideoCloud::electLogInstance();
	elect_log->init( "../Log/", "DBHA_election", Dahua::EFS::CLog::logLevelInfo, false, true, 30, 5, 512 << 10, false );
	elect_log->setNoCache();
    //注册中断信号
    Dahua::EFS::dahuaBacktraceInit( );

    //打印版本信息
    displayVersion();

    /* 设置信号处理函数 */
    setSignalHandler();

    // 线程数是否从配置文件中读取
    if( Dahua::NetFramework::CNetThread::CreateThreadPool( \
            ( uint32_t )Dahua::VideoCloud::DBHA::g_DBHAConfig->getThreadNum() ) > 0 )
    {
        DBHALOG_ERROR( "CreateThreadPool error!" );
        Dahua::NetFramework::CNetThread::DestroyThreadPool();
        return -1;
    }

    Dahua::VideoCloud::DBHA::CDBHAServer* dbha_server = new Dahua::VideoCloud::DBHA::CDBHAServer();
	if( !dbha_server ){
		Dahua::NetFramework::CNetThread::DestroyThreadPool();
		DBHALOG_ERROR("new CDBHAServer fail.");
		return -1;
	}
	//todo调用白总脚本删除之前未备份完成的脚本
	//启动前先执行本地恢复
    //if( DBHA::g_DBHAConfig->getSlaveIsRecoveringFlag() )
	//{
	//	DBHALOG_INFO("last recover has been interrupt,recover resume now,wait...\n");
	//	dbha_server->createMysqlRecoverThread();
	//	waitExit();
	//	dbha_server->destroyMysqlRecoverThread();
//	}

    // 在恢复过程中直接退出，则不需要往下执行了
	if( g_exit ){
		delete dbha_server;
		dbha_server=NULL;
		Dahua::NetFramework::CNetThread::DestroyThreadPool();
		return 0;
	}

	//启动服务
    if ( dbha_server->init() ) {
        bool ret = dbha_server->startup();
		if (ret){
			do {
				g_need_restart = false;
				waitExit();
				if ( g_need_restart ) {
					sleep( 2 );
					if ( !dbha_server->reloadDBHAConfig() ) {
						//出错继续让用户设置HA信息
						DBHALOG_ERROR( "reload db ha config fail." );
						continue;
					}
				}
			} while( g_need_restart );
		}
    }


	if (dbha_server)
	{
		dbha_server->stop();
		delete dbha_server;
		dbha_server=NULL;
	}
    Dahua::NetFramework::CNetThread::DestroyThreadPool();

    DBHALOG_INFO( "exit to DBHA main" );
    return 0;
}
