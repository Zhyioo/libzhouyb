//========================================================= 
/**@file ICBC_MT_CmdDriver.h
 * @brief ���������ն���������
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

#include "../printer/ICBC_PrinterCmdAdapter.h"
#include "../printer/ICBC_XmlPrinter.h"
using zhou_yb::application::printer::ICBC_PrinterCmdAdapter;
using zhou_yb::application::printer::XmlPrinter;

#include "../finger/FingerDevAdapter.h"
using zhou_yb::application::finger::WE_FingerDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// ICBCά��ָ������������
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
    /// ת��ָ�����ݸ�ʽ
    static bool Encoding(const char* encode, const ByteArray& finger, ByteBuilder& buff)
    {
        ByteArray enc(encode);
        if(StringConvert::Compare(enc, "HEX", true))
        {
            ByteConvert::ToAscii(finger, buff);
            return true;
        }
        else if(StringConvert::Compare(enc, "Base64", true))
        {
            Base64_Provider::Encode(finger, buff);
            return true;
        }

        return false;
    }
    /**
     * @brief ��ȡָ��ģ��
     * @date 2016-06-12 20:01
     * 
     * @param [in] Encode : string ָ�Ƹ�ʽ
     * - ����:
     *  - Hex 
     *  - Base64
     * .
     * 
     * @retval Finger : string ָ��ģ��  
     */
    LC_CMD_METHOD(GetFeature)
    {
        string enc = arg["Encode"].To<string>("Base64");
        ByteBuilder finger(64);
        if(!_fingerAdapter.GetFeature(finger))
            return false;

        ByteBuilder buff(64);
        Encoding(enc.c_str(), finger, buff);
        rlt.PushValue("Finger", buff.GetString());
        return true;
    }
    /**
     * @brief ��ȡָ��ģ��
     * @date 2016-06-12 20:10
     * 
     * @param [in] Encode : string ָ�Ƹ�ʽ
     * - ����:
     *  - Hex
     *  - Base64
     * .
     * @retval Finger : string 
     */
    LC_CMD_METHOD(GetTemplate)
    {
        string enc = arg["Encode"].To<string>("Base64");
        ByteBuilder finger(64);
        if(!_fingerAdapter.GetFeature(finger))
            return false;

        ByteBuilder buff(64);
        Encoding(enc.c_str(), finger, buff);
        rlt.PushValue("Finger", buff.GetString());
        return true;
    }
};
//--------------------------------------------------------- 
/// ICBC-MT����
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
            _magDriver, _pinDriver, _icDriver, _pbocDriver, _idDriver, _fingerDriver;
        select_helper<InterruptInvoker::SelecterType>::select(_interruptInvoker),
            _magDriver, _icDriver, _idDriver;
        select_helper<DevAdapterInvoker<IInteractiveTrans> >::select(_adapterInvoker),
            _magDriver, _pinDriver, _icDriver, _idDriver, _fingerDriver;

        _Registe("SendCommand", (*this), &ICBC_MT_CmdDriver::SendCommand);
        _Registe("Interrupt", (*this), &ICBC_MT_CmdDriver::Interrupt);
        _Registe("InterrupterReset", (*this), &ICBC_MT_CmdDriver::InterrupterReset);

        Ref<Command> gateCmd = Command::Make((*this), &ICBC_MT_CmdDriver::SendCommand);
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
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapterInvoker);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_INTERRUPT(_interruptInvoker);
    /// ������Ϣ��ǩ��
    inline void SetTransTable(const ushort* infoTable, const ushort* amountTable, const ushort* detailTable)
    {
        if(infoTable != NULL) PBOC_CmdDriver::InformationTABLE = infoTable;
        if(amountTable != NULL) PBOC_CmdDriver::AmountTABLE = amountTable;
        if(detailTable != NULL) PBOC_CmdDriver::DetailTABLE = detailTable;
    }
    /// ����TLVת������
    inline void SetTlvConvert(PbocTlvConverter::fpTlvAnsConvert ansConvert)
    {
        PBOC_CmdDriver::TlvConvert = ansConvert;
    }
    /// �������֤��Ƭ������
    inline void SetWltDecoder(Ref<IWltDecoder> wltDecoder)
    {
        _idDriver.WltDecoder = wltDecoder;
    }
    /// �������֤��Ϣת���ӿ�
    inline void SetIdcConvert(IDCardParser::fpIdcConvert idcConvert)
    {
        _idDriver.IdcConvert = idcConvert;
    }
    /// ����Ƿ������豸
    LC_CMD_METHOD(IsValid)
    {
        return BaseDevAdapterBehavior<IInteractiveTrans>::IsValid();
    }
    /**
    * @brief ���Ϳ���ָ��
    * @date 2016-06-09 10:52
    *
    * @param [in] Send : string ��Ҫ���͵�ָ��
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
    /// �ж϶�ȡ����
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
    /// �����ж�״̬
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
#endif // _LIBZHOUYB_ICBC_MT_CMDDRIVER_H_
//========================================================= 