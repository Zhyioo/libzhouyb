//========================================================= 
/**@file HID_IC_DevUpdater.h 
 * @brief HID IC卡固件升级程序 
 * 
 * @date 2015-01-03   11:13:01 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HID_IC_DEVUPDATER_H_
#define _LIBZHOUYB_HID_IC_DEVUPDATER_H_
//--------------------------------------------------------- 
#include "../DevUpdater.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Extension.h"

#include "../../test_frame/linker/TestLinkerHelper.h"
using zhou_yb::application::test::TestLinkerHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// HID读卡器切换升级模式连接器(负责发送切换指令)
struct HidUpdateModeTestLinker : public TestLinker<HidDevice>
{
    /**
     * @brief 检测是否有待升级状态的设备存在,没有的话发送指令进行切换  
     * 
     * @param [in] dev 需要连接的设备
     * @param [in] devArg 参数
     * - 参数
     *  - Updater 升级模式名称
     *  - Reader 正常模式名称
     *  - TransmitMode 端点传输方式
     * .
     * @param [in] printer 文本输出器
     */
    virtual bool Link(HidDevice& dev, const char* devArg, TextPrinter& printer)
    {
        // 检查设备是否为升级模式 
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidUpdateModeTestLinker::Link"));
        string reader = "";
        string upgrade = "";
        string mode = "";

        ArgParser cfg;
        if(cfg.Parse(devArg))
        {
            cfg.GetValue("Updater", upgrade);
            cfg.GetValue("Reader", reader);
            cfg.GetValue("TransmitMode", mode);
        }
        else
        {
            upgrade = _strput(devArg);
        }

        LOGGER(StringLogger stringlogger;
        stringlogger << "Updater:<" << upgrade
            << ">,Reader:<" << reader
            << ">,TransmitMode:<" << mode << ">";
        printer.TextPrint(TextPrinter::TextLogger, stringlogger.String().c_str()));

        list<HidDevice::device_info> devlist;
        list<HidDevice::device_info>::iterator itr;
        dev.EnumDevice(devlist);

        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(ByteArray(itr->Name.c_str(), itr->Name.length()),
                ByteArray(upgrade.c_str(), upgrade.length()), true))
            {
                LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Contains Updater"));
                return true;
            }
        }
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Open Reader"));
        if(HidDeviceHelper::OpenDevice<HidDevice>(dev, reader.c_str(), SIZE_EOF, &devlist) != DevHelper::EnumSUCCESS)
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Open Failed"));
            return false;
        }

        HidDevice::TransmitMode hidMode = HidDevice::StringToMode(mode.c_str());
        dev.SetTransmitMode(hidMode);

        if(!TestLinkerHelper::LinkTimeoutBehavior(dev, cfg, printer))
            return false;
        if(!TestLinkerHelper::LinkCommand(dev, cfg, printer))
            return false;
        
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Change Reader To Updater"));

        ComICCardCmdAdapter cmdAdapter;
        ByteBuilder updateRecv(8);

        cmdAdapter.SelectDevice(dev);
        cmdAdapter.Write(DevCommand::FromAscii("31 15"));

        return dev.Read(updateRecv);
    }
};
/// HID读卡器升级连接器
struct HidUpdaterTestLinker : public TestLinker<HidDevice>
{
    /**
     * @brief 连接设备
     * 
     * @param [in] dev 需要连接的设备
     * @param [in] devArg 参数 "[Updater:<升级模式名称>]"
     * @param [in] printer 文本输出器
     */
    virtual bool Link(HidDevice& dev, const char* devArg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidUpdaterTestLinker::Link"));
        string reader = "";
        ArgParser cfg;

        if(cfg.Parse(devArg))
        {
            cfg.GetValue("Updater", reader);
        }
        else
        {
            reader = _strput(devArg);
        }
        LOGGER(StringLogger stringlogger;
        stringlogger << "Hid Updater:<" << reader << ">";
        printer.TextPrint(TextPrinter::TextLogger, stringlogger.String().c_str()));
        bool bLink = (HidDeviceHelper::OpenDevice(dev, reader.c_str(), SIZE_EOF) == DevHelper::EnumSUCCESS);
        return bLink;
    }
    /// 断开连接
    virtual bool UnLink(HidDevice& dev, TextPrinter&)
    {
        dev.Close();
        return true;
    }
};
//--------------------------------------------------------- 
/// HID读卡器文件行升级器 
class HidUpdaterTestCase : public ITestCase<HidDevice>
{
protected:
    /// 发送的升级数据
    ByteBuilder _updateBin;
public:
    /// 升级文件行 
    virtual bool Test(Ref<HidDevice>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        /* 将多个bin数据直接拼成HID设备的整包 */
        size_t len = testArg.GetLength() + _updateBin.GetLength();
        if(len < testObj->GetSendLength())
        {
            _updateBin += testArg;

            // 不是文件最后一行,需要继续补包
            if(!DevUpdaterConvert::IsEOF(testArg))
                return true;
        }

        /* 开始升级数据 */
        if(testObj->Write(_updateBin))
        {
            _updateBin.Clear();
            ByteBuilder tmp(8);
            if(!testObj->Read(tmp))
                return false;

            switch(tmp[0])
            {
            case 0x00:
                break;
            case 0x90:
                // 把当前行添加到升级数据缓冲区中
                _updateBin += testArg;
                return true;
            case 0x63:
            default:
                return false;
            }
        }

        return false;
    }
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HID_IC_DEVUPDATER_H_
//========================================================= 