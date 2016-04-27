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
#include "../../../extension/ability/win_helper/WinCCID_EscapeCommandHelper.h"
using zhou_yb::extension::ability::WinCCID_EscapeCommandHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// CCID读卡器切换升级模式连接器(负责发送切换指令)
struct CCID_UpdateModeTestLinker : public TestLinker<CCID_Device>
{
    /**
     * @brief 检测是否有待升级状态的设备存在,没有的话发送指令进行切换
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] devArg 参数 "[Updater:<Upgrade>][Reader:<SAM>]"
     * @param [in] printer 文本输出器
     */
    virtual bool Link(CCID_Device& dev, const char* devArg, TextPrinter& printer)
    {
        // 检查设备是否为升级模式 
        string reader = "";
        string upgrade = "";

        ArgParser cfg;
        if(cfg.Parse(devArg))
        {
            cfg.GetValue("Updater", upgrade);
            cfg.GetValue("Reader", reader);
        }
        else
        {
            upgrade = _strput(devArg);
        }

        list<string> devlist;
        list<string>::iterator itr;
        dev.EnumDevice(devlist);

        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(
                ByteArray(itr->c_str(), itr->length()),
                ByteArray(upgrade.c_str(), upgrade.length()), true))
            {
                return true;
            }
        }

        if(CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) != DevHelper::EnumSUCCESS)
            return false;

        ByteBuilder sApdu(8);
        ByteBuilder rApdu(8);
        DevCommand::FromAscii("FF 00 82 00 00", sApdu);
        dev.Apdu(sApdu, rApdu);
        dev.PowerOff();
        return false;
    }
};
//--------------------------------------------------------- 
/// CCID读卡器升级连接器 
struct CCID_UpdaterTestLinker : public TestLinker<CCID_Device>
{
    /**
     * @brief 检查是否存在指定名称的设备
     * 
     * @param [in] dev 需要操作的设备 
     * @param [in] devArg 参数 "[Updater:<Upgrade>][EscapeCommand:<Auto|True|False>]"
     * @param [in] printer 文本输出器 
     */
    virtual bool Link(CCID_Device& dev, const char* devArg, TextPrinter& printer)
    {
        // 连接多个设备时的设备索引号 
        string reader = "";
        string escapeMode = "";
        ArgParser cfg;

        bool bLink = false;
        if(cfg.Parse(devArg))
        {
            cfg.GetValue("EscapeCommand", escapeMode);
            cfg.GetValue("Updater", reader);
        }
        else
        {
            reader = _strput(devArg);
            escapeMode = "Auto";
        }

        list<string> devlist;
        dev.EnumDevice(devlist);

        if(StringConvert::Compare(ByteArray(escapeMode.c_str(), escapeMode.length()), "True", true))
        {
            dev.SetMode(CCID_Device::EscapeCommand);
            bLink = CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) == DevHelper::EnumSUCCESS;
        }
        else if(StringConvert::Compare(ByteArray(escapeMode.c_str(), escapeMode.length()), "False", true))
        {
            dev.SetMode(CCID_Device::ApduCommand);
            bLink = CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) == DevHelper::EnumSUCCESS;
        }
        else
        {
            dev.SetMode(CCID_Device::ApduCommand);
            if(CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) != DevHelper::EnumSUCCESS)
            {
                // 尝试用EscapeCommand方式连接设备 
                dev.SetMode(CCID_Device::EscapeCommand);
                bLink = CCID_DeviceHelper::PowerOn(dev, reader.c_str(), NULL, SIZE_EOF, &devlist) == DevHelper::EnumSUCCESS;
            }
            else
            {
                bLink = true;
            }
        }
        return bLink;
    }
    /// 断开连接
    virtual bool UnLink(CCID_Device& dev, TextPrinter&)
    {
        dev.PowerOff();
        return true;
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
     * @param [in] testArg 参数 "[Updater:<SAM>][VID:<1DFC>][PID:<8903>]"
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
            cfg.GetValue("Updater", devName);
            cfg.GetValue("VID", vid);
            cfg.GetValue("PID", pid);
        }

        testObj->SetMode(CCID_Device::EscapeCommand);
        // 当前已经支持
        if(CCID_DeviceHelper::PowerOn(testObj, devName.c_str(), NULL, SIZE_EOF) != DevHelper::EnumSUCCESS)
            return true;

        // 修改注册表并重新尝试
        bool isChanged = false;
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