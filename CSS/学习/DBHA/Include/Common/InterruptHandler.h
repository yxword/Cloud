#ifndef __INCLUDE_DAHUA_EFS_INTERRUPT_HANDLER_H__
#define __INCLUDE_DAHUA_EFS_INTERRUPT_HANDLER_H__

namespace Dahua {
namespace EFS {

    class CLog;

    void dahuaBacktraceInit(CLog* log=NULL);

#ifndef WIN32
    typedef Infra::TFunction1<void, int> SignalTrigger;

    // 注意: USR1和USR2支持注册多个trigger.
    // Exit只支持注册一个trigger.
    void registerSigUSR1Trigger(SignalTrigger trigger);
    void registerSigUSR2Trigger(SignalTrigger trigger);
    void registerExitSigTrigger(SignalTrigger trigger);      //SIGINT SIGQUIT SIGTERM
#endif

}//end of namespace EFS
}//end of name space Dahua

#endif

