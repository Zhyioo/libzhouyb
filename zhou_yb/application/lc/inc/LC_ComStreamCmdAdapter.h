//========================================================= 
/**@file LC_ComStreamCmdAdapter.h
 * @brief 
 * 
 * @date 2016-01-04   17:02:26
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace lc {
//--------------------------------------------------------- 
/// LC流指令数据接收器
class LC_ComStreamCmdAdapter :
    public IInteractiveTrans,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    /// ACK应答指令
    static const byte _ACK_COMMAND[4];
public:
    /// ACK应答数据
    static const ByteArray ACK_COMMAND;
    /// 包头
    static const byte ACK_STX;
protected:
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
public:
    /// 发送数据 
    virtual bool Write(const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _tmpBuffer.Clear();
        _tmpBuffer += ACK_STX;
        _tmpBuffer += _itobyte(data.GetLength());
        _tmpBuffer += data;

        byte lrc = ByteConvert::XorVal(_tmpBuffer);
        _tmpBuffer += lrc;

        ASSERT_FuncErrRet(_pDev->Write(_tmpBuffer), DeviceError::SendErr);
        _tmpBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_tmpBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_tmpBuffer.IsEqual(ACK_COMMAND), DeviceError::RecvFormatErr);

        return _logRetValue(true);
    }
    /// 接收数据 
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _tmpBuffer.Clear();
        bool isContinue = true;
        while(isContinue)
        {
            ASSERT_FuncErrRet(_pDev->Read(_tmpBuffer), DeviceError::RecvErr);
            // 接收的长度不正确 
            if(_tmpBuffer.GetLength() < 2)
                continue;
            // 数据头不正确 
            ASSERT_FuncErrRet(_tmpBuffer[0] == ACK_STX, DeviceError::RecvFormatErr);
            
            // 检查长度位
            size_t len = _tmpBuffer[1];
            // 80 + LEN + XX + LRC 3字节
            size_t fulllen = _tmpBuffer.GetLength() - 3;
            // 数据没有接收全 
            if(len > fulllen)
                continue;

            isContinue = false;
            // 截掉多余的数据 
            fulllen = len + 3;
            _tmpBuffer.RemoveTail(_tmpBuffer.GetLength() - fulllen);

            LOGGER(_log << "RECV:<" << ArgConvert::ToString(_tmpBuffer) << ">\n");

            // 比对LRC
            ByteArray recvBuff = _tmpBuffer.SubArray(2, len);
            byte recvLrc = _tmpBuffer[_tmpBuffer.GetLength() - 1];
            _tmpBuffer.RemoveTail();
            byte lrc = ByteConvert::XorVal(_tmpBuffer);

            LOGGER(_log << "LRC:<" << _hex(lrc) << ">, RecvLRC:<" << _hex(recvLrc) << ">\n");
            ASSERT_FuncErrRet(lrc == recvLrc, DeviceError::DevVerifyErr);

            data += recvBuff;
        }

        // 应答包 
        ASSERT_FuncErrRet(_pDev->Write(ACK_COMMAND), DeviceError::SendErr);
        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace lc
} // namespace application 
} // namespace zhou_yb
//========================================================= 
