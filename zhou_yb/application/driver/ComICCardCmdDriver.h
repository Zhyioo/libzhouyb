//========================================================= 
/**@file ComICCardCmdDriver.h
 * @brief 三合一读卡器驱动逻辑
 * 
 * @date 2016-05-04   21:09:37
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BP8903CMDDRIVER_H_
#define _LIBZHOUYB_BP8903CMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"
#include "ICCardCmdDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 三合一命令驱动
class ComICCardCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>, 
    public CommandCollection,
    public InterruptBehavior,
    public RefObject
{
protected:
    LastErrInvoker _objErr;
    LastErrExtractor _icErr;
    LastErrExtractor _lastErr;
    LoggerInvoker _logInvoker;
    InterruptInvoker _interruptInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapter;
    ComICCardCmdAdapter _cmdAdapter;
    ComContactICCardDevAdapter _contactIC;
    ComContactlessICCardDevAdapter _contactlessIC;
    ComPsamICCardDevAdapter _psam1;
    ComPsamICCardDevAdapter _psam2;
    ICCardCmdDriver _icDriver;

    LC_CMD_METHOD(RemovePSAM)
    {
        _icDriver.ReleaseDevice();
        _icDriver.SelectDevice(_contactlessIC);
        _icDriver.SelectDevice(_contactIC);
        return true;
    }
    LC_CMD_METHOD(InsertPSAM)
    {
        _icDriver.SelectDevice(_psam1, "0");
        _icDriver.SelectDevice(_psam2, "1");
        return true;
    }
public:
    ComICCardCmdDriver()
    {
        _interruptInvoker.select(_icDriver);

        _icErr.IsFormatMSG = false;
        _icErr.IsLayerMSG = false;
        _icErr.Select(_contactIC, "CONTACT");
        _icErr.Select(_contactlessIC, "CONTACTLESS");
        _icErr.Select(_psam1, "PSAM1");
        _icErr.Select(_psam2, "PSAM2");

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_cmdAdapter, "IC_CMD");
        _lastErr.Select(_icErr);
        _lastErr.Select(_icDriver, "IC_DRV");
        _objErr.Invoke(_lasterr, _errinfo);
        _lastErr.Select(_objErr);

        _contactIC.SelectDevice(_cmdAdapter);
        _contactlessIC.SelectDevice(_cmdAdapter);
        _psam1.SelectDevice(_cmdAdapter);
        _psam2.SelectDevice(_cmdAdapter);

        _icDriver.SelectDevice(_contactlessIC);
        _icDriver.SelectDevice(_contactIC);
        _icDriver.SelectDevice(_psam1, "0");
        _icDriver.SelectDevice(_psam2, "1");

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _cmdAdapter, _contactIC, _contactlessIC, _psam1, _psam2;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType>::select(_adapter),
            _cmdAdapter;

        list<Ref<ComplexCommand> > cmds = _icDriver.GetCommand("");
        Registe(cmds);

        /* 自动寻卡时不需要检测PSAM卡 */
        Ref<ComplexCommand> waitForCardCmd = Registe("WaitForCard");
        Ref<Command> insertPsamCmd = Command::Make((*this), &ComICCardCmdDriver::InsertPSAM);
        Ref<Command> removePsamCmd = Command::Make((*this), &ComICCardCmdDriver::RemovePSAM);
        waitForCardCmd->PreBind(removePsamCmd);
        waitForCardCmd->Bind(insertPsamCmd);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_INTERRUPT(_interruptInvoker);
    /// 当前激活的IC卡
    inline Ref<IICCardDevice> ActiveIC()
    {
        return _icDriver.ActiveIC();
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BP8903CMDDRIVER_H_
//========================================================= 