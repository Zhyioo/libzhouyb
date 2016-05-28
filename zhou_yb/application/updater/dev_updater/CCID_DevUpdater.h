//========================================================= 
/**@file CCID_DevUpdater.h 
 * @brief CCID IC卡读卡器固件升级程序 
 * 
 * @date 2015-01-02   15:46:22 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_DEVUPDATER_H_
#define _LIBZHOUYB_CCID_DEVUPDATER_H_
//--------------------------------------------------------- 
#include "../DevUpdater.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Extension.h"

#include "../../test_frame/linker/CCID_TestLinker.h"
using zhou_yb::application::test::CCID_TestLinker;

#include "../../../extension/ability/win_helper/WinCCID_EscapeCommandHelper.h"
using zhou_yb::extension::ability::WinCCID_EscapeCommandHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// CCID读卡器切换升级模式连接器(负责发送切换指令)
class CCID_UpdateModeTestLinker : public TestLinker<CCID_Device>
{
protected:
    CCID_TestLinker _ccidLinker;
public:
    /**
     * @brief 检测是否有待升级状态的设备存在,没有的话发送指令进行切换
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] devArg 参数 "[Updater:<[Name:<Upgrade>]>][Reader:<[Name:<SAM>]>]"
     * @param [in] printer 文本输出器
     */
    virtual bool Link(CCID_Device& dev, IArgParser<string, string>& devArg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "CCID_UpdateModeTestLinker::Link"));
        // 检查设备是否为升级模式
        ArgParser cfg;
        string reader = "";
        string upgrade = "";
        cfg.Parse(devArg["Updater"].Value.c_str());
        upgrade = cfg["Name"].To<string>();

        cfg.Clear();
        cfg.Parse(devArg["Reader"].Value.c_str());
        reader = cfg["Name"].To<string>();

        LOGGER(StringLogger stringlogger;
        stringlogger << "Updater:<" << upgrade
            << ">,Name:<" << reader << ">";
        printer.TextPrint(TextPrinter::TextLogger, stringlogger.String().c_str()));

        list<string> devlist;
        list<string>::iterator itr;
        dev.EnumDevice(devlist);

        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(
                ByteArray(itr->c_str(), itr->length()),
                ByteArray(upgrade.c_str(), upgrade.length()), true))
            {
                LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Contains Updater"));
                return true;
            }
        }

        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Open Reader"));
        if(!_ccidLinker.Link(dev, cfg, printer))
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Open Failed"));
            return false;
        }

        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Change Reader To Updater"));
        ByteBuilder sApdu(8);
        ByteBuilder rApdu(8);
        DevCommand::FromAscii("FF 00 82 00 00", sApdu);
        dev.Apdu(sApdu, rApdu);
        dev.PowerOff();
        return false;
    }
    /// 断开连接
    virtual bool UnLink(CCID_Device& dev, TextPrinter& printer)
    {
        return _ccidLinker.UnLink(dev, printer);
    }
};
//--------------------------------------------------------- 
/// CCID读卡器升级连接器 
class CCID_UpdaterTestLinker : public TestLinker<CCID_Device>
{
protected:
    CCID_TestLinker _ccidLinker;
public:
    /**
     * @brief 检查是否存在指定名称的设备
     * 
     * @param [in] dev 需要操作的设备 
     * @param [in] devArg 参数 嵌套的"Updater" "[Name:<Upgrade>][EscapeCommand:<Auto|True|False>]"
     * @param [in] printer 文本输出器 
     */
    virtual bool Link(CCID_Device& dev, IArgParser<string, string>& devArg, TextPrinter& printer)
    {
        ArgParser cfg;
        cfg.Parse(devArg["Updater"].Value.c_str());
        return _ccidLinker.Link(dev, cfg, printer);
    }
    /// 断开连接
    virtual bool UnLink(CCID_Device& dev, TextPrinter& printer)
    {
        return _ccidLinker.UnLink(dev, printer);
    }
};
//--------------------------------------------------------- 
/// CCID读卡器EscapeCommand测试器
class CCID_EscapeCommandTestCase : public ITestCase<CCID_Device>
{
public:
    /**
     * @brief EscapeCommand测试
     * 
     * @param [in] testObj 需要操作的设备
     * @param [in] testArg 参数 "[Name:<SAM>][VID:<1DFC>][PID:<8903>]"
     * @param [in] printer 文本输出器
     */
    virtual bool Test(Ref<CCID_Device>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        string vid;
        string pid;
        string devName = testArg.GetString();

        ArgParser cfg;
        if(cfg.Parse(testArg.GetString()))
        {
            cfg.GetValue("Name", devName);
            cfg.GetValue("VID", vid);
            cfg.GetValue("PID", pid);
        }

        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "CCID_Device::EscapeCommand"));
        testObj->SetMode(CCID_Device::EscapeCommand);
        // 当前已经支持
        if(CCID_DeviceHelper::PowerOn(testObj, devName.c_str(), NULL, SIZE_EOF) != DevHelper::EnumSUCCESS)
        {
            LOGGER(printer.TextPrint(TextPrinter::TextLogger, "EscapeCommand Is Success"));
            return true;
        }

        // 修改注册表并重新尝试
        bool isChanged = false;
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "SetEscapeCommandEnable"));
        bool isEnable = WinCCID_EscapeCommandHelper::SetEscapeCommandEnable(vid.c_str(), pid.c_str(), true, &isChanged);
        if(isChanged)
        {
            printer.TextPrint(TextPrinter::TextError, "修改注册表成功,请拔插设备或重启电脑后启动程序进行升级");
            return false;
        }
        if(!isEnable)
        {
            printer.TextPrint(TextPrinter::TextError, "修改注册表失败,请以管理员权限重新运行升级程序");
            return false;
        }

        return true;
    }
};
//--------------------------------------------------------- 
/// CCID读卡器文件行升级器 
class CCID_UpdaterTestCase : public ITestCase<CCID_Device>
{
protected:
    /// 发送的APDU 
    ByteBuilder _sApdu;
public:
    CCID_UpdaterTestCase()
    {
        DevCommand::FromAscii("FF 00 83 00", _sApdu);
    }
    /// 升级文件行
    virtual bool Test(Ref<CCID_Device>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        ByteBuilder rApdu(4);
        bool bApdu = false;
        switch(testObj->GetMode())
        {
        case CCID_Device::EscapeCommand:
            bApdu = testObj->Apdu(testArg, rApdu);
            break;
        case CCID_Device::ApduCommand:
            // 需要封装到指令头中 
            _sApdu += _itobyte(testArg.GetLength());
            _sApdu += testArg;

            bApdu = testObj->Apdu(_sApdu, rApdu);

            _sApdu.RemoveTail(testArg.GetLength() + 1);
            break;
        default:
            bApdu = true;
            break;
        }

        // 如果不是最后一行hex数据则判断状态码(是最后一行数据时,设备升级切换,会导致设备连接失败)
        if(DevUpdaterConvert::IsEOF(testArg))
        {
            bApdu = true;
        }
        else
        {
            ushort sw = ICCardLibrary::GetSW(rApdu);
            return bApdu && ICCardLibrary::IsSuccessSW(sw);
        }
        return bApdu;
    }
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_DEVUPDATER_H_
//========================================================= 