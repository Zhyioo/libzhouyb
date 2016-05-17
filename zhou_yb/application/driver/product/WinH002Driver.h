//========================================================= 
/**@file WinH002Driver.h
 * @brief Windows下H002驱动
 * 
 * @date 2016-05-15   16:50:10
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINH002DRIVER_H_
#define _LIBZHOUYB_WINH002DRIVER_H_
//--------------------------------------------------------- 
#include "../CommonCmdDriver.h"
#include "../H002CmdDriver.h"

#include "../../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// Windows下H002驱动
template<class TArgParser>
class WinH002Driver : public CommandDriver<TArgParser>
{
protected:
    HidDevice _hidDev;
    Ref<IInteractiveTrans> _activeDev;

    LOGGER(FileLogger _file);
    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _devErr;
    LastErrExtractor _lastErr;
    BoolInterrupter _interrupter;
    HidCmdAdapter<HidDevice, 0, 1> _hidCmdAdapter;
    H002CmdDriver<TArgParser> _h002;
public:
    WinH002Driver() : CommandDriver()
    {
        _hidDev.Interrupter = _interrupter;
        _h002.SetInterrupter(_interrupter);

        _hidCmdAdapter.SelectDevice(_hidDev);
        _h002.SelectDevice(_hidCmdAdapter);

        _devErr.IsFormatMSG = false;
        _devErr.IsLayerMSG = false;
        _devErr.Select(_hidDev, "HID");

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_devErr);
        _lastErr.Select(_h002, "H002");
        _objErr.Invoke(_lasterr, _errinfo);
        _lastErr.Select(_objErr, "APP");

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker), _h002;

        _Registe("NativeInit", (*this), &WinH002Driver::NativeInit);
        _Registe("NativeDestory", (*this), &WinH002Driver::NativeDestory);

        _Registe("EnumCommand", (*this), &WinH002Driver::EnumCommand);
        _Registe("Open", (*this), &WinH002Driver::Open);
        _Registe("Close", (*this), &WinH002Driver::Close);
        _Registe("LastError", (*this), &WinH002Driver::LastError);
        _Registe("InterrupterReset", (*this), &WinH002Driver::InterrupterReset);
        
        Registe(_h002.GetCommand(""));

        Ref<ComplexCommand> complexCmd = _Registe("Interrupt", (*this), &WinH002Driver::Interrupt);
        complexCmd->PreBind(LookUp("UpdateInterrupter"));
    }
    Ref<IInteractiveTrans> ActiveDevice()
    {
        return _activeDev;
    }
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_METHOD(NativeInit)
    {
        LOGGER(_file.Open(WinHelper::GetSystemPath());
        _log.Select(_file));

        LOGGER(_h002.SelectLogger(_log));
        return true;
    }
    LC_CMD_METHOD(NativeDestory)
    {
        LOGGER(_file.Close();
        _log.Release();
        _h002.ReleaseLogger());
        return true;
    }
    LC_CMD_METHOD(Open)
    {
        string devName = arg["Name"].To<string>("BP8903-H002");
        if(HidDeviceHelper::OpenDevice<HidDevice>(_hidDev, devName.c_str()) != DevHelper::EnumSUCCESS)
        {
            _logErr(DeviceError::ArgErr, "没有找到指定的设备");
            return false;
        }
        return true;
    }
    LC_CMD_METHOD(Close)
    {
        _hidDev.Close();
        return true;
    }
    LC_CMD_METHOD(Interrupt)
    {
        if(_interrupter.Interrupt())
        {
            while(!_interrupter.IsBreakout())
            {
                Timer::Wait(DEV_OPERATOR_INTERVAL);
            }
        }
        return true;
    }
    LC_CMD_METHOD(InterrupterReset)
    {
        return _interrupter.Reset();
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINH002DRIVER_H_
//========================================================= 