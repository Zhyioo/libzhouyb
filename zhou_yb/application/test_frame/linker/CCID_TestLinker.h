//========================================================= 
/**@file CCID_TestLinker.h 
 * @brief CCID设备连接器 
 * 
 * @date 2015-01-24   19:26:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_TESTLINKER_H_
#define _LIBZHOUYB_CCID_TESTLINKER_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// CCID读卡器的连接器 
struct CCID_TestLinker : public TestLinker<CCID_Device>
{
    /**
     * @brief 查找是否有指定的读卡器
     * @param [in] dev 需要连接的读卡器
     * @param [in] arg 参数列表
     * - 参数
     *  - Name 需要连接的读卡器名称
     *  - EscapeCommand 支持方式
     * .
     * @param [in] printer 输出器   
     */
    virtual bool Link(CCID_Device& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "CCID_UpdaterTestLinker::Link"));
        list<string> devlist;
        list<string>::iterator itr;

        dev.EnumDevice(devlist);
        LOGGER(dev.SelectLogger(printer.GetLogger()));

        bool bLink = false;
        string reader = arg["Name"].To<string>();
        string escapeMode = arg["EscapeCommand"].To<string>("Auto");

        LOGGER(StringLogger stringlogger;
        stringlogger << "EscapeCommand:<" << escapeMode
            << ">,Updater:<" << reader << ">";
        printer.TextPrint(TextPrinter::TextLogger, stringlogger.String().c_str()));

        for(itr = devlist.begin(); itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), reader.c_str(), true))
            {
                bLink = true;
                LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Contains Reader."));
                break;
            }
        }
        // 没有找到设备 
        if(!bLink)
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Not Exist Reader."));
            return false;
        }
        bLink = false;
        if(StringConvert::Compare(ByteArray(escapeMode.c_str(), escapeMode.length()), "True", true))
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "CCID_Device::EscapeCommand"));
            dev.SetMode(CCID_Device::EscapeCommand);
            bLink = CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) == DevHelper::EnumSUCCESS;
        }
        else if(StringConvert::Compare(ByteArray(escapeMode.c_str(), escapeMode.length()), "False", true))
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "CCID_Device::ApduCommand"));
            dev.SetMode(CCID_Device::ApduCommand);
            bLink = CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) == DevHelper::EnumSUCCESS;
        }
        else
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Try CCID_Device::ApduCommand"));
            dev.SetMode(CCID_Device::ApduCommand);
            if(CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) != DevHelper::EnumSUCCESS)
            {
                // 尝试用EscapeCommand方式连接设备 
                LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Try CCID_Device::EscapeCommand"));
                dev.SetMode(CCID_Device::EscapeCommand);
                bLink = CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) == DevHelper::EnumSUCCESS;
            }
            else
            {
                bLink = true;
            }
        }
        LOGGER(dev.ReleaseLogger(&printer.GetLogger()));

        return bLink;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_TESTLINKER_H_
//========================================================= 