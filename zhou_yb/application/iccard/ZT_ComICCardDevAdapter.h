//========================================================= 
/**@file ZT_ComICCardCmdAdapter.h
 * @brief 证通IC卡指令集
 * 
 * @date 2016-05-11   14:06:41
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ZT_COMICCARDCMDADAPTER_H_
#define _LIBZHOUYB_ZT_COMICCARDCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace application {
namespace iccard {
//--------------------------------------------------------- 
/// 证通IC卡指令集
class ZT_ComICCardCmdAdapter : 
    public IInteractiveTrans,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    /// 临时缓冲区
    ByteBuilder _tmpBuffer;
    /// 状态错误码
    byte _swCode;
    inline bool _IsSuccessRecv(const ByteArray& data)
    {
        return data.GetLength() > 0 && data[0] == 0x00;
    }
    bool _PackSendData(const ByteArray& cmd, ByteBuilder& send)
    {
        byte cmdSTX[2] = { 0x1B, '=' };
        send.Append(ByteArray(cmdSTX, 2));
        ByteConvert::ToAscii(cmd, send);

        byte xor = ByteConvert::XorVal(cmd);
        ByteConvert::ToAscii(xor, send);
        send += 0x03;
        return true;
    }
    bool _UnPackRecvData(const ByteArray& recv, ByteBuilder& data)
    {
        const size_t CMD_LENGTH = 3;
        const byte OK_CODE = 0x00;

        byte cmdSTX[2] = { 0x1B, '=' };
        if(recv.GetLength() < CMD_LENGTH)
        {
            _logErr(DeviceError::ArgLengthErr);
            return false;
        }
        if(!StringConvert::StartWith(recv, ByteArray(cmdSTX, 2)))
        {
            _logErr(DeviceError::ArgFormatErr);
            return false;
        }
        // 检查标识长度和实际长度是否一致
        ByteBuilder tmp(32);
        ByteConvert::FromAscii(recv.SubArray(2, recv.GetLength() - CMD_LENGTH), tmp);
        // 长度位
        if(tmp.GetLength() < 1)
        {
            _logErr(DeviceError::RecvFormatErr);
            return false;
        }

        size_t len = tmp[0];
        if((len + 1) < tmp.GetLength())
        {
            _logErr(DeviceError::ArgLengthErr);
            return false;
        }
        byte devXor = tmp[tmp.GetLength() - 1];
        tmp.RemoveTail();
        byte xor = ByteConvert::XorVal(tmp);
        if(xor != devXor)
        {
            _logErr(DeviceError::DevVerifyErr, "校验和错误");
            return false;
        }

        _swCode = tmp[1];
        if(_swCode != OK_CODE)
        {
            _logErr(DeviceError::DevReturnErr);
            return false;
        }
        data += tmp.SubArray(2);
        return true;
    }
public:
    /// 按照格式接收数据 
    static bool RecvByFormat(IInteractiveTrans& dev, ByteBuilder& emptyBuffer)
    {
        // 成功：ESC = 02 00 02 ETX
        // 失败：ESC = 02 24 26 ETX

        // 先收取3个字节
        byte cmdSTX[2] = { 0x1B, '=' };

        const size_t CMD_LENGTH = 4;
        while(emptyBuffer.GetLength() < CMD_LENGTH)
        {
            if(!dev.Read(emptyBuffer))
                return false;
        }
        if(!StringConvert::StartWith(emptyBuffer, ByteArray(cmdSTX, 2)))
            return false;

        ByteBuilder tmp(8);
        ByteConvert::FromAscii(emptyBuffer.SubArray(2, 2), tmp);
        size_t len = 2 * tmp[0];
        size_t totallen = len + CMD_LENGTH + 1;
        while(emptyBuffer.GetLength() < totallen)
        {
            if(!dev.Read(emptyBuffer))
                return false;
        }
        if(totallen < emptyBuffer.GetLength())
            emptyBuffer.RemoveTail(emptyBuffer.GetLength() - totallen);
        return true;
    }
    /// 发数据 
    virtual bool Write(const ByteArray& data)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("Send:");
        _log.WriteLine(data));

        if(_tmpBuffer.GetSize() < 1)
        {
            _tmpBuffer.Resize(DEV_BUFFER_SIZE);
        }
        _tmpBuffer.Clear();

        /* 把命令组包 */
        ASSERT_FuncErrRet(_PackSendData(data, _tmpBuffer), DeviceError::ArgFormatErr);
        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_tmpBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    /// 收数据 
    virtual bool Read(ByteBuilder& data)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        _tmpBuffer.Clear();

        /* 获取数据 */
        bool bRead = RecvByFormat(_pDev, _tmpBuffer);
        LOGGER(_log.WriteLine("Read FormatCmd:").WriteLine(_tmpBuffer));
        ASSERT_FuncErrRet(bRead, DeviceError::RecvFormatErr);
        ASSERT_FuncErrRet(_UnPackRecvData(_tmpBuffer, data), DeviceError::RecvFormatErr);

        LOGGER(_log.WriteLine("Recv:");
        _log.WriteLine(data));

        return _logRetValue(true);
    }
    /// 获取错误状态码
    inline byte GetStatusCode()
    {
        return _swCode;
    }
};
//--------------------------------------------------------- 
/// 证通IC卡驱动
class ZT_ComICCardDevAdapter :
    public ICCardDevice,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
public:
    enum CardType
    {
        Card_SLE_4418 = 0x18,
        Card_SLE_4428 = 0x28,
        Card_SLE_4432 = 0x32,
        Card_SLE_4442 = 0x42,
        Card_CPU = 0x88
    };
protected:
    /// 发送缓冲区(减小空间分配的性能开销)
    ByteBuilder _sendBuffer;
    /// 接收缓冲区(减小空间分配的性能开销)
    ByteBuilder _recvBuffer;
    /// 是否上电标识位 
    bool _hasPowerOn;
public:
    ZT_ComICCardDevAdapter() : DevAdapterBehavior() { _hasPowerOn = false; }
    virtual ~ZT_ComICCardDevAdapter() { PowerOff(); }
    /**
     * @brief 选卡座
     * @date 2016-05-11 14:31
     * 
     * @param [in] slot 卡座
     * - 参数:
     *  - 0x01 顶卡座
     *  - 0x02 侧卡座
     * .
     */
    bool SelectSLOT(byte slot)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        _sendBuffer += 0x25;
        _sendBuffer += slot;
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /// 选卡类型
    bool SelectCardType(CardType cardType)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        _sendBuffer += 0x54;
        _sendBuffer += cardType;
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /// 测卡状态
    bool DistinguishCard(byte* pSlot, byte* pCardType, byte* pStatus)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        _sendBuffer.Append(0x47, 2);
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 3, DeviceError::ArgLengthErr);
        if(pSlot != NULL)
        {
            (*pSlot) = _recvBuffer[0];
        }
        if(pCardType != NULL)
        {
            (*pCardType) = _recvBuffer[1];
        }
        if(pStatus != NULL)
        {
            (*pStatus) = _recvBuffer[2];
        }

        return _logRetValue(true);
    }
    /// 卡片上电
    bool ActiveCard(ByteBuilder* pAtr = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        _sendBuffer.Append(0x55, 2);
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "ATR:";
        _log.WriteStream(_recvBuffer) << endl);

        if(pAtr != NULL)
        {
            (*pAtr) += _recvBuffer;
        }

        return _logRetValue(true);
    }
    virtual bool PowerOn(const char*readerName = NULL, ByteBuilder*pAtr = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        PowerOff();

        byte slot = 0x01;
        byte cardType = Card_CPU;

        ArgParser cfg;
        if(cfg.Parse(readerName))
        {
            ArgConvert::FromConfig<byte>(cfg, "SLOT", slot);
            ArgConvert::FromConfig<byte>(cfg, "CardType", cardType);
        }
        else
        {
            if(!_is_empty_or_null(readerName))
            {
                slot = _itobyte(ArgConvert::FromString<int>(readerName));
            }
        }
        
        ASSERT_FuncInfoRet(SelectSLOT(slot), "选卡座失败");
        ASSERT_FuncInfoRet(SelectCardType(static_cast<CardType>(cardType)), "选卡类型失败");
        ASSERT_FuncInfoRet(ActiveCard(pAtr), "卡座上电失败");

        _hasPowerOn = true;
        return _logRetValue(true);
    }
    virtual bool PowerOff()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        if(!HasPowerOn())
            return _logRetValue(true);

        _hasPowerOn = false;
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        _sendBuffer.Append(0x44, 2);
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        return _logRetValue(true);
    }
    virtual bool HasPowerOn() const
    {
        return _hasPowerOn;
    }
    virtual bool TransCommand(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ASSERT_FuncErr(!sendBcd.IsEmpty(), DeviceError::ArgIsNullErr);
        ASSERT_FuncErr(HasPowerOn(), DeviceError::DevStateErr);

        LOGGER(_log.WriteLine("Send Apdu :");
        _log.WriteLine(sendBcd));

        _sendBuffer.Clear();

        _sendBuffer += 0x5A;
        _sendBuffer += sendBcd;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(recvBcd), DeviceError::RecvErr);

        LOGGER(_log.WriteLine("Recv Apdu :");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        /* 验证是否可操作 */
        ASSERT_Device();

        ASSERT_FuncErr(HasPowerOn(), DeviceError::DevStateErr);

        LOGGER(_log.WriteLine("发送 Apdu:");
        _log.WriteLine(sendBcd));

        ASSERT_FuncErrInfoRet(ICCardDevice::Apdu(sendBcd, recvBcd),
            DeviceError::TransceiveErr, "交换Apdu失败");

        LOGGER(_log.WriteLine("接收 Apdu:");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace iccard
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ZT_COMICCARDCMDADAPTER_H_
//========================================================= 