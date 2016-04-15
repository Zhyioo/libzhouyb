//========================================================= 
/**@file ComStorageCardDevAdapter.h 
 * @brief 串口存储卡适配器
 * 
 * @date 2014-10-17   16:39:21 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMSTORAGECARDDEVADAPTER_H_
#define _LIBZHOUYB_COMSTORAGECARDDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace device {
//---------------------------------------------------------
/// 串口接触式存储卡设备适配器
class ComContactStorageCardDevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
public:
    //----------------------------------------------------- 
    /// 接触式卡片类型定义 
    enum CardType
    {
        /// 自动识别 
        Card_Auto = 0x00,
        /// AT24C0x ST14C0xC 系列卡 
        Card_AT24C0x_ST14C0xC_GPMxK = 0x01,
        /// Card_45D041卡 
        Card_45D041 = 0x02,
        /// SLE4428 SLE4448
        Card_SLE44x8 = 0x03,
        /// SLE4442 
        Card_SLE44x2 = 0x04,
        /// AT24C32 AT24C64
        Card_AT24C32_64 = 0x05,
        /// 93C64
        Card_93C64 = 0x06,
        /// AT153
        Card_AT153 = 0x07,
        /// AT88SC102卡 
        Card_AT88SC102 = 0x08,
        /// AT88SC1604A
        Card_AT88SC1604 = 0x09,
        /// AT88SC1604B
        Card_AT88SC1604B = 0x0A,
        /// AT88SC1608 
        Card_AT88SC1608 = 0x0B
    };
    //----------------------------------------------------- 
    /// 错误的卡标识位
    static const byte ErrorCardNumber/* = 0xFF*/;
    //----------------------------------------------------- 
protected:
    //-----------------------------------------------------
    /// 卡序号 00H - 0FH
    byte _cardNum;
    /// 发送缓冲区(减小空间分配的性能开销)
    ByteBuilder _sendBuffer;
    /// 接收缓冲区(减小空间分配的性能开销)
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
    /// 返回指定的卡序号是否为有效序号,如果有效则对其进行转换
    virtual bool _isVaildCardNumber(byte& cardNum)
    {
        // 只需要判断，不需要转换
        return (cardNum >= 0x00 && cardNum <= 0x1F);
    }
    /// 初始化缓冲区 
    inline void _init_buffer()
    {
        if(_sendBuffer.GetSize() < 1)
        {
            _sendBuffer.Resize(DEV_BUFFER_SIZE);
            _recvBuffer.Resize(DEV_BUFFER_SIZE);
        }
    }
    /// 初始化数据成员 
    inline void _init()
    {
        _cardNum = ErrorCardNumber;
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    ComContactStorageCardDevAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //-----------------------------------------------------
    /**
     * @brief 设置卡片类型 
     * @param [in] cardNo 卡槽号 
     * @param [in] cardType 需要设置的卡片类型  
     */ 
    bool SetCardType(byte cardNo, CardType cardType)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(
        _log<<"卡类型:<"<<_hex(static_cast<byte>(cardType))<<">\n"
            <<"卡槽号:<"<<_hex(cardNo)<<">\n");

        /* 重置卡槽号为无效标记 */
        _cardNum = ErrorCardNumber;

        /* 验证参数合法性 */
        ASSERT_FuncErrInfoRet(_isVaildCardNumber(cardNo),
            DeviceError::ArgRangeErr, "存储卡卡槽号范围不正确");

        /* 准备发送的指令 */
        _init_buffer();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 31", _sendBuffer);
        _sendBuffer += cardNo;
        _sendBuffer += static_cast<byte>(cardType);

        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        _cardNum = cardNo;

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 自动识别卡类型 
    bool DistinguishCardType(byte cardNo, CardType* pCardType = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"卡槽号:<"<<_hex(cardNo)<<">\n");

        /* 重置卡槽号为无效标记 */
        _cardNum = ErrorCardNumber;

        /* 验证参数合法性 */
        ASSERT_FuncErrInfoRet(_isVaildCardNumber(cardNo),
            DeviceError::ArgRangeErr, "存储卡卡槽号范围不正确");

        /* 准备发送的命令 */
        _init_buffer();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 32", _sendBuffer);
        _sendBuffer += cardNo;

        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        _cardNum = cardNo;
        
        LOGGER(_log << "CardType:<" << _hex(_recvBuffer[0]) << ">\n");
        if(_recvBuffer.GetLength() > 0 && NULL != pCardType)
        {
            *pCardType = static_cast<CardType>(_recvBuffer[0]);
        }

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 接触式存储卡密钥初始化 
    bool KeyInit(const ByteArray& keyBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        /* 验证卡槽号 */
        ASSERT_FuncErrInfoRet(_isVaildCardNumber(_cardNum), 
            DeviceError::ArgErr, "未初始化卡槽");

        /* 验证密钥长度(255以内) */
        ASSERT_FuncErrInfoRet(keyBuff.GetLength() <= static_cast<byte>(-1), 
            DeviceError::ArgRangeErr, "密钥长度错误(0-255)");

        /* 准备命令 */
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 33", _sendBuffer);
        byte len = _itobyte(keyBuff.GetLength());
        _sendBuffer += _cardNum;
        _sendBuffer += len;
        _sendBuffer.Append(keyBuff.SubArray(0, len));

        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 接触式存储卡密码校验 
    bool VerifyKey(const ByteArray& keyBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        /* 验证卡槽号 */
        ASSERT_FuncErrInfoRet(_isVaildCardNumber(_cardNum), 
            DeviceError::ArgErr, "未初始化卡槽");

        /* 验证密钥长度(255以内) */
        ASSERT_FuncErrInfoRet(keyBuff.GetLength() <= static_cast<byte>(-1), 
            DeviceError::ArgRangeErr, "密钥长度错误(0-255]");

        /* 准备命令 */
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 34", _sendBuffer);
        _sendBuffer += _cardNum;
        byte len = _itobyte(keyBuff.GetLength());
        _sendBuffer += len;
        _sendBuffer.Append(keyBuff.SubArray(0, len));

        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /**
     * @brief 接触式存储卡读数据
     * @param [in] adr 读取数据的起始地址 
     * @param [in] len 读取数据的长度 
     * @param [out] readBuff 读取数据的缓冲区 
     */
    bool ReadMemory(ushort adr, ushort len, ByteBuilder& readBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"读取数据的起始地址:<"<<adr<<">,读取长度:<"<<len<<">\n");

        /* 验证卡槽号 */
        ASSERT_FuncErrInfoRet(_isVaildCardNumber(_cardNum), 
            DeviceError::ArgErr, "未初始化卡槽");

        /* 准备命令 */
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 35", _sendBuffer);
        _sendBuffer += _cardNum;
        // 取地址字节 
        ByteBuilder lenBuff(2);
        ByteConvert::FromObject(adr, lenBuff);

        // 地址高低字节是反序的 
        _sendBuffer += lenBuff[1];
        _sendBuffer += lenBuff[0];
        ByteConvert::FromObject(len, _sendBuffer);

        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        LOGGER(
        _log.WriteLine("读取到数据:");
        _log.WriteLine(_recvBuffer));

        readBuff += _recvBuffer;

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /**
     * @brief 接触式存储卡写数据
     * @param [in] adr 写入数据的起始地址 
     * @param [in] wrtBuff 写入的数据 
     */
    bool WriteMemory(ushort adr, const ByteArray& wrtBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"写入数据的起始地址:<"<<adr<<">\n";
        _log.WriteLine("写入数据:");
        _log.WriteLine(wrtBuff));

        /* 验证卡槽号 */
        ASSERT_FuncErrInfoRet(_isVaildCardNumber(_cardNum), 
            DeviceError::ArgErr, "未初始化卡槽");

        /* 准备命令 */
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 36", _sendBuffer);
        _sendBuffer += _cardNum;
        // adr 采用 ushort 刚好 2 字节，命令要求是 2 字节长度，方便用FromObject直接转
        ByteBuilder lenBuff(2);
        ByteConvert::FromObject(adr, lenBuff);

        // 地址高低字节是反序的 
        _sendBuffer += lenBuff[1];
        _sendBuffer += lenBuff[0];

        ByteConvert::FromObject(static_cast<ushort>(wrtBuff.GetLength()), _sendBuffer);
        _sendBuffer += wrtBuff;
        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //-----------------------------------------------------
};
//---------------------------------------------------------
/// 串口非接触式存储卡设备适配器 
class ComContactlessStorageCardDevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
protected:
    //----------------------------------------------------- 
    /// 是否已经激活 
    bool _hasActive;
    /// 收发数据自备缓冲区 
    ByteBuilder _sendBuff;
    ByteBuilder _recvBuff;
    /// 初始化数据成员 
    inline void _init()
    {
        _hasActive = false;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ComContactlessStorageCardDevAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //----------------------------------------------------- 
    /** 
     * @brief 激活非接触式存储卡 
     * @param [in] delayTime 等待时间 
     * @param [out] pType [default:NULL] 返回的卡类型(0x0A:A卡,0x0B:B卡) 
     * @param [out] pUid [default:NULL] 返回的卡片的UID标识 
     */ 
    bool ActiveStorageCard(ushort delayTime, byte* pType = NULL, ByteBuilder* pUid = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"等待放卡时间:<"<<delayTime<<">\n");

        /* 准备发送的命令 */
        if(_sendBuff.GetSize() < 1)
        {
            _sendBuff.Resize(DEV_BUFFER_SIZE);
            _recvBuff.Resize(DEV_BUFFER_SIZE);
        }

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 41", _sendBuff);
        ByteConvert::FromObject(delayTime, _sendBuff);

        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        /* 需要获取卡类型和卡片UID */
        if((NULL != pType) || (NULL != pUid))
        {
            LOGGER(_log<<"收到的数据长度:<"<<_recvBuff.GetLength()<<">\n");

            ASSERT_FuncErrInfoRet(_recvBuff.GetLength() >= 1, 
                DeviceError::RecvFormatErr, "接收到的数据长度小于1字节");

            if(NULL != pType)
            {
                (*pType) = _recvBuff[0];
            }
            if(NULL != pUid)
            {
                // UID 长度
                (*pUid).Append(_recvBuff.SubArray(1));
            }
        }

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡认证扇区 
     * @param [in] sectorIndex 扇区号 
     * @param [in] pinType 密钥类型(0x60或0x61) 
     * @param [in] pin 密钥(6字节) 
     */ 
    bool AuthSector(byte sectorIndex, byte pinType, const ByteArray& pin)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"扇区号:<"<<sectorIndex<<">,密钥类型:<"<<pinType<<">,密钥:<"<<pin<<">\n");

        ASSERT_FuncErrInfoRet(pin.GetLength() == 6,
            DeviceError::ArgFormatErr, "密钥长度不为6字节");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 42", _sendBuff);
        _sendBuff += sectorIndex;
        _sendBuff += pinType;
        _sendBuff += pin;

        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡读块  
     * @param [in] blockIndex 块号
     * @param [out] recvBuff 读到的数据 
     */ 
    bool ReadBlock(byte blockIndex, ByteBuilder& recvBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"块号:<"<<blockIndex<<">\n");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 43",  _sendBuff);
        _sendBuff += blockIndex;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        recvBuff += _recvBuff;

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡写块 
     * @param [in] blockIndex 块号  
     * @param [in] wrtBuff 写入的数据(16字节) 
     */ 
    bool WriteBlock(byte blockIndex, const ByteArray& wrtBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"块号:<"<<blockIndex<<">\n"
            <<"写入的数据:<"<<wrtBuff<<">\n");

        ASSERT_FuncErrInfoRet(wrtBuff.GetLength() == 16,
            DeviceError::ArgFormatErr, "写入数据的长度不为16字节");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 44", _sendBuff);
        _sendBuff += blockIndex;
        _sendBuff += wrtBuff;

        /* 交换数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡读值块 
     * @param [in] blockIndex 块号 
     * @param [out] readBuff 读取出来的数据 
     */ 
    bool ReadValueBlock(byte blockIndex, ByteBuilder& readBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"值块号:<"<<blockIndex<<">\n");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 45", _sendBuff);
        _sendBuff += blockIndex;

        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        /* 保存数据 */
        readBuff += _recvBuff;

        return _logRetValue(true);
    }  
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡写值块 
     * @param [in] blockIndex 块号 
     * @param [in] wrtBuff 写入的数据(4字节) 
     */ 
    bool WriteValueBlock(byte blockIndex, const ByteArray& wrtBuff)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"值块号:<"<<blockIndex<<">\n"
            <<"写入数据:<"<<wrtBuff<<">\n");

        ASSERT_FuncErrInfoRet(wrtBuff.GetLength() == 4,
            DeviceError::ArgFormatErr, "写入的值块的数据不为4字节");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 46", _sendBuff);
        _sendBuff += blockIndex;
        _sendBuff += wrtBuff;

        /* 发送数据 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡加值 
     * @param [in] blockIndex 块号
     * @param [in] valBlock 新的数据值 
     */ 
    bool IncreaseValueBlock(byte blockIndex, const ByteArray& valBlock)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"块号:<"<<blockIndex<<">\n"
            <<"写入数据:<"<<valBlock<<">\n");

        ASSERT_FuncErrInfoRet(valBlock.GetLength() == 4,
            DeviceError::ArgFormatErr, "加值块长度不为4字节");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 47", _sendBuff);
        _sendBuff += blockIndex;
        _sendBuff += valBlock;

        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡减值 
     * @param [in] blockIndex 块号 
     * @param [in] valBlock 新的数据值 
     */ 
    bool DecreaseValueBlock(byte blockIndex, const ByteArray& valBlock)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"块号:<"<<blockIndex<<">\n"
            <<"写入数据:<"<<valBlock<<">\n");

        ASSERT_FuncErrInfoRet(valBlock.GetLength() == 4,
            DeviceError::ArgFormatErr, "减值块长度不为4字节");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 48", _sendBuff);
        _sendBuff += blockIndex;
        _sendBuff += valBlock;

        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /** 
     * @brief 非接触式存储卡写主密钥 
     * @param [in] keyMode 密钥模式 
     * @param [in] sectorIndex 扇区号 
     * @param [in] key 密钥(6字节) 
     */ 
    bool WriteMainKey(byte keyMode, byte sectorIndex, const ByteArray& key)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"密钥模式:<"<<_hex(keyMode)<<">,扇区号:<"<<sectorIndex<<">\n"
            <<"密钥:<"<<key<<">\n");

        ASSERT_FuncErrInfoRet(key.GetLength() == 6,
            DeviceError::ArgFormatErr, "写主密钥的密钥长度不为6字节");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 49", _sendBuff);
        _sendBuff += keyMode;
        _sendBuff += sectorIndex;
        _sendBuff += key;

        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /**
     * @brief 设置卡片类型 
     * @param [in] cardType 需要设置的卡片类型 
     */ 
    bool SetCardType(byte cardType)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"卡类型:<"<<_hex(cardType)<<">\n");

        /* 准备命令 */
        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("32 4A", _sendBuff);
        _sendBuff += cardType;

        /* 发送命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//---------------------------------------------------------
} // namespace device
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMSTORAGECARDDEVADAPTER_H_
//=========================================================
