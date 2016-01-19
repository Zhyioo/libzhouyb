//========================================================= 
/**@file Sm4_PinDevAdapter.h 
 * @brief 带SM4功能的密码键盘指令封装 
 * 
 * @date 2014-11-02   15:20:30 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"
#include "../../../include/BaseDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace pinpad {
//--------------------------------------------------------- 
/// 带SM4功能的密码键盘指令封装实现 
class Sm4_PinDevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
protected:
    //----------------------------------------------------- 
    ByteBuilder _sendBuff;
    ByteBuilder _recvBuff;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 恢复密钥为出厂设置
     * @param [in] keyIndex [default:0xFF 恢复所有] 需要恢复的密钥索引,索引范围:[0,7]
     */
    bool ResetKey(byte keyIndex = 0xFF)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();
        DevCommand::FromAscii("1B 64 41 0D", _sendBuff);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace pinpad
} // nameapace device
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 