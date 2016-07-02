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
#include "../H002/H002CmdDriver.h"

#include "WE_FingerCmdDriver.h"

#include "../../printer/ICBC_PrinterCmdAdapter.h"
#include "../../printer/ICBC_XmlPrinter.h"
using namespace zhou_yb::application::printer;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// ICBC-MT驱动
template<class TArgParser>
class ICBC_MT_CmdDriver : public H002CmdDriver<TArgParser>
{
protected:
    WE_FingerCmdDriver _fingerDriver;

    ICBC_PrinterCmdAdapter _printerCmdAdapter;
    XmlPrinter _xmlPrinter;
public:
    ICBC_MT_CmdDriver() : H002CmdDriver<TArgParser>()
    {
        this->_appErr.Select(_fingerDriver, "Finger");

        select_helper<LoggerInvoker::SelecterType>::select(this->_logInvoker), _fingerDriver, _xmlPrinter;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType >::select(this->_adapterInvoker),
            _fingerDriver, _printerCmdAdapter;

        _xmlPrinter.SelectDevice(_printerCmdAdapter);

        Ref<Command> gateCmd = Command::Make((*this), &H002CmdDriver<TArgParser>::SendCommand);
        string gateKey = "Send";
        string magArg = CommandDriver<TArgParser>::Arg(gateKey, "1B 24 41");

        list<Ref<ComplexCommand> > cmds;
        cmds.push_back(this->_Registe("PrintXml", (*this), &ICBC_MT_CmdDriver::PrintXml));
        cmds.push_back(this->_Registe("PrintStream", (*this), &ICBC_MT_CmdDriver::PrintStream));
        this->_PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 50").c_str());
        this->_Bind(cmds, gateCmd, ComplexCommand::RunOnFinal, magArg.c_str());

        cmds = _fingerDriver.GetCommand("");
        this->_PreBind(cmds, gateCmd, CommandDriver<TArgParser>::Arg(gateKey, "1B 24 46").c_str());
        this->_Bind(cmds, gateCmd, ComplexCommand::RunOnFinal, magArg.c_str());
        this->Registe(cmds);

        _xmlPrinter.Add<XmlSetter>();
        _xmlPrinter.Add<XmlEnter>();
        _xmlPrinter.Add<XmlTab>();
        _xmlPrinter.Add<XmlString>();
        _xmlPrinter.Add<XmlBarcode>();

        this->_Registe("InputPin", (*this), &ICBC_MT_CmdDriver::InputPin);
    }
    LC_CMD_METHOD(PrintXml)
    {
        string path = arg["Path"].To<string>();
        return _xmlPrinter.Print(path.c_str());
    }
    LC_CMD_METHOD(PrintStream)
    {
        _printerCmdAdapter.Write(DevCommand::FromAscii("1B 40"));
        _printerCmdAdapter.Write(DevCommand::FromAscii("1B 63 00"));

        string text = arg["String"].To<string>();
        ByteBuilder cmd(8);
        DevCommand::FromAscii("1B 38", cmd);

        //IconvCharsetConvert::UTF8ToGBK(text.c_str(), text.length(), cmd);
        cmd += "1234567890";
        cmd += 0x0D;
        _printerCmdAdapter.Write(cmd);
        
        return true;
    }
    LC_CMD_METHOD(InputPin)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        this->_pDev->Write(DevCommand::FromAscii("1B 24 4B 82"));
        Timer timer;
        ByteBuilder key(2);
        ByteBuilder input(8);
        while(timer.Elapsed() < timeoutMs)
        {
            key.Clear();
            if(!this->_pDev->Read(key))
                break;

            for(size_t i = 0;i < key.GetLength(); ++i)
            {
                PSBC_PinCmdDriver::FormatInput(key[i], input);
                if(key[i] == 0x0D || key[i] == 0x1B || key[i] == 0x08)
                {
                    timeoutMs = 0;
                    break;
                }
            }
        }
        this->_pDev->Write(DevCommand::FromAscii("83 1B 24 41"));
        if(input.IsEmpty())
            return false;
        rlt.PushValue("Input", input.GetString());
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