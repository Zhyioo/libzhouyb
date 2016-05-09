//========================================================= 
/**@file HidComIC_DevUpdater.h
 * @brief 
 * 
 * @date 2016-05-07   16:15:58
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
#define _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
//--------------------------------------------------------- 
#include "COM_IC_DevUpdater.h"
#include "../../test_frame/linker/WinHidTestLinker.h"
using zhou_yb::application::test::WinHidTestLinker;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 通过COM的协议检测HID升级设备是否存在
class HidComUpdateModeTestLinker : public TestLinker<FixedHidTestDevice>
{
protected:
    WinHidTestLinker _hidLinker;
public:
    /**
     * @brief 扫描串口,并发送升级切换指令
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] devArg 参数 
     * @param [in] printer 文本输出器
     */
    virtual bool Link(FixedHidTestDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidComUpdateModeTestLinker::Link"));

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

        if(ComUpdateModeTestLinker::ToUpgradeMode(dev))
        {
            if(ComUpdateModeTestLinker::IsUpgradeMode(dev))
                return true;
        }
        dev.Base().Close();
        return false;
    }
    /// 关闭串口
    virtual bool UnLink(FixedHidTestDevice& dev, TextPrinter&)
    {
        dev.Base().Close();
        return true;
    }
};
//--------------------------------------------------------- 
/// 连接升级模式下的设备
class HidComUpdateTestLinker : public WinHidTestLinker
{
public:
    virtual bool Link(FixedHidTestDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        ArgParser cfg;
        cfg.Parse(arg["Updater"].Value.c_str());
        if(WinHidTestLinker::Link(dev, cfg, printer))
        {
            if(ComUpdateModeTestLinker::IsUpgradeMode(dev))
                return true;
            WinHidTestLinker::UnLink(dev, printer);
        }
        return false;
    }
};
//--------------------------------------------------------- 
class HidComUpdaterTestCase : public ITestCase<FixedHidTestDevice>
{
protected:
    /// 已升级的文件行数
    size_t _updateCount;
    /// 发送N条后接收状态码
    size_t _swCount;
    /// 发送的升级数据
    ByteBuilder _updateBin;
public:
    HidComUpdaterTestCase(size_t swCount = 1) : _swCount(swCount) {}
    /// 升级行
    virtual bool Test(Ref<FixedHidTestDevice>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        /* 开始升级数据 */
        byte len = _itobyte(testArg.GetLength());
        _updateBin.Clear();
        _updateBin += len;
        _updateBin += testArg;
        bool bUpdate = testObj->Write(_updateBin);
        if(bUpdate)
        {
            ++_updateCount;
            // 连续发送N条或最后一条才判断状态码 
            if((_updateCount >= _swCount))
            {
                _updateCount = 0;
                return ComUpdateModeTestLinker::WaitSW(testObj);
            }
            return true;
        }
        return false;
    }
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
//========================================================= 
