//========================================================= 
/**@file CCID_IC_ReaderDevAdapter.h 
 * @brief CCID设备控制指令
 * 
 * @date 2014-09-06   16:28:48 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_IC_READERDEVADAPTER_H_
#define _LIBZHOUYB_CCID_IC_READERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../device/iccard/base/ICCardLibrary.h"

#include "../../device/iccard/base/ICCardAppAdapter.h"
using zhou_yb::device::iccard::base::ICCardAppAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace device {
//--------------------------------------------------------- 
/// CCID IC读卡器操作 
class CCID_IC_ReaderDevAdapter : public ICCardAppAdapter
{
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuff;
    /// 接收缓冲区 
    ByteBuilder _recvBuff;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 获取读卡器版本信息  
     * @param [out] verBuff 获取到的版本信息 
     */ 
    bool GetVersion(ByteBuilder& verBuff)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("E0 00 00 18 00", _sendBuff);
        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        LOGGER(_log.WriteLine("返回数据:").WriteLine(_recvBuff));

        _recvBuff.RemoveFront(5);
        verBuff = _recvBuff;

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief LED灯控制 
     * @param [in] isTurnOn 是否打开LED灯 
     * @param [in] isControlRed [default:true] 是否控制红灯(默认:true) 
     * @param [in] isControlGreen [default:true] 是否打开绿灯(默认:true) 
     */ 
    bool LedControl(bool isTurnOn, bool isControlRed = true, bool isControlGreen = true)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"LED控制,灯状态:<"<<(isTurnOn ? "开" : "关")<<">,控制红灯:<"
            <<isControlRed<<">,控制绿灯:<"<<isControlGreen<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("E0 00 00 29 01", _sendBuff);


        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace device
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_IC_READERDEVADAPTER_H_
//========================================================= 