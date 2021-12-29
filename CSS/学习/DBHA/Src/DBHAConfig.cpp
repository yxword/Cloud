#include "DBHAConfig.h"
#include "Infra/PrintLog.h"
#include "Infra/Mutex.h"
#include "Infra/Guard.h"
#include "HALog.h"

#include <sstream>
#include <iostream>

DBHALOG_CLASSNAME( CDBHAConfig );

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

using namespace Dahua::EFS;
using namespace std;
using namespace Infra;

CDBHAConfig* CDBHAConfig::m_pSignle = NULL;

const std::string gDynamicCfgKeys[] = {
		managerLocalHeartbeatIP,
		managerLocalBusinessIP,
		managerPeerHeartbeatIP,
		managerPeerBusinessIP,
		managerVIP,
		managerGateway,
		managerVIPNetmask,
		managerVifName,
		managerIfName,
		managerHeartbeatIfName,
		expandHAFlag
};
const uint8_t DYNAMIC_KEYS_COUNT = sizeof(gDynamicCfgKeys) / sizeof(gDynamicCfgKeys[0]);

CDBHAConfig::CDBHAConfig()
    : m_configFile( "" )
    , m_configTool( NULL )
{
}

CDBHAConfig::CDBHAConfig( string file )
    : m_configFile( file )
    , m_configTool( NULL )
{
}


CDBHAConfig::~CDBHAConfig()
{
    if( m_configTool ) {
        delete m_configTool;
        m_configTool = NULL;
    }
}

CDBHAConfig* CDBHAConfig::instance( string file )
{
    static Infra::CMutex    mutex;
    if( file != "" ) {
        if( m_pSignle == NULL ) {
            Infra::CGuard guard( mutex );
            m_pSignle = new CDBHAConfig( file );
        }
        if ( !m_pSignle->init( file ) ) {
            DBHALOG_ERROR( "CDBHAConfig init failed." );
            return NULL;
        }
    }
    return m_pSignle;
}

bool CDBHAConfig::init( string &file )
{
    m_configTool = new CConfigTool();
    if ( !m_configTool ) {
        DBHALOG_ERROR( "new m_configTool fail." );
        return false;
    }

    if( false == ( m_configTool->loadConfigFile( file, managerDomain ) ) ) {
        DBHALOG_ERROR( "CConfigTool load config file %s failed!",
                      file.c_str() );
        return false;
    }
    return true;
}

bool CDBHAConfig::reload()
{
	std::vector<std::string> keys(gDynamicCfgKeys, gDynamicCfgKeys+DYNAMIC_KEYS_COUNT);
	if( !m_configTool->reloadConfig( managerDomain, keys ) ){
		DBHALOG_ERROR( "CConfigTool reload config file failed!");
		return false;
	}
	DBHALOG_INFO("reload success! new config:\n%s---\n", display().c_str());

	return true;
}

std::string CDBHAConfig::display()
{
	std::ostringstream oss;
	oss << managerLocalHeartbeatIP << ":" << getLocalHeartbeatIP() << std::endl
		<< managerLocalBusinessIP << ":" << getLocalBusinessIP() << std::endl
		<< managerPeerHeartbeatIP << ":" << getPeerHeartbeatIP() << std::endl
		<< managerPeerBusinessIP << ":" << getPeerBusinessIP() << std::endl
		<< managerVIP << ":" << getVIP() << std::endl
		<< managerVifName << ":" << getVifName() << std::endl
		<< managerIfName << ":" << getIfName() << std::endl
		<< managerHeartbeatIfName << ":" << getHeartbeatIfName() << std::endl
		<< expandHAFlag << ":" << serviceInExpandMode() << std::endl;

	return oss.str();
}

#define MANAGER_CONFIG_GET_INT(defaultValue, key)   {   \
	int32_t ret = defaultValue; \
	if( m_configTool && !m_configTool->getInt(managerDomain, key, ret) ){    \
	ret = defaultValue; \
	}   \
	return ret; \
}

#define MANAGER_CONFIG_GET_STRING(defaultValue, key)    {   \
	std::string ret = defaultValue; \
	if( m_configTool && !m_configTool->getString(managerDomain, key, ret) ){ \
	ret = defaultValue; \
	}   \
	return ret; \
}

string CDBHAConfig::getLocalHeartbeatIP()
{
	MANAGER_CONFIG_GET_STRING( managerLocalHeartbeatIPDefault, managerLocalHeartbeatIP );
}

bool CDBHAConfig::setLocalHeartbeatIP( const string& ip )
{
	return m_configTool->setString( managerDomain, managerLocalHeartbeatIP, ip );
}

string CDBHAConfig::getPeerHeartbeatIP()
{
	MANAGER_CONFIG_GET_STRING( managerPeerHeartbeatIPDefault, managerPeerHeartbeatIP );
}
bool CDBHAConfig::setPeerHeartbeatIP( const string& ip )
{
	return m_configTool->setString( managerDomain, managerPeerHeartbeatIP, ip );
}

string CDBHAConfig::getLocalBusinessIP()
{
	MANAGER_CONFIG_GET_STRING( managerLocalBusinessIPDefault, managerLocalBusinessIP );
}

bool CDBHAConfig::setLocalBusinessIP( const string& ip )
{
	return m_configTool->setString( managerDomain, managerLocalBusinessIP, ip );
}

string CDBHAConfig::getPeerBusinessIP()
{
	MANAGER_CONFIG_GET_STRING( managerPeerBusinessIPDefault, managerPeerBusinessIP );
}

bool CDBHAConfig::setPeerBusinessIP( const string& ip )
{
	return m_configTool->setString( managerDomain, managerPeerBusinessIP, ip );
}

string CDBHAConfig::getVIP()
{
	MANAGER_CONFIG_GET_STRING( managerVIPDefault, managerVIP );
}

bool CDBHAConfig::setVIP( const string& ip )
{
	return m_configTool->setString( managerDomain, managerVIP, ip );
}

string CDBHAConfig::getVIPNetmask()
{
	MANAGER_CONFIG_GET_STRING( managerVIPNetmaskDefault, managerVIPNetmask );
}

bool CDBHAConfig::setVIPNetmask( const string& mask )
{
	return m_configTool->setString( managerDomain, managerVIPNetmask, mask );
}

int32_t CDBHAConfig::getSyncPort()
{
	MANAGER_CONFIG_GET_INT( managerSyncPortDefault, managerSyncPort );
}

bool CDBHAConfig::setSyncPort( const int32_t port )
{
	return m_configTool->setInt( managerDomain, managerSyncPort, port );
}

int32_t CDBHAConfig::getThreadNum()
{
	MANAGER_CONFIG_GET_INT( managerThreadNumDefault, managerThreadNum );
}

bool CDBHAConfig::setThreadNum( int32_t& num )
{
	return m_configTool->setInt( managerDomain, managerThreadNum, num );
}

string CDBHAConfig::getMulticastIP()
{
	MANAGER_CONFIG_GET_STRING( managerMulticastIPDefault, managerMulticastIP );
}

bool CDBHAConfig::setMulticastIP( const string& ip )
{
	return m_configTool->setString( managerDomain, managerMulticastIP, ip );
}

int32_t  CDBHAConfig::getMulticastPort()
{
	MANAGER_CONFIG_GET_INT( managerMulticastPortDefault, managerMulticastPort );
}

bool  CDBHAConfig::setMulticastPort( const int32_t port )
{
	return m_configTool->setInt( managerDomain, managerMulticastPort, port );
}

string CDBHAConfig::getVifName()
{
	MANAGER_CONFIG_GET_STRING( managerVifNameDefault, managerVifName );
}

bool CDBHAConfig::setVifName( const string& vifName )
{
	return m_configTool->setString( managerDomain, managerVifName, vifName );
}

string CDBHAConfig::getIfName()
{
	MANAGER_CONFIG_GET_STRING( managerIfNameDefault, managerIfName );
}

std::string CDBHAConfig::getHeartbeatIfName()
{
	MANAGER_CONFIG_GET_STRING( managerHeartbeatIfNameDefault, managerHeartbeatIfName );
}

bool CDBHAConfig::setIfName( const string& ifName )
{
	return m_configTool->setString( managerDomain, managerIfName, ifName );
}

string CDBHAConfig::getServiceName()
{
	MANAGER_CONFIG_GET_STRING( managerServiceNameDefault, managerServiceName );
}

bool CDBHAConfig::setServiceName( const string& name )
{
	return m_configTool->setString( managerDomain, managerServiceName, name );
}

string CDBHAConfig::getUserDefined()
{
	MANAGER_CONFIG_GET_STRING( managerUserDefinedDefault, managerUserDefined );
}

bool CDBHAConfig::setUserDefined( const string& user )
{
	return m_configTool->setString( managerDomain, managerUserDefined, user );
}

int32_t CDBHAConfig::getLogLevel()
{
	MANAGER_CONFIG_GET_INT( DBHALogLevelDefault, DBHALogLevel );
}

int32_t CDBHAConfig::getCheckMysqlServicePeriod()
{
	MANAGER_CONFIG_GET_INT( checkMysqlServicePeriodDefault, checkMysqlServicePeriod );
}

int32_t CDBHAConfig::getMaxServiceStopNum()
{
	MANAGER_CONFIG_GET_INT(maxMysqlServiceStopStateNumDefault,maxMysqlServiceStopStateNum);
}

// 暂时写次数，后续变成时间，1.5*6
int32_t CDBHAConfig::getMaxSlaveSyncWithMasterNum()
{
	MANAGER_CONFIG_GET_INT( maxSlaveSyncWithMasterNumDefault, maxSlaveSyncWithMasterNum );
}

//int32_t CDBHAConfig::getMaxMysqlSlaveWaitMasterUpNum()
//{
//	MANAGER_CONFIG_GET_INT( maxMysqlSlaveWaitMasterUpNumDefault, maxMysqlSlaveWaitMasterUpNum );
//}

std::string CDBHAConfig::getSetServerIDScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlSetServerIDScriptDefault, mysqlSetServerIDScript );
}

std::string CDBHAConfig::getMasterScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlMasterScriptDefault, mysqlMasterScript );
}

std::string CDBHAConfig::getSlaveScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlSlaveScriptDefault, mysqlSlaveScript );
}

std::string CDBHAConfig::getBinlogInfoScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlGetBinlogScriptDefault, mysqlGetBinlogScript );
}

std::string CDBHAConfig::getSlaveStateScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlGetSlaveStateScriptDefault, mysqlGetSlaveStateScript );
}

std::string CDBHAConfig::getMasterBackupPath()
{
	MANAGER_CONFIG_GET_STRING( mysqlMasterBackupPathDefault, mysqlMasterBackupPath );
}

std::string CDBHAConfig::getSlaveBackupPath()
{
	MANAGER_CONFIG_GET_STRING( mysqlSlaveBackupPathDefault, mysqlSlaveBackupPath );
}

std::string CDBHAConfig::getBackupScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlBackupScriptDefault, mysqlBackupScript );
}

std::string CDBHAConfig::getRecoveryScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlRecoveryScriptDefault, mysqlRecoveryScript );
}

std::string CDBHAConfig::opMasterBackupFile()
{
	MANAGER_CONFIG_GET_STRING( mysqlMasterBackupFileDefault, mysqlMasterBackupFile );
}

std::string CDBHAConfig::getSlaveRecoverDumpFileName()
{
	MANAGER_CONFIG_GET_STRING(slaveRecoveryDumpFileNameDefault,slaveRecoveryDumpFileName);
}

bool CDBHAConfig::setSlaveIsRecoveringFlag( const int32_t flag )
{
	return m_configTool->setInt( managerDomain, slaveIsRecovering, flag );
}

bool CDBHAConfig::setSlaveRecoveringBinlog( std::string binlogFile, int64_t binlogPos, std::string md5sum )
{
	if( !m_configTool->setString( managerDomain, slaveRecoverBinlogFile, binlogFile ) )
		return false;

	if( !m_configTool->setDouble( managerDomain, slaveRecoverBinlogPos, binlogPos ) )
		return false;

	if( !m_configTool->setString( managerDomain, slaveRecoverMd5sum, md5sum ) )
		return false;

	return true;
}

bool CDBHAConfig::getSlaveRecoveringBinlog( std::string &binlogFile, int64_t &binlogPos, std::string &md5sum )
{
	if( m_configTool && !m_configTool->getString(managerDomain, slaveRecoverBinlogFile, binlogFile) ){
		return false;
	}

	double retVal = 0;
	if( m_configTool && !m_configTool->getDouble(managerDomain, slaveRecoverBinlogPos, retVal) ){
		return false;
	}
	binlogPos = retVal;

	if( m_configTool && !m_configTool->getString(managerDomain, slaveRecoverMd5sum, md5sum) ){
		return false;
	}
	return true;
}

int32_t CDBHAConfig::getSlaveIsRecoveringFlag()
{
	MANAGER_CONFIG_GET_INT(slaveIsRecoveringDefault,slaveIsRecovering);
}

int32_t CDBHAConfig::getSlaveLastRecoveryResultFlag()
{
	MANAGER_CONFIG_GET_INT(slaveLastRecoveryResultDefault,slaveLastRecoveryResult);
}

bool CDBHAConfig::setSlaveLastRecoveryResultFlag( const int32_t flag)
{
	return m_configTool->setInt( managerDomain,slaveLastRecoveryResult, flag );
}

bool CDBHAConfig::getSingleMode()
{
	string localBusinessIP = getLocalBusinessIP();
	string peerBusinessIP = getPeerBusinessIP();
	if ((localBusinessIP == peerBusinessIP) ||(localBusinessIP != "" && peerBusinessIP == "")){
		//DBHALOG_WARN( "this is the single mode" );
		return true;
	}else{
		return false;
	}
}

int32_t CDBHAConfig::getCheckSyncFlag()
{
	MANAGER_CONFIG_GET_INT( checkSyncWithMasterDefault, checkSyncWithMaster );
}

int32_t CDBHAConfig::getDumpFileValidNum()
{
	MANAGER_CONFIG_GET_INT( dumpFileValidNumDefault, dumpFileValidNum );
}

std::string CDBHAConfig::getCheckSyncState()
{
	MANAGER_CONFIG_GET_STRING( mysqlCheckSyncStateDefault, mysqlCheckSyncState );
}

int32_t CDBHAConfig::getBinlogFileDelayNum()
{
	MANAGER_CONFIG_GET_INT( binlogFileDelayNumDefault, binlogFileDelayNum );
}

bool CDBHAConfig::serviceInExpandMode()
{
	int32_t ret = expandHAFlagDefault;
	if( m_configTool && !m_configTool->getInt(managerDomain, expandHAFlag, ret) ){
		ret = expandHAFlagDefault;
	}

	return ret > 0 ? true : false;
}

bool CDBHAConfig::serviceLeavingExpandMode()
{
	return m_configTool->setInt( managerDomain, expandHAFlag, -1 );
}

std::string CDBHAConfig::getMasterStateScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlCheckMasterStateScriptDefault, mysqlCheckMasterStateScript );
}

std::string CDBHAConfig::getMysqlServerScript()
{
	MANAGER_CONFIG_GET_STRING( mysqlServerScriptDefault, mysqlServerScript );
}

int32_t CDBHAConfig::getDiffPosHighThreshold()
{
	MANAGER_CONFIG_GET_INT( sameBinlogDiffPosHighThresholdDefault, sameBinlogDiffPosHighThreshold );
}

int32_t CDBHAConfig::getDiffPosLowThreshold()
{
	MANAGER_CONFIG_GET_INT( sameBinlogDiffPosLowThresholdDefault, sameBinlogDiffPosLowThreshold );
}

int32_t CDBHAConfig::getWaitToBecomeMasterTime()
{
	MANAGER_CONFIG_GET_INT( waitToBecomeMasterDefault, waitToBecomeMaster );
}

}// DBHA
}// VideoCloud
}// Dahua
