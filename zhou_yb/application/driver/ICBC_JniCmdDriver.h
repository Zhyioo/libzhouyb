//========================================================= 
/**@file ICBC_JniCmdDriver.h
 * @brief 工行交互终端驱动
 * 
 * @date 2016-04-02   13:32:49
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICBC_JNIDRIVER_H_
#define _LIBZHOUYB_ICBC_JNIDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"

#include "../tools/IJniCmdDriver.h"

#include "PSBC_PinCmdDriver.h"
using zhou_yb::application::driver::PSBC_PinCmdDriver;

#include "../../extension/ability/SplitArgParser.h"
using zhou_yb::extension::ability::SplitArgParser;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 工行交互终端驱动
class ICBC_JniCmdDriver : public CommandDriver<SplitArgParser>
{
protected:
    /* 设备 */
    LOGGER(FolderLogger _folder);
    JniEnvCmdDriver _jniDriver;
    BoolInterrupter _interrupter;

    /* 密码键盘 */
    PSBC_PinCmdDriver _pinDriver;
public:
    ICBC_JniCmdDriver()
    {
        /* 公共部分 */
        RegisteCommand(Command::Bind("NativeInit", (*this), &ICBC_JniCmdDriver::NativeInit));
        RegisteCommand(Command::Bind("NativeDestory", (*this), &ICBC_JniCmdDriver::NativeDestory));
        RegisteCommand(Command::Bind("DriverVersion", (*this), &ICBC_JniCmdDriver::DriverVersion));

        RegisteCommand(_jniDriver);

        /* 密码键盘部分 */
        RegisteCommand(_pinDriver);
    }

    /* Driver部分 */
    LC_CMD_METHOD(NativeInit)
    {
        LOGGER(string dir = arg["Path"].To<string>();
        _folder.Open(dir.c_str(), "driver", 2, FILE_K(256));
        _log.Select(_folder));

        LOGGER(_pinDriver.SelectLogger(_log));

        _jniDriver.Interrupter = _interrupter;
        _pinDriver.SelectDevice(_jniDriver);

        return true;
    }
    LC_CMD_METHOD(NativeDestory)
    {
        LOGGER(_folder.Close();
        _log.Release());

        return true;
    }
    /**
     * @brief 获取驱动版本
     * @date 2016-05-06 17:10
     * 
     * @return Version
     */
    LC_CMD_METHOD(DriverVersion)
    {
        rlt.PutValue("Version", "LC v1.0.0.1 20160402");
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICBC_JNIDRIVER_H_
//========================================================= 
