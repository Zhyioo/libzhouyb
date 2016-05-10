//========================================================= 
/**@file H002CmdDriver.h
 * @brief H002 Android下驱动
 * 
 * @date 2016-04-26   17:15:16
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_H002JNICMDDRIVER_H_
#define _LIBZHOUYB_H002JNICMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"
using zhou_yb::application::driver::CommandDriver;

#include "ICCardCmdDriver.h"
#include "PSBC_PinCmdDriver.h"
#include "ComICCardCmdDriver.h"
#include "IDCardCmdDriver.h"

#include "../../extension/ability/SplitArgParser.h"
using zhou_yb::extension::ability::SplitArgParser;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// H002驱动
template<class TArgParser>
class H002CmdDriver :
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public CommandDriver<TArgParser>
{
protected:
    LoggerInvoker _logInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapter;
    /// 上次错误信息
    LastErrExtractor _lastErr;
    /// 密码键盘
    PSBC_PinCmdDriver _pinDriver;
    /// IC卡
    ComICCardCmdDriver _icDriver;
    /// 二代证
    IDCardCmdDriver _idDriver;
public:
    H002CmdDriver()
    {
        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = false;
        _lastErr.Select(_pinDriver);
        _lastErr.Select(_icDriver);
        _lastErr.Select(_idDriver);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _pinDriver, _icDriver, _idDriver;
        select_helper<DevAdapterInvoker<IInteractiveTrans> >::select(_adapter),
            _pinDriver, _icDriver, _idDriver;

        _Registe("LedControl", (*this), &H002CmdDriver::LedControl);
        _Registe("ToGate", (*this), &H002CmdDriver::ToGate);

        Ref<Command> gateCmd = Command::Make((*this), &H002CmdDriver::ToGate);
        list<Ref<ComplexCommand> > cmds = _pinDriver.GetCommand("");
        _PreBind(cmds, gateCmd, "K");
        _Bind(cmds, gateCmd, "A");
        Registe(cmds);

        cmds = _icDriver.GetCommand("");
        _PreBind(cmds, gateCmd, "I");
        _Bind(cmds, gateCmd, "A");
        Registe(cmds);

        cmds = _idDriver.GetCommand("");
        _PreBind(cmds, gateCmd, "I");
        _Bind(cmds, gateCmd, "S");
        Registe(cmds);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    /// LED
    LC_CMD_METHOD(LedControl)
    {
        return true;
    }
    /// 转口
    LC_CMD_METHOD(ToGate)
    {
        string gate = arg["Gate"].To<string>();
        byte gateCmd[3] = { 0x1B, 0x24, 0x00 };
        if(gate.length() > 0)
        {
            gateCmd[2] = _get_upper(gate[0]);
            return _pDev->Write(ByteArray(gateCmd, 3));
        }
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_H002JNICMDDRIVER_H_
//========================================================= 