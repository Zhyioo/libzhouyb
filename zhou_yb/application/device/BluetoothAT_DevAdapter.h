//========================================================= 
/**@file BluetoothAT_DevAdapter.h
 * @brief 蓝牙AT指令集 
 * 
 * @date 2015-09-13   11:00:32
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BLUETOOTHAT_DEVADAPTER_H_
#define _LIBZHOUYB_BLUETOOTHAT_DEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace device {
//--------------------------------------------------------- 
/// 蓝牙设备AT指令适配器
class BluetoothAT_CmdAdapter : 
    public IInteractiveTrans,
    public ITransceiveTrans,
    public BaseDevAdapterBehavior<IInteractiveTrans>
{
protected:
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区 
    ByteBuilder _recvBuffer;
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
public:
    /// 读取一行AT数据
    virtual bool Read(ByteBuilder& data)
    {
        ASSERT_Func(IsValid());
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
    /// 发送AT指令
    virtual bool Write(const ByteArray& data)
    {
        ASSERT_Func(IsValid());
        Clean();

        _sendBuffer.Clear();
        _sendBuffer += data;
        _sendBuffer += "\r\n";
        return _pDev->Write(_sendBuffer);
    }
    /// 交互一次AT指令
    virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv)
    {
        ASSERT_Func(IsValid());
        ASSERT_Func(Write(send));
        const char ok[] = "OK";
        size_t len = 0;
        _recvBuffer.Clear();
        while(Read(_recvBuffer))
        {
            if(StringConvert::Compare(_recvBuffer, ByteArray(ok, 2)))
                return true;
            recv += _recvBuffer;
            len += _recvBuffer.GetLength();
            _recvBuffer.Clear();
        }
        recv.RemoveTail(len);
        return false;
    }
    /// 清空缓冲区 
    inline void Clean()
    {
        _tmpBuffer.Clear();
    }
};
//--------------------------------------------------------- 
/// 蓝牙设备AT控制指令集 
class BluetoothAT_DevAdapter : public DevAdapterBehavior<ITransceiveTrans>
{
public:
    /// 测试是否连接 
    bool IsConnected()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ByteBuilder tmp(8);
        ASSERT_FuncErrRet(_pDev->TransCommand("AT", tmp), DeviceError::TransceiveErr);

        return _logRetValue(true);
    }
    /// 获取本地蓝牙地址 
    bool GetLocalAddress(ByteBuilder& adr_6)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ASSERT_FuncErrRet(_pDev->TransCommand("AT+LADDR", adr_6), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
/// BLE蓝牙4.0设备AT控制指令集
class BluetoothBleAT_DevAdapter : public DevAdapterBehavior<ITransceiveTrans>
{
public:
    /// 获取蓝牙名称
    bool GetName()
    {
    }
    /// 设置蓝牙名称
    bool SetName()
    {
    }
    /// 获取蓝牙BLE名称
    bool GetBleName()
    {
    }
    /// 设置蓝牙BLE名称
    bool SetBleName()
    {
    }
    /// 获取配对密码
    bool GetPin()
    {
    }
    /// 设置配对密码
    bool SetPin()
    {

    }
};
//--------------------------------------------------------- 
} // namespace device
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BLUETOOTHAT_DEVADAPTER_H_
//========================================================= 