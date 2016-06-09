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

        _Registe("Verify", (*this), LC_CmdDriver::Verify);
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
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_CMDDRIVER_H_
//========================================================= 