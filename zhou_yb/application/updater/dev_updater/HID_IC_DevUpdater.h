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

#include "../../test_frame/linker/WinHidTestLinker.h"
using zhou_yb::application::test::WinHidTestLinker;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// HID读卡器切换升级模式连接器(负责发送切换指令)
class HidUpdateModeTestLinker : public TestLinker<FixedHidTestDevice>
{
protected:
    WinHidTestLinker _hidLinker;
public:
    /**
     * @brief 检测是否有待升级状态的设备存在,没有的话发送指令进行切换  
     * 
     * @param [in] dev 需要连接的设备
     * @param [in] arg 参数
     * - 参数
     *  - Boot 升级模式名称
     *  - Name 正常模式名称
     *  - TransmitMode 端点传输方式
     * .
     * @param [in] printer 文本输出器
     */
    virtual bool Link(FixedHidTestDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        // 检查设备是否为升级模式 
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidUpdateModeTestLinker::Link"));
        string reader = "";
        string upgrade = "";

        ArgParser cfg;
        cfg.Parse(arg["Updater"].Value.c_str());
        upgrade = cfg["Name"].To<string>();

        cfg.Clear();
        cfg.Parse(arg["Reader"].Value.c_str());
        reader = cfg["Name"].To<string>();

        LOGGER(StringLogger stringlogger;
        stringlogger << "Updater:<" << upgrade
            << ">,Reader:<" << reader;
        printer.TextPrint(TextPrinter::TextLogger, stringlogger.String().c_str()));

        list<HidDevice::device_info> devlist;
        list<HidDevice::device_info>::iterator itr;
        dev.Base().EnumDevice(devlist);

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
        if(!_hidLinker.Link(dev, cfg, printer))
            return false;
        
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Change Reader To Updater"));

        ComICCardCmdAdapter cmdAdapter;
        ByteBuilder updateRecv(8);

        cmdAdapter.SelectDevice(dev);
        cmdAdapter.Write(DevCommand::FromAscii("31 15"));

        return dev.Read(updateRecv);
    }
    /// 关闭设备
    virtual bool UnLink(FixedHidTestDevice& dev, TextPrinter& printer)
    {
        return _hidLinker.UnLink(dev, printer);
    }
};
//--------------------------------------------------------- 
/// HID读卡器文件行升级器 
class HidUpdaterTestCase : public ITestCase<FixedHidTestDevice>
{
protected:
    /// 发送的升级数据
    ByteBuilder _updateBin;
public:
    /// 升级文件行 
    virtual bool Test(Ref<FixedHidTestDevice>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        /* 将多个bin数据直接拼成HID设备的整包 */
        size_t len = testArg.GetLength() + _updateBin.GetLength();
        if(len < testObj->Base().GetSendLength())
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