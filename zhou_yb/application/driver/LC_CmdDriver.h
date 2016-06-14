//========================================================= 
/**@file LC_CmdDriver.h
 * @brief 
 * 
 * @date 2016-06-09   10:36:33
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_CMDDRIVER_H_
#define _LIBZHOUYB_LC_CMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"

#include "../lc/inc/LC_ComToCCID_CmdAdapter.h"
#include "../lc/inc/LC_ReaderDevAdapter.h"
using zhou_yb::application::lc::LC_ComToCCID_CmdAdapter;
using zhou_yb::application::lc::LC_ReaderDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// LC命令驱动
class LC_CmdDriver :
    public DevAdapterBehavior<IInteractiveTrans>,
    public CommandCollection,
    public RefObject
{
protected:
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;
    LoggerInvoker _logInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapter;
    ComICCardCmdAdapter _cmdAdapter;
    LC_ComToCCID_CmdAdapter _lcCmdAdapter;
    LC_ReaderDevAdapter _lcAdapter;
public:
    LC_CmdDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_cmdAdapter, "CMD");
        _lastErr.Select(_lcCmdAdapter, "LC_CMD");
        _lastErr.Select(_lcAdapter, "LC");

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _cmdAdapter, _lcCmdAdapter, _lcAdapter;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType>::select(_adapter),
            _cmdAdapter;
        _lcCmdAdapter.SelectDevice(_cmdAdapter);
        _lcAdapter.SelectDevice(_lcCmdAdapter);

        _Registe("Verify", (*this), &LC_CmdDriver::Verify);
        _Registe("Version", (*this), &LC_CmdDriver::Version);
        _Registe("SetAckMode", (*this), &LC_CmdDriver::SetAckMode);
        _Registe("GetSerialNumber", (*this), &LC_CmdDriver::GetSerialNumber);
        _Registe("SetSerialNumber", (*this), &LC_CmdDriver::SetSerialNumber);
        _Registe("GetPower", (*this), &LC_CmdDriver::GetPower);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_METHOD(Verify)
    {
        if(_lcAdapter.Verify())
            return true;

        _logErr(DeviceError::DevVerifyErr);
        return false;
    }
    /**
     * @brief 获取设备版本
     * @date 2016-06-09 10:50
     * 
     * @retval Version : string
     */
    LC_CMD_METHOD(Version)
    {
        ByteBuilder version(8);
        if(!_lcAdapter.GetVersion(version))
            return false;

        rlt.PushValue("Version", ArgConvert::ToString<ByteBuilder>(version));
        return true;
    }
    /**
     * @brief 设置ACK模式
     * @date 2016-06-09 10:50
     * 
     * @param [in] AckMode : bool 需要设置的ACK模式
     */
    LC_CMD_METHOD(SetAckMode)
    {
        bool isAck = arg["AckMode"].To<bool>(false);
        return _lcAdapter.SetAckMode(isAck);
    }
    LC_CMD_METHOD(GetSerialNumber)
    {
        ByteBuilder serialNumber(8);
        if(!_lcAdapter.GetSerialNumber(serialNumber))
            return false;
        string enc = arg["Encode"].To<string>("Hex");
        ByteBuilder tmp(8);
        CommandDriverHelper::Encoding(enc.c_str(), serialNumber, tmp);
        rlt.PushValue("SerialNumber", tmp.GetString());
        return true;
    }
    LC_CMD_METHOD(SetSerialNumber)
    {
        string serialNumber = arg["SerialNumber"].To<string>();
        return true;
    }
    LC_CMD_METHOD(GetPower)
    {
        uint v = 0;
        bool lastVal = _lcCmdAdapter.IsFormatRecv;
        _lcCmdAdapter.IsFormatRecv = false;
        if(!_lcAdapter.GetVoltage(v))
            return false;
        _lcCmdAdapter.IsFormatRecv = lastVal;
        v /= 2;
        
        uint e = LC_ReaderDevAdapter::VoltageToElectricity(v * 10);
        ByteBuilder tmp(8);
        // 精度为 0.01 v
        tmp.Format("%d.%d", v / 100, v % 100);
        rlt.PushValue("V", tmp.GetString());
        rlt.PushValue("E", ArgConvert::ToString<uint>(e));
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_CMDDRIVER_H_
//========================================================= 