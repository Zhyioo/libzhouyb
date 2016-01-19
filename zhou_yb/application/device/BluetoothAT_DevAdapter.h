//========================================================= 
/**@file BluetoothAT_DevAdapter.h
 * @brief 蓝牙AT指令集 
 * 
 * @date 2015-09-13   11:00:32
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace device {
//--------------------------------------------------------- 
/// 蓝牙设备AT控制指令集 
class BluetoothAT_DevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
protected:
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区 
    ByteBuilder _recvBuffer;
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
    /// 发送数据 
    inline bool _Write(ByteBuilder& data)
    {
        data += "\r\n";
        return _pDev->Write(data);
    }
    /// 接收一行数据 
    bool _Read(ByteBuilder& data)
    {
        // 当前接收到的数据 
        size_t index = 0;
        ByteBuilder* pBuff = &_tmpBuffer;
        // 需要读下一个包 
        size_t rlen = 0;
        bool has0D = false;
        do
        {
            // 查找0D 
            rlen = pBuff->GetLength();
            for(;index < rlen; ++index)
            {
                if((*pBuff)[index] == 0x0D)
                {
                    has0D = true;
                    break;
                }
            }
            // 找到结束符 
            if(has0D && (index < rlen - 1) && (*pBuff)[index + 1] == 0x0A)
            {
                if(pBuff == &data)
                {
                    _tmpBuffer.Clear();

                    rlen = data.GetLength() - index;
                    _tmpBuffer.Append(data.SubArray(index + 2, rlen - 2));

                    data.RemoveTail(rlen);
                }
                else
                {
                    data.Append(_tmpBuffer.SubArray(0, index));
                    _tmpBuffer.RemoveFront(index + 2);
                }
                return true;
            }
            else
            {
                pBuff = &data;
                data += _tmpBuffer;
                _tmpBuffer.Clear();
            }
        } while(_pDev->Read(data));

        return false;
    }
public:
    /// 读数据行 
    bool ReadLine(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(size_t lastLen = data.GetLength());
        bool bRet = _Read(data);
        LOGGER(if(bRet)
        {
            _log << "ReadLine:" << data.GetString() + lastLen << endl;
        });
        return _logRetValue(bRet);
    }
    /// 清空缓冲区 
    inline void Clean()
    {
        _tmpBuffer.Clear();
    }
    /// 测试是否连接 
    bool IsConnected()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _sendBuffer += "AT";

        ASSERT_FuncErrRet(_Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /// 获取本地蓝牙地址 
    bool GetLocalAddress(ByteBuilder& adr_6)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _sendBuffer += "AT+LADDR";
        ASSERT_FuncErrRet(_Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_Read(_recvBuffer), DeviceError::RecvErr);

        adr_6 += _tmpBuffer;

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace device
} // namespace application
} // namespace zhou_yb
//========================================================= 