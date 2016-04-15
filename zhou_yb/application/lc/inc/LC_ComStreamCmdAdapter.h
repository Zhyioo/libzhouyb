//========================================================= 
/**@file LC_ComStreamCmdAdapter.h
 * @brief 
 * 
 * @date 2016-01-04   17:02:26
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_COMSTREAMCMDADAPTER_H_
#define _LIBZHOUYB_LC_COMSTREAMCMDADAPTER_H_
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
    LC_ComStreamCmdAdapter() : DevAdapterBehavior() { IsCheckLRC = true; }
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
        LOGGER(_log<<"Recv:"<<_tmpBuffer<<endl);
        ASSERT_FuncErrRet(StringConvert::StartWith(_tmpBuffer, ACK_COMMAND), DeviceError::RecvFormatErr);
        _tmpBuffer.RemoveFront(ACK_COMMAND.GetLength());
        LOGGER(if(!_tmpBuffer.IsEmpty())
        {
            _log<<"Last:"<<_tmpBuffer<<endl;
        });

        return _logRetValue(true);
    }
    /// 接收数据 
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        bool isContinue = true;
        while(isContinue)
        {
            // 接收的长度不正确 
            if(_tmpBuffer.GetLength() < 3)
            {
                ASSERT_FuncErrRet(_pDev->Read(_tmpBuffer), DeviceError::RecvErr);
                continue;
            }
            
            // 数据头不正确
            ASSERT_FuncErrRet(_tmpBuffer[0] == ACK_STX, DeviceError::RecvFormatErr);
            
            // 检查长度位
            size_t len = _tmpBuffer[1];
            // 80 + LEN + XX + LRC 3字节
            size_t fulllen = _tmpBuffer.GetLength() - 3;
            // 数据没有接收全 
            if(len > fulllen)
            {
                // 接收应答
                ASSERT_FuncErrRet(_pDev->Read(_tmpBuffer), DeviceError::RecvErr);
                continue;
            }

            isContinue = false;
            // 截掉多余的数据 
            fulllen = len + 3;
            LOGGER(_log << "RECV:" << _tmpBuffer << endl);
            _tmpBuffer.RemoveTail(_tmpBuffer.GetLength() - fulllen);
            LOGGER(_log << "Remove Len:<" << _tmpBuffer.GetLength() - fulllen << ">\n");

            ByteArray recvBuff = _tmpBuffer.SubArray(2, len);
            if(IsCheckLRC)
            {
                // 比对LRC
                byte recvLrc = _tmpBuffer[_tmpBuffer.GetLength() - 1];
                _tmpBuffer.RemoveTail();
                byte lrc = ByteConvert::XorVal(_tmpBuffer);

                LOGGER(_log << "LRC:<" << _hex(lrc) << ">, RecvLRC:<" << _hex(recvLrc) << ">\n");
                ASSERT_FuncErrRet(lrc == recvLrc, DeviceError::DevVerifyErr);
            }

            data += recvBuff;
            _tmpBuffer.Clear();
        }
        // 回复应答包
        ASSERT_FuncErrRet(_pDev->Write(ACK_COMMAND), DeviceError::SendErr);

        return _logRetValue(true);
    }
    /**
     * @brief 是否校验LRC
     */
    bool IsCheckLRC;
};
//--------------------------------------------------------- 
} // namespace lc
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_COMSTREAMCMDADAPTER_H_
//========================================================= 

