//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_PRINT_LOG_H__
#define __INFRA3_PRINT_LOG_H__

#include <stdio.h>
#include "Defs.h"
#include "IntTypes.h"
#include "Function.h"


/// \defgroup PrintLog PrintLog
/// ��ӡ��Ϣ��¼�ȵȡ�
/// ��ӡ��Ϣ��trace, debug, info, warn, error, fatal��6�����𣬼�¼����ʽ������
/// ֱ�Ӵ�ӡ����׼�����Ҳ���Լ�¼���ļ����߷��͵�Զ����־��������Ŀǰֻ֧��
/// ��һ�ַ�ʽ��ÿ�д�ӡ��Ϣǰ�Զ�������ʱ��ʹ�ӡ���͵��ַ���������"12:54:12|info"
/// @{

namespace Dahua{
namespace Infra{


/// ��ӡ����ص���������
/// ����ΪҪ��ӡ���ַ���
typedef TFunction1<void, char const*> LogPrinterProc;

/// ���ô�ӡ������ص�����
/// \param [in] printer ����ص�����, Ϊ��ʱ���ô�ӡ�������׼����豸
/// \return �ɹ�����0, ʧ�ܷ���-1
int INFRA_API setLogPrinter(LogPrinterProc printer);

/// ���ô�ӡ�ļ��𣬸��ڸü���Ĵ�ӡ�������
void INFRA_API setPrintLogLevel(int level);

/// ��ӡ���ÿ�������¼������Ϣ������nLevel����ֱ�ӵ���
/// \return ���ش�ӡ���ֽ���
int INFRA_API logLibName(int nLevel, const char* libName, const char* fmt, ...);

/// ��¼������Ϣ������6����ֱ�ӵ��ã�����ʵ��debgf��
/// \return ���ش�ӡ���ֽ���
int INFRA_API logDebug(const char* fmt, ...);

/// ��¼������Ϣ������5����ֱ�ӵ��ã�����ʵ��tracef��
/// \return ���ش�ӡ���ֽ���
int INFRA_API logTrace(const char* fmt, ...);

/// ��¼������Ϣ������4����ֱ�ӵ��ã�����ʵ��infof��
/// \return ���ش�ӡ���ֽ���
int INFRA_API logInfo(const char* fmt, ...);

/// ��¼������Ϣ������3����ֱ�ӵ��ã�����ʵ��warnf��
/// \return ���ش�ӡ���ֽ���
int INFRA_API logWarn(const char* fmt, ...);

/// ��¼������Ϣ������2����ֱ�ӵ��ã�����ʵ��errorf��
/// \return ���ش�ӡ���ֽ���
int INFRA_API logError(const char* fmt, ...);

/// ��¼����������Ϣ������1����ֱ�ӵ��ã�����ʵ��fatalf��
/// \return ���ش�ӡ���ֽ���
int INFRA_API logFatal(const char* fmt, ...);

/// ��16���ƺ��ַ���Ӧ�ķ�ʽ��ӡ�ڴ�����
/// \param pdat ���ݻ���ָ��
/// \param length ���ݻ��峤��
void INFRA_API dumpHex(uint8_t* pdat, size_t length = 512);


} // namespace Infra
} // namespace Dahua

//////////////////////////////////////////////////////////////////////////
// print message, controlled by messsage level

#undef debugf
#undef tracef
#undef infof
#undef warnf
#undef errorf
#undef fatalf
#undef tracepoint

#ifdef __GNUC__ // perform complete xxxxf macro replacement
#	ifdef LOG_LIB_NAME
#		define debugf(format, ...) logLibName(6, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define tracef(format, ...) logLibName(5, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define infof(format, ...) logLibName(4, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define warnf(format, ...) logLibName(3, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define errorf(format, ...) logLibName(2, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define fatalf(format, ...) logLibName(1, LOG_LIB_NAME, format, ## __VA_ARGS__)
#	else
#		define debugf(format, ...) logDebug(format, ## __VA_ARGS__)
#		define tracef(format, ...) logTrace(format, ## __VA_ARGS__)
#		define infof(format, ...) logInfo(format, ## __VA_ARGS__)
#		define warnf(format, ...) logWarn(format, ## __VA_ARGS__)
#		define errorf(format, ...) logError(format, ## __VA_ARGS__)
#		define fatalf(format, ...) logFatal(format, ## __VA_ARGS__)
#	endif
#else
#	define debugf logDebug
#	define tracef logTrace
#	define infof logInfo
#	define warnf logWarn
#	define errorf logError
#	define fatalf logFatal
#endif

#define tracepoint()  logDebug("tracepoint: %s, %d.\n",__FILE__,__LINE__)

/// @} end of group

#endif //__INFRA_PRINT_LOG_H__

