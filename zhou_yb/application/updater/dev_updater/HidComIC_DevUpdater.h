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
class HidComUpdateModeTestLinker : public TestLinker<FixedHidTestDevice>
{
protected:
    WinHidTestLinker _hidLinker;
public:
    /**
     * @brief 扫描串口,并发送升级切换指令
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] devArg 参数 [COM]
     * @param [in] printer 文本输出器
     */
    virtual bool Link(FixedHidTestDevice& dev, const char* devArg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidComUpdateModeTestLinker::Link"));
        string reader = "";
        string upgrade = "";
        string mode = "";

        ArgParser cfg;
        if(cfg.Parse(devArg))
        {
            cfg.GetValue("Boot", upgrade);
            cfg.GetValue("Name", reader);
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
        if(!_hidLinker.Link(dev, devArg, printer))
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
class HidComUpdaterTestCase : public ITestCase<IInteractiveTrans>
{
protected:
    /// 已升级的文件行数
    size_t _updateCount;
    size_t _swCount;
public:
    HidComUpdaterTestCase(size_t swCount = 1) : _swCount(swCount) {}
    /// 升级行
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        // 所有数据已经发送完
        if(testArg.IsEmpty())
            return ComUpdateModeTestLinker::WaitSW(testObj);

        // 先发送长度
        byte len = _itobyte(testArg.GetLength());
        // 直接发送数据包 
        if(testObj->Write(ByteArray(&len, 1)) && testObj->Write(testArg))
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
/// 串口协议HID接口IC卡读卡器固件升级程序 
template<
    class TLinker, 
    class TContainer = TestContainer<FixedHidTestDevice, IInteractiveTrans>, 
    class TDecoder = UpdateDecoder>
class HidComIC_DevUpdater : public DevUpdater<FixedHidTestDevice, IInteractiveTrans, TLinker, TContainer, TDecoder>
{
protected:
    //----------------------------------------------------- 
    /// 加密的随机数 
    ByteBuilder _random;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    virtual bool PreTest(const char* preArg)
    {
        if(!DevUpdater::PreTest(preArg))
            return false;
        // 状态准备
        if(!ComUpdateModeTestLinker::IsUpgradeMode(_testInterface))
        {
            TextPrint(TextPrinter::TextError, "设备状态错误");
            return false;
        }

        // 获取随机数
        _random.Clear();
        if(!ComUpdateModeTestLinker::GetRandom(_testInterface, _random))
        {
            TextPrint(TextPrinter::TextError, "获取随机密钥失败");
            return false;
        }
        
        // 通讯握手
        if(!ComUpdateModeTestLinker::IsUpgradeReady(_testInterface))
        {
            TextPrint(TextPrinter::TextError, "通讯握手失败");
            return false;
        }

        // 处理升级数据
        list<ByteBuilder>::iterator itr;
        for(itr = DevUpdater::_updateList.begin();itr != DevUpdater::_updateList.end(); ++itr)
        {
            ByteConvert::Xor(_random, *itr);
            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                TextPrint(TextPrinter::TextError, "操作被取消");
                return false;
            }
        }
        // 增加一个空行标记文件升级结束
        _updateList.push_back(ByteBuilder());
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
//========================================================= 
