//========================================================= 
/**@file ICBC_MT_CmdDriver.h
 * @brief 工行外派终端命令驱动
 * 
 * @date 2016-06-12   15:51:52
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICBC_MT_CMDDRIVER_H_
#define _LIBZHOUYB_ICBC_MT_CMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"

#include "ICCardCmdDriver.h"
#include "PSBC_PinCmdDriver.h"
#include "ComICCardCmdDriver.h"
#include "IDCardCmdDriver.h"
#include "PBOC_CmdDriver.h"
#include "LC_CmdDriver.h"
#include "MagneticCmdDriver.h"

 #include "../../extension/ability/IconvCharsetConvert.h"
 using zhou_yb::extension::ability::IconvCharsetConvert;

#include "../printer/ICBC_PrinterCmdAdapter.h"
#include "../printer/ICBC_XmlPrinter.h"
using namespace zhou_yb::application::printer;

#include "../finger/FingerDevAdapter.h"
using zhou_yb::application::finger::WE_FingerDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// ICBC维尔指纹仪命令驱动
class WE_FingerCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>,
    public InterruptBehavior,
    public CommandCollection,
    public RefObject
{
protected:
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;
    WE_FingerDevAdapter _fingerAdapter;
public:
    WE_FingerCmdDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_fingerAdapter, "DRV");
        _lastErr.Select(_objErr);

        _Registe("GetFeature", (*this), &WE_FingerCmdDriver::GetFeature);
        _Registe("GetTemplate", (*this), &WE_FingerCmdDriver::GetTemplate);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _fingerAdapter);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_LOGGER(_fingerAdapter);
    /**
     * @brief 获取指纹模板
     * @date 2016-06-12 20:01
     * 
     * @param [in] Encode : string 指纹格式
     * - 参数:
     *  - Hex 
     *  - Base64
     * .
     * 
     * @retval Finger : string 指纹模板  
     */
    LC_CMD_METHOD(GetFeature)
    {
        string enc = arg["Encode"].To<string>("Base64");
        ByteBuilder finger(64);
        if(!_fingerAdapter.GetFeature(finger))
            return false;

        ByteBuilder buff(64);
        CommandDriverHelper::Encoding(enc.c_str(), finger, buff);
        rlt.PushValue("Finger", buff.GetString());
        return true;
    }
    /**
     * @brief 获取指纹模板
     * @date 2016-06-12 20:10
     * 
     * @param [in] Encode : string 指纹格式
     * - 参数:
     *  - Hex
     *  - Base64
     * .
     * @retval Finger : string 
     */
    LC_CMD_METHOD(GetTemplate)
    {
        string enc = arg["Encode"].To<string>("Base64");
        ByteBuilder finger(64);
        if(!_fingerAdapter.GetTemplate(finger))
            return false;

        ByteBuilder buff(64);
        CommandDriverHelper::Encoding(enc.c_str(), finger, buff);
        rlt.PushValue("Finger", buff.GetString());
        return true;
    }
};
//--------------------------------------------------------- 
/// ICBC-MT驱动
template<class TArgParser>
class ICBC_MT_CmdDriver :
    public DevAdapterBehavior<IInteractiveTrans>,
    public InterruptBehavior,
    public CommandCollection,
    public RefObject
{
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
    WE_FingerCmdDriver _fingerDriver;

    ICBC_PrinterCmdAdapter _printerCmdAdapter;
    XmlPrinter _xmlPrinter;

    BoolInterrupter _interrupter;

    LC_CMD_METHOD(UpdateIC)
    {
        _pbocDriver.SelectDevice(_icDriver.ActiveIC());
        return true;
    }
public:
    ICBC_MT_CmdDriver()
    {
        _icErr.IsFormatMSG = false;
        _icErr.IsLayerMSG = true;
        _icErr.Select(_icDriver, "DRV");
        _icErr.Select(_pbocDriver, "PBOC");

        _appErr.IsFormatMSG = false;
        _appErr.IsLayerMSG = false;
        _appErr.Select(_magDriver, "MAG");
        _appErr.Select(_pinDriver, "PIN");
        _appErr.Select(_icErr, "IC");
        _appErr.Select(_idDriver, "ID");
        _appErr.Select(_fingerDriver, "FIN");

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_appErr, "MOD");
        _objErr.Invoke(this->_lasterr, this->_errinfo);
        _lastErr.Select(_objErr);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _magDriver, _pinDriver, _icDriver, _pbocDriver, _idDriver, _lcDriver, _fingerDriver, _xmlPrinter;
        select_helper<InterruptInvoker::SelecterType>::select(_interruptInvoker),
            _magDriver, _icDriver, _idDriver;
        select_helper<DevAdapterInvoker<IInteractiveTrans> >::select(_adapterInvoker),
            _magDriver, _pinDriver, _icDriver, _idDriver, _lcDriver, _fingerDriver, _printerCmdAdapter;

        _xmlPrinter.SelectDevice(_printerCmdAdapter);

        _Registe("SendCommand", (*this), &ICBC_MT_CmdDriver::SendCommand);
        _Registe("Interrupt", (*this), &ICBC_MT_CmdDriver::Interrupt);
        _Registe("InterrupterReset", (*this), &ICBC_MT_CmdDriver::InterrupterReset);

        Ref<Command> gateCmd = Command::Make((*this), &ICBC_MT_CmdDriver::SendCommand);
        string gateKey = "Send";
        string magArg = CommandDriver<TArgParser>::Arg(gateKey, "1B 24 41");

        /*
        _Registe("PrintXml", (*this), &ICBC_MT_CmdDriver::PrintXml)
            ->PreBind(gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 50").c_str());
        */
        _Registe("InputPin", (*this), &ICBC_MT_CmdDriver::InputPin);
        _Registe("PrintString", (*this), &ICBC_MT_CmdDriver::PrintString)
            ->PreBind(gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 50").c_str());

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

        cmds = _idDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 53").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        cmds = _fingerDriver.GetCommand("");
        _PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 46").c_str());
        _Bind(cmds, gateCmd, magArg.c_str());
        Registe(cmds);

        Ref<Command> setIcCmd = Command::Make((*this), &ICBC_MT_CmdDriver::UpdateIC);
        Ref<ComplexCommand> complexCmd = LookUp("WaitForCard");
        if(!complexCmd.IsNull())
            complexCmd->Bind(setIcCmd);

        complexCmd = LookUp("SelectSLOT");
        if(!complexCmd.IsNull())
            complexCmd->Bind(setIcCmd);

        _xmlPrinter.Add<XmlSetter>();
        _xmlPrinter.Add<XmlEnter>();
        _xmlPrinter.Add<XmlTab>();
        _xmlPrinter.Add<XmlString>();
        _xmlPrinter.Add<XmlBarcode>();
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapterInvoker);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_INTERRUPT(_interruptInvoker);
    /// 设置信息标签表
    inline void SetTransTable(const ushort* infoTable, const ushort* amountTable, const ushort* detailTable)
    {
        if(infoTable != NULL) PBOC_CmdDriver::InformationTABLE = infoTable;
        if(amountTable != NULL) PBOC_CmdDriver::AmountTABLE = amountTable;
        if(detailTable != NULL) PBOC_CmdDriver::DetailTABLE = detailTable;
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
    LC_CMD_METHOD(PrintXml)
    {
        string path = arg["Path"].To<string>();
        return _xmlPrinter.Print(path.c_str());
    }
    LC_CMD_METHOD(PrintString)
    {
        _printerCmdAdapter.Write(DevCommand::FromAscii("1B 40"));
        _printerCmdAdapter.Write(DevCommand::FromAscii("1B 63 00"));

        string text = arg["String"].To<string>();
        ByteBuilder cmd(8);
        DevCommand::FromAscii("1B 38", cmd);

        IconvCharsetConvert::UTF8ToGBK(text.c_str(), text.length(), cmd);
        _printerCmdAdapter.Write(cmd);
        return true;
    }
    LC_CMD_METHOD(InputPin)
    {
        _pDev->Write(DevCommand::FromAscii("1B 24 4B"));
        _pDev->Write(DevCommand::FromAscii("81"));
        ByteBuilder buff(8);
        _pDev->Read(buff);
        _pDev->Write(DevCommand::FromAscii("83"));

        ByteBuilder tmp(8);
        ByteConvert::ToAscii(buff, tmp);
        rlt.PushValue("Key", tmp.GetString());
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICBC_MT_CMDDRIVER_H_
//========================================================= 