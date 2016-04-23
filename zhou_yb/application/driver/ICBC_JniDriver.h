//========================================================= 
/**@file ICBC_JniDriver.h
 * @brief ���н����ն�����
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
/// ���н����ն�����
class ICBC_JniDriver : public CommandDriver<SplitArgParser>
{
protected:
    /* �豸 */
    LOGGER(FolderLogger _folder);
    JniEnvCmdDriver _jniDriver;
    BoolInterrupter _interrupter;

    /* ������� */
    PSBC_PinCmdDriver _pinDriver;
public:
    ICBC_JniDriver()
    {
        /* �������� */
        RegisteCommand(Command::Bind("NativeInit", (*this), &ICBC_JniDriver::NativeInit));
        RegisteCommand(Command::Bind("NativeDestory", (*this), &ICBC_JniDriver::NativeDestory));
        RegisteCommand(Command::Bind("DriverVersion", (*this), &ICBC_JniDriver::DriverVersion));

        RegisteCommand(_jniDriver);

        /* ������̲��� */
        RegisteCommand(_pinDriver);
    }

    /* Driver���� */
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
    LC_CMD_METHOD(DriverVersion)
    {
        recv = "LC v1.0.0.1 20160402";
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
