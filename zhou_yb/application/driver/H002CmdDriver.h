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
#include "CommonCmdDriver.h"

#include "ICCardCmdDriver.h"
#include "PSBC_PinCmdDriver.h"
#include "ComICCardCmdDriver.h"
#include "IDCardCmdDriver.h"
#include "PBOC_CmdDriver.h"

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
    public InterruptBehavior,
    public CommandDriver<TArgParser>
{
protected:
    LoggerInvoker _logInvoker;
    InterruptInvoker _interruptInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapterInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _icErr;
    LastErrExtractor _appErr;
    LastErrExtractor _lastErr;

    PSBC_PinCmdDriver _pinDriver;
    ComICCardCmdDriver _icDriver;
    PBOC_CmdDriver _pbocDriver;
    IDCardCmdDriver _idDriver;

    LC_CMD_METHOD(UpdateIC)
    {
        _pbocDriver.SelectDevice(_icDriver.ActiveIC());
        return true;
    }
public:
    H002CmdDriver()
    {
        _icErr.IsFormatMSG = false;
        _icErr.IsLayerMSG = true;
        _icErr.Select(_icDriver, "IC_DRV");
        _icErr.Select(_pbocDriver, "PBOC");

        _appErr.IsFormatMSG = false;
        _appErr.IsLayerMSG = false;
        _appErr.Select(_pinDriver, "PIN");
        _appErr.Select(_icErr);
        _appErr.Select(_idDriver, "ID");

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = false;
        _lastErr.Select(_appErr, "MOD");
        _objErr.Invoke(_lasterr, _errinfo);
        _lastErr.Select(_objErr);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _pinDriver, _icDriver, _pbocDriver, _idDriver;
        select_helper<InterruptInvoker::SelecterType>::select(_interruptInvoker),
            _icDriver, _idDriver;
        select_helper<DevAdapterInvoker<IInteractiveTrans> >::select(_adapterInvoker),
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

        cmds = _pbocDriver.GetCommand("");
        _PreBind(cmds, gateCmd, "I");
        _Bind(cmds, gateCmd, "A");
        Registe(cmds);

        cmds = _idDriver.GetCommand("");
        _PreBind(cmds, gateCmd, "I");
        _Bind(cmds, gateCmd, "S");
        Registe(cmds);

        Ref<Command> setIcCmd = Command::Make((*this), &H002CmdDriver::UpdateIC);
        Ref<ComplexCommand> complexCmd = LookUp("WaitForCard");
        if(!complexCmd.IsNull())
            complexCmd->Bind(setIcCmd);

        complexCmd = LookUp("SelectSLOT");
        if(!complexCmd.IsNull())
            complexCmd->Bind(setIcCmd);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapterInvoker);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_INTERRUPT(_interruptInvoker);
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