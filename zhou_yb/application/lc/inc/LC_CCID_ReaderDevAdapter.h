//========================================================= 
/**@file LC_CCID_ReaderDevAdapter.h
 * @brief CCID读卡器操作逻辑
 * 
 * @date 2015-12-08   16:12:33
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include "../../../device/iccard/base/ICCardLibrary.h"
using zhou_yb::device::iccard::base::ICCardLibrary;

#include "../../../device/iccard/base/ICCardAppAdapter.h"
using zhou_yb::device::iccard::base::ICCardAppAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace lc {
//--------------------------------------------------------- 
/// CCID设备操作逻辑
class LC_CCID_ReaderDevAdapter : public ICCardAppAdapter
{
public:
    //----------------------------------------------------- 
    /// 天线模式
    enum AntennaMode
    {
        /// CPU卡
        AntennaCPU = 0x00,
        /// M1卡
        AntennaM1 = 0x01
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区
    ByteBuilder _sendBuffer;
    /// 接收缓冲区
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 设置天线模式
     * @warning 需要使用EscapeCommand模式连接设备
     * 
     * @param [in] mode 需要设置的模式
     */
    bool SetAntennaMode(AntennaMode mode)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "Mode:<" << ((mode == AntennaM1) ? "M1" : "CPU") << ">\n");

        _recvBuffer.Clear();
        switch(mode)
        {
        case AntennaM1:
            if(!_apdu(DevCommand::FromAscii("FF 00 51 83 00"), _recvBuffer))
            {
                if(!_apdu(DevCommand::FromAscii("E0 20 00 00 01 83"), _recvBuffer))
                    return _logRetValue(false);
            }
            break;
        case AntennaCPU:
            if(!_apdu(DevCommand::FromAscii("FF 00 51 C3 00"), _recvBuffer))
            {
                if(!_apdu(DevCommand::FromAscii("E0 20 00 00 01 C3"), _recvBuffer))
                    return _logRetValue(false);
            }
            break;
        default:
            break;
        }

        return _logRetValue(true);
    }
    /// 蜂鸣器控制
    bool Beep(byte times = 1)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "Times:<" << static_cast<int>(times) << ">\n");

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 00 42 00 03 00 02 01", _sendBuffer);
        _sendBuffer[5] = times;
        return _logRetValue(_apdu(_sendBuffer, _recvBuffer));
    }
    /// 获取非接卡片UID
    bool GetContactlessUID(ByteBuilder& uid)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF CA 00 00 00", _sendBuffer);
        ASSERT_FuncErrRet(_apdu(_sendBuffer, _recvBuffer), DeviceError::OperatorErr);

        LOGGER(_log << "UID:<";
        _log.WriteStream(_recvBuffer) << ">\n");

        uid += _recvBuffer;

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace lc
} // namespace application
} // namespace zhou_yb
//========================================================= 