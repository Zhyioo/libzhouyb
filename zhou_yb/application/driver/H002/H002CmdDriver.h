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
#include "../CommonCmdDriver.h"

#include "../ICCardCmdDriver.h"
#include "../PSBC_PinCmdDriver.h"
#include "../ComICCardCmdDriver.h"
#include "../IDCardCmdDriver.h"
#include "../PBOC_CmdDriver.h"
#include "../LC_CmdDriver.h"
#include "../MagneticCmdDriver.h"

 #include "../../lc/inc/LC_ComStreamCmdAdapter.h"
using zhou_yb::application::lc::LC_ComStreamCmdAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// H002驱动
template<class TArgParser>
class H002CmdDriver :
    public DevAdapterBehavior<IInteractiveTrans>,
    public InterruptBehavior,
    public CommandCollection,
    public RefObject
{
public:
    typedef TArgParser ArgParserType;
protected:
    LoggerInvoker _logInvoker;
    InterruptInvoker _interruptInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapterInvoker;

    LastErrInvoker _objErr;
    LastErrExtractor _icErr;
    LastErrExtractor _appErr;
    LastErrExtractor _lastErr;

    MagneticCmdDriver _magDriver;
    PSBC_PinCmdDriver _pinDriver;
    ComICCardCmdDriver _icDriver;
    PBOC_CmdDriver _pbocDriver;
    IDCardCmdDriver _idDriver;
    LC_CmdDriver _lcDriver;

    LC_ComStreamCmdAdapter _cmdAdapter;

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
        _icErr.Select(_icDriver, "ICC");
        _icErr.Select(_pbocDriver, "PBOC");

        _appErr.IsFormatMSG = false;
        _appErr.IsLayerMSG = false;
        _appErr.Select(_magDriver, "MAG");
        _appErr.Select(_pinDriver, "PIN");
        _appErr.Select(_icErr, "IC");
        _appErr.Select(_idDriver, "ID");
        _appErr.Select(_lcDriver, "LC");

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_appErr, "MOD");
        _objErr.Invoke(this->_lasterr, this->_errinfo);
        _lastErr.Select(_objErr);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _magDriver, _pinDriver, _icDriver, _pbocDriver, _idDriver, _lcDriver;
        select_helper<InterruptInvoker::SelecterType>::select(_interruptInvoker),
            _magDriver, _icDriver, _idDriver;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType >::select(_adapterInvoker),
            _magDriver, _pinDriver, _icDriver, _idDriver, _lcDriver;

        _Registe("SendCommand", (*this), &H002CmdDriver::SendCommand);
        _Registe("Interrupt", (*this), &H002CmdDriver::Interrupt);
        _Registe("InterrupterReset", (*this), &H002CmdDriver::InterrupterReset);

        Ref<Command> gateCmd = Command::Make((*this), &H002CmdDriver::SendCommand);
        string gateKey = "Send";
        string magArg = CommandDriver<TArgParser>::Arg(gateKey, "1B 24 41");
        list<Ref<ComplexCommand> > cmds = _magDriver.GetCommand("");
        _PreBind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _pinDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 4B").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _icDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 49").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _pbocDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 49").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _lcDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 49").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);
        // 在LC之后注册,确保SetAckMode能够正确触发
        _Registe("SetAckMode", (*this), &H002CmdDriver::SetAckMode);

        cmds = _idDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 53").c_str());
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
    /// 设置PBOC相关转换表
    inline void SetPbocTable(const ushort infoTable[], const ushort amountTable[], const ushort detailTable[])
    {
        if(infoTable != NULL) _pbocDriver.InformationTABLE = infoTable;
        if(amountTable != NULL) _pbocDriver.AmountTABLE = amountTable;
        if(detailTable != NULL) _pbocDriver.DetailTABLE = detailTable;
    }
    /// 设置TLV转换函数
    inline void SetTlvConvert(PbocTlvConverter::fpTlvAnsConvert ansConvert)
    {
        _pbocDriver.TlvConvert = ansConvert;
    }
    /// 设置身份证照片解码器
    inline void SetWltDecoder(Ref<IWltDecoder> wltDecoder)
    {
        _idDriver.WltDecoder = wltDecoder;
    }
    /// 设置身份证信息转换接口
    inline void SetIdcConvert(IDCardParser::fpIdcConvert idcConvert)
    {
        _idDriver.IdcConvert = idcConvert;
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
        string send = arg["Send"].To<string>();
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
        return InterruptBehavior::Interrupt(*this);
    }
    /// 重置中断状态
    LC_CMD_METHOD(InterrupterReset)
    {
        InterruptBehavior::Reset(*this);
        return true;
    }
    /**
     * @brief 设置ACK模式
     * @date 2016-06-20 19:50
     * 
     * @param [in] AckMode : bool 需要设置的ACK模式
     */
    LC_CMD_METHOD(SetAckMode)
    {
        /*
        bool isAck = arg["AckMode"].To<bool>(false);
        if(isAck)
        {
            // 未设置过该属性
            if(_cmdAdapter != _pDev)
            {
                _cmdAdapter.SelectDevice(_pDev);
                _pDev = _cmdAdapter;
                _adapterInvoker.SelectDevice(_pDev);
                LOGGER(_log.WriteLine("Enable ACK"));
            }
        }
        else
        {
            if(_cmdAdapter == _pDev)
            {
                _pDev = _cmdAdapter.ActiveDevice();
                _cmdAdapter.SelectDevice(_pDev);
                _adapterInvoker.SelectDevice(_pDev);
                LOGGER(_log.WriteLine("Disable ACK"));
            }
        }
        */
        _cmdAdapter.SelectDevice(_pDev);
        _pDev = _cmdAdapter;
        _adapterInvoker.SelectDevice(_pDev);
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