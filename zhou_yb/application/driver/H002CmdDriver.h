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

    LC_ComToCCID_CmdAdapter _lcCmdAdapter;
    LC_ReaderDevAdapter _lcAdapter;

    BoolInterrupter _interrupter;

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

        _Registe("SendCommand", (*this), &H002CmdDriver::SendCommand);
        _Registe("Interrupt", (*this), &H002CmdDriver::Interrupt);
        _Registe("InterrupterReset", (*this), &H002CmdDriver::InterrupterReset);

        Ref<Command> gateCmd = Command::Make((*this), &H002CmdDriver::SendCommand);
        string gateKey = "SEND";
        string magArg = Arg(gateKey, "1B 24 41");
        list<Ref<ComplexCommand> > cmds = _pinDriver.GetCommand("");
        _PreBind(cmds, gateCmd, Arg(gateKey, "1B 24 4B").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _icDriver.GetCommand("");
        _PreBind(cmds, gateCmd, Arg(gateKey, "1B 24 49").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _pbocDriver.GetCommand("");
        _PreBind(cmds, gateCmd, Arg(gateKey, "1B 24 49").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _idDriver.GetCommand("");
        _PreBind(cmds, gateCmd, Arg(gateKey, "1B 24 53").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
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
    /// 设置信息标签表
    void SetTransTable(const ushort* infoTable, const ushort* amountTable, const ushort* detailTable)
    {
        PBOC_CmdDriver::InformationTABLE = infoTable;
        PBOC_CmdDriver::AmountTABLE = amountTable;
        PBOC_CmdDriver::DetailTABLE = detailTable;
    }
    /// 设置TLV转换函数
    void SetTlvConverter(PbocTlvConverter::fpTlvAnsConvert ansConverter)
    {
        PBOC_CmdDriver::TlvConverter = ansConverter;
    }
    /// 检查是否适配设备
    LC_CMD_METHOD(IsValid)
    {
        return BaseDevAdapterBehavior<IInteractiveTrans>::IsValid();
    }
    /**
     * @brief 发送控制指令
     * @date 2016-06-09 10:52
     * 
     * @param [in] Send : string 需要发送的指令
     */
    LC_CMD_METHOD(SendCommand)
    {
        string send = arg["SEND"].To<string>();
        ByteBuilder cmd(8);
        DevCommand::FromAscii(send.c_str(), cmd);
        if(!cmd.IsEmpty())
        {
            return _pDev->Write(cmd);
        }
        return true;
    }
    /// 中断读取过程
    LC_CMD_METHOD(Interrupt)
    {
        if(_interrupter.Interrupt())
        {
            while(!_interrupter.IsBreakout())
            {
                Timer::Wait(DEV_OPERATOR_INTERVAL);
            }
        }
        return true;
    }
    /// 重置中断状态
    LC_CMD_METHOD(InterrupterReset)
    {
        return _interrupter.Reset();
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_H002JNICMDDRIVER_H_
//========================================================= 