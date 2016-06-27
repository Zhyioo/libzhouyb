//========================================================= 
/**@file WinDeviceCmdDriver.h
 * @brief Windows下基础设备的命令驱动
 * 
 * @date 2016-06-25   14:31:01
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINDEVICECMDDRIVER_H_
#define _LIBZHOUYB_WINDEVICECMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"

#include "../../include/BaseDevice.h"
#include "../../include/Extension.h"

#include "../../application/tools/ParamHelper.h"
using zhou_yb::application::tools::ParamHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// Windows下基础类设备的命令驱动
template<class TCmdDriver>
class WinDriver : public CommandDriver<typename TCmdDriver::ArgParserType>
{
protected:
    LOGGER(FolderLogger _folder);

    ComDevice _comDev;
    HidDevice _hidDev;
    BluetoothDevice _bthDev;
    Ref<IBaseDevice> _activeDev;
    Ref<IInteractiveTrans> _activeTrans;

    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _devErr;
    LastErrExtractor _lastErr;
    HidFixedCmdAdapter<HidDevice> _hidCmdAdapter;

    BoolInterrupter _interrupter;
    TCmdDriver _driver;
public:
    WinDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _devErr.IsFormatMSG = false;
        _devErr.IsLayerMSG = false;
        _devErr.Select(_comDev, "COM");
        _devErr.Select(_bthDev, "BTH");
        _devErr.Select(_hidDev, "HID");

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_devErr);
        _lastErr.Select(_driver, "driver");
        _lastErr.Select(_objErr);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker), _hidDev, _bthDev, _comDev, _driver;

        // 设置中断器
        _hidDev.Interrupter = _interrupter;
        _comDev.Interrupter = _interrupter;
        _bthDev.Interrupter = _interrupter;
        _driver.SetInterrupter(_interrupter);

        _activeDev = _comDev;
        _activeTrans = _comDev;
        _hidCmdAdapter.SelectDevice(_hidDev);

        _Registe("NativeInit", (*this), &WinDriver::NativeInit);
        _Registe("NativeDestory", (*this), &WinDriver::NativeDestory);

        _Registe("EnumCommand", (*this), &WinDriver::EnumCommand);
        _Registe("LastError", (*this), &WinDriver::LastError);

        _Registe("Open", (*this), &WinDriver::Open);
        _Registe("IsOpen", (*this), &WinDriver::IsOpen);
        _Registe("Close", (*this), &WinDriver::Close);
        _Registe("SetTimeoutMS", (*this), &WinDriver::SetTimeoutMS);
    }
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_LOGGER(_logInvoker);
    /// 当前激活的设备
    Ref<IInteractiveTrans> ActiveDevice()
    {
        return _activeTrans;
    }
    /**
     * @brief 打开设备
     * @date 2016-06-09 10:55
     * 
     * @param [in] Mode : string 需要打开的设备类型
     * - 参数:
     *  - USB
     *  - BTH
     *  - COM
     * .
     * @param [in] Name : string 需要打开的设备名称
     * 
     * 蓝牙部分:
     * @param [in] PIN : string 使用蓝牙时的配对密码
     * @param [in] Timeout : string 搜索设备的超时时间
     * 
     * HID部分:
     * @param [in] FixedIN : uint HID接收包中长度标识字节大小
     * @param [in] FixedOut : uint HID发送包中长度标识字节大小
     * @param [in] TransmitMode : string 断点传输类型
     * - 参数:
     *  - Interrupt
     *  - Control
     *  - Feature
     * .
     */
    LC_CMD_METHOD(Open)
    {
        ByteBuilder devType = arg["Mode"].To<string>("AUTO").c_str();
        string devName;
        if(!arg["Name"].Get(devName))
        {
            _logErr(DeviceError::ArgFormatErr, "查找不到设备名称");
            return false;
        }
        if(StringConvert::Compare(devType, "USB", true))
        {
            if(HidDeviceHelper::OpenDevice<HidDevice>(_hidDev, devName.c_str()) != DevHelper::EnumSUCCESS)
            {
                _logErr(DeviceError::ArgErr, "打开USB设备失败");
                return false;
            }
            uint fixedLen = arg["FixedIN"].To<uint>(0);
            _hidCmdAdapter.FixedInput = fixedLen;
            fixedLen = arg["FixedOut"].To<uint>(0);
            _hidCmdAdapter.FixedOutput = fixedLen;

            string sMode = arg["TransmitMode"].To<string>();
            HidDevice::TransmitMode hidMode = HidDevice::StringToMode(sMode.c_str());
            _hidDev.SetTransmitMode(hidMode);

            _activeDev = _hidDev;
            _activeTrans = _hidCmdAdapter;
        }
        else if(StringConvert::Compare(devType, "BTH", true))
        {
            string devPin = arg["Pin"].To<string>().c_str();
            if(BluetoothDeviceHelper::OpenDevice<BluetoothDevice>(_bthDev, devName.c_str(), devPin.c_str()) != DevHelper::EnumSUCCESS)
            {
                _logErr(DeviceError::ArgErr, "打开蓝牙设备失败");
                return false;
            }
            _activeDev = _bthDev;
            _activeTrans = _bthDev;
        }
        else if(StringConvert::Compare(devType, "COM", true))
        {
            byte gate = 0;
            uint baud = CBR_9600;
            uint port = ParamHelper::ParseCOM(devName.c_str(), gate, baud);
            if(ComDeviceHelper::OpenDevice<ComDevice>(_comDev, port, baud) != DevHelper::EnumSUCCESS)
            {
                _logErr(DeviceError::ArgErr, "打开串口失败");
                return false;
            }
            // 转口
            if(gate != 0)
            {
                ByteBuilder cmd(8);
                DevCommand::FromAscii("1B 24", cmd);
                cmd += _get_upper(gate);

                _comDev.Write(cmd);
            }
            _activeTrans = _comDev;
            _activeDev = _comDev;
        }
        else
        {
            _logErr(DeviceError::ArgFormatErr, "错误的设备类型");
            return false;
        }
        _driver.SelectDevice(_activeTrans);
        return true;
    }
    LC_CMD_METHOD(IsOpen)
    {
        return _activeDev->IsOpen();
    }
    /// 关闭设备
    LC_CMD_METHOD(Close)
    {
        _activeDev->Close();
        return true;
    }
    /**
     * @brief 初始化调用
     * @date 2016-06-25 17:32
     *
     * @param [in] Path : string 需要设置的日志目录
     */
    LC_CMD_METHOD(NativeInit)
    {
        LOGGER(string dir = arg["Path"].To<string>();
        _folder.Open(dir.c_str(), "driver", 2, FILE_K(256));
        CommandDriver<typename TCmdDriver::ArgParserType>::_log.Select(_folder);
        _logInvoker.SelectLogger(CommandDriver<typename TCmdDriver::ArgParserType>::_log));

        return true;
    }
    LC_CMD_METHOD(NativeDestory)
    {
        LOGGER(_folder.Close();
        CommandDriver<typename TCmdDriver::ArgParserType>::_log.Release();
        _logInvoker.ReleaseLogger());

        return true;
    }
    /**
     * @brief 设置设备读写超时时间
     * @date 2016-06-25 17:38
     * 
     * @param [in] Timeout : uint 超时时间(ms)
     *
     * @retval Timeout : uint 上次的超时时间(ms)
     */
    LC_CMD_METHOD(SetTimeoutMS)
    {
        const char TimeoutMS[] = "Timeout";
        uint timeoutMs = arg[TimeoutMS].To<uint>(DEV_WAIT_TIMEOUT);
        uint lastVal = _comDev.SetWaitTimeout(timeoutMs);
        _bthDev.SetWaitTimeout(timeoutMs);
        _hidDev.SetWaitTimeout(timeoutMs);
        rlt.PushValue(TimeoutMS, ArgConvert::ToString<uint>(lastVal));
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINDEVICECMDDRIVER_H_
//========================================================= 