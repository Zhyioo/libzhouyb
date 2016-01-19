//=========================================================
/**@file BOC_ComICCardDevAdapter.h
 * @brief 中行莱克指令集三合一IC卡读写器驱动操作逻辑 
 */
#pragma once
//---------------------------------------------------------
#include "../../include/Base.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace application {
namespace iccard {
//--------------------------------------------------------- 
/// 接触式读卡器  
class BOC_ComContactICCardDevAdapter :
    public ICCardDevice,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    /// 发送缓冲区(减小空间分配的性能开销)
    ByteBuilder _sendBuffer;
    /// 接收缓冲区(减小空间分配的性能开销)
    ByteBuilder _recvBuffer;
    /// 是否上电标识位 
    bool _hasPowerOn;
    /// 上电命令 
    byte _poweron_cmd;
    /// 下电命令 
    byte _poweroff_cmd;
    /// 选择并定义卡片类型 
    byte _setcardtype_cmd;
    /// Apdu命令 
    byte _apdu_cmd;
    /// 初始化IC卡命令参数 
    void _init_cmd()
    {
        _poweron_cmd = 0x12;
        _poweroff_cmd = 0x11;
        _apdu_cmd = 0x15;
        _setcardtype_cmd = 0x02;
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    /// 构造发送的数据  
    static void PackSendCmd(const byte cmd, const ByteArray& message, ByteBuilder& cmdBuff)
    {
        ByteBuilder tmp;
        // ACK 命令头 
        tmp += 0x60;
        // LN
        tmp += message.GetLength() + 1;
        // CMD
        tmp += cmd;
        // MESSAGE
        tmp += message;
        byte xor = ByteConvert::XorVal(tmp);
        // LRC
        tmp += xor;
        
        ByteConvert::ToAscii(tmp, cmdBuff);
        cmdBuff += 0x03;
    }
    /// 解析接收的数据(返回状态字节) 
    static const byte UnpackRecvData(const ByteArray& recvBuff, ByteBuilder* pData = NULL)
    {
        const byte CMD_ERROR = -1;
        // 60 01 E5 6D 00 EB 0x03
        if(recvBuff.GetLength() < 7)
            return CMD_ERROR;
        ByteBuilder tmp(recvBuff.GetLength() / 2);
        ByteConvert::FromAscii(recvBuff, tmp);
        // 0x60 0x01 0xE5 0x6D 0x00 0xEB 末尾的0x03自动丢弃
        if(tmp.GetLength() < 4)
            return CMD_ERROR;
        
        byte status = tmp[2];
        if(pData != NULL)
        {
            // 删除状态字 
            pData->Append(tmp.SubArray(3, tmp.GetLength() - 4));
        }
        return status;
    }
public:
    //-----------------------------------------------------
    BOC_ComContactICCardDevAdapter() : DevAdapterBehavior<IInteractiveTrans>()
    {
        _hasPowerOn = false;
        _init_cmd();
    }
    virtual ~BOC_ComContactICCardDevAdapter()
    {
        PowerOff();
    }
    //-----------------------------------------------------
    enum BOC_ComICCardErr
    {
        /// 读卡器操作成功  
        ReaderSuccess = DeviceError::ENUM_MAX(ErrorCode),
        /// 卡片未上电  
        CardNoPowerOnErr,
        /// 不支持的卡片 
        NotSupportCardErr,
        /// 参数个数错误 
        ArgCountErr,
        /// 返回的数据溢出 
        RetMsgOutOfRangeErr,
        /// 通讯协议错误 
        CommunicationProtocolErr,
        /// 卡片复位应答错误 
        CardResetRetErr,
        /// 读字节错误 
        ReadByteErr,
        /// 卡片已下电 
        CardHasPowerOffErr,
        /// APDU长度错误 
        ApduLengthErr,
        /// TCK字节错误 
        TckByteErr,
        /// 非接触卡片无反应 
        ContactlessCardNoRecvErr,
        /// 非接触芯片无反应
        ContactlessChipNoRecvErr,
        /// 非接触芯片复位失败 
        ContactlessChipResetErr,
        /// 非接触通讯传输错误 
        ContactlessTransCmdErr,
        /// 非接触缓冲区溢出 
        ContactlessBuffOutOfRangeErr,
        /// 非接触TYPE_A寻卡失败
        ContactlessTypeAFindCardErr,
        /// 非接触TYPE_B寻卡失败 
        ContactlessTypeBFindCardErr,
        /// 非接触卡片Apdu交换失败
        ContactlessApduErr,
        /// 非接触卡同时找到TypaA,TypeB
        FindTypeAB_CardErr,
        /// 找不到非接触卡 
        NoContactlessCardErr,
        /// 卡片复位应答错误 
        CardResetRecvErr,
        /// 卡片超时通讯中断 
        CardTimeoutErr,
        /// 数据交换过程中出现Parity错误 
        CardParityErr,
        /// 卡片发出无效的过程字节 
        CardSendByteErr,
        /// 卡片返回的SW1,SW2不等于9000H
        SWErr,
        /// 数据交换过程中卡片被意外拔出  
        CardPullOutErr,
        /// 卡片短路 
        CardShortCircuitErr,
        /// 未插卡 
        NoCardErr        
    };
    /// 获取错误码的文字描述
    virtual const char* TransErrToString(int errCode) const
    {
        if(errCode < DeviceError::ENUM_MAX(ErrorCode))
            return DeviceError::TransErrToString(errCode);
        switch(errCode)
        {
        case ReaderSuccess:
            return "读卡器操作成功";
        case CardNoPowerOnErr:
            return "卡片未上电";
        case NotSupportCardErr:
            return "不支持的卡片";
        case ArgCountErr:
            return "参数个数错误";
        case RetMsgOutOfRangeErr:
            return "返回的数据溢出";
        case CommunicationProtocolErr:
            return "通讯协议错误";
        case CardResetRetErr:
            return "卡片复位应答错误";
        case ReadByteErr:
            return "读字节错误";
        case CardHasPowerOffErr:
            return "卡片已下电";
        case ApduLengthErr:
            return "APDU长度错误";
        case TckByteErr:
            return "TCK字节错误";
        case ContactlessCardNoRecvErr:
            return "非接触卡片无反应";
        case ContactlessChipNoRecvErr:
            return "非接触芯片无反应";
        case ContactlessChipResetErr:
            return "非接触芯片复位失败";
        case ContactlessTransCmdErr:
            return "非接触通讯传输错误";
        case ContactlessBuffOutOfRangeErr:
            return "非接触缓冲区溢出";
        case ContactlessTypeAFindCardErr:
            return "非接触TYPE_A寻卡失败";
        case ContactlessTypeBFindCardErr:
            return "非接触TYPE_B寻卡失败";
        case ContactlessApduErr:
            return "非接触卡片Apdu交换失败";
        case FindTypeAB_CardErr:
            return "非接触卡同时找到TypaA,TypeB";
        case NoContactlessCardErr:
            return "找不到非接触卡";
        case CardResetRecvErr:
            return "卡片复位应答错误";
        case CardTimeoutErr:
            return "卡片超时通讯中断";
        case CardParityErr:
            return "数据交换过程中出现Parity错误";
        case CardSendByteErr:
            return "卡片发出无效的过程字节";
        case SWErr:
            return "卡片返回的SW1,SW2不等于9000H";
        case CardPullOutErr:
            return "数据交换过程中卡片被意外拔出";
        case CardShortCircuitErr:
            return "卡片短路";
        case NoCardErr:
            return "未插卡";
        default:
            return "未知的错误";
        }
    }
    static int GetErrorCode(ushort errCode)
    {
        switch(errCode)
        {
        case 0x00:
            return DeviceError::Success;
            //return ReaderSuccess;
        case 0x02:
            return NotSupportCardErr;
        case 0x03:
            return ArgCountErr;
        case 0x05:
            return RetMsgOutOfRangeErr;
        case 0x09:
            return CommunicationProtocolErr;
        case 0x10:
            return CardResetRetErr;
        case 0x13:
            return ReadByteErr;
        case 0x15:
            return CardHasPowerOffErr;
        case 0x1B:
            return ApduLengthErr;
        case 0x1D:
            return TckByteErr;
        case 0x21:
            return ContactlessCardNoRecvErr;
        case 0x22:
            return ContactlessChipNoRecvErr;
        case 0x23:
            return ContactlessChipResetErr;
        case 0x24:
            return ContactlessTransCmdErr;
        case 0x25:
            return ContactlessBuffOutOfRangeErr;
        case 0x26:
            return ContactlessTypeAFindCardErr;
        case 0x27:
            return ContactlessTypeBFindCardErr;
        case 0x28:
            return ContactlessApduErr;
        case 0x2A:
            return FindTypeAB_CardErr;
        case 0x2B:
            return NoContactlessCardErr;
        case 0xA0:
            return CardResetRecvErr;
        case 0xA2:
            return CardTimeoutErr;
        case 0xA3:
            return CardParityErr;
        case 0xA4:
            return CardSendByteErr;
        case 0xE7:
            return SWErr;
        case 0xF7:
            return CardPullOutErr;
        case 0xF8:
            return CardShortCircuitErr;
        case 0xFB:
            return NoCardErr;
        default:
            return DeviceError::UnKnownErr;
        }
    }
    //-----------------------------------------------------
    /**
     * @brief 查询卡状态 
     * @return 卡状态值 
     * @retval -2 错误 
     * @retval -1 无卡 
     * @retval 0 有卡未上电 
     * @retval 1 有卡已上电 
     */
    bool CheckCardStatus(bool* pHasCard = NULL, bool* pHasPowered = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        /* 查询卡状态命令 */
        // 36 30 30 31 31 37 37 36 03
        ByteBuilder cmd;
        cmd = "60011776";
        cmd += 0x03;

        _recvBuffer.Clear();

        bool hasCard = false;
        bool hasPowered = false;

        bool bRet = false;
        if(_pDev->Write(cmd) && _pDev->Read(_recvBuffer))
        {
            _recvBuffer.RemoveTail();

            // 接收到的数据长度不正确(长度至少为4) 
            ASSERT_FuncErrRet(_recvBuffer.GetLength() >= 4, DeviceError::RecvFormatErr);

            cmd.Clear();
            ByteConvert::FromAscii(_recvBuffer, cmd);

            switch(cmd[3])
            {
            // 无卡 
            case 0x00:
                hasCard = false;
                break;
            // 有卡未上电  
            case 0x04:
                hasCard = true;
                break;
            // 有卡已上电 
            case 0x06:
                hasCard = true;
                hasPowered = true;
                break;
            }

            if(pHasCard != NULL)
                (*pHasCard) = hasCard;
            if(pHasPowered != NULL)
                (*pHasPowered) = hasPowered;

            bRet = true;
        }
        // Write(),Read()失败 
        return _logRetValue(bRet);
    }
    //@{
    /**@name 
     * @brief IICCardDevice成员 
     */
    /**
     * @brief 接触式IC卡上电 
     * @param [in] readerName 读卡器名称 
     * @param [out] pAtr 上电返回的ATR信息,默认不需要ATR信息 
     */
    virtual bool PowerOn(const char* readerName = NULL, ByteBuilder* pAtr = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log<<"读卡器参数:<"<<_strput(readerName)<<">\n");

        /* 已经上电则下电 */
        if(HasPowerOn())
        {
            PowerOff();
        }

        /* 上电命令 */
        _sendBuffer.Clear();
        _recvBuffer.Clear();
        // 上电命令 
        PackSendCmd(_poweron_cmd, "", _sendBuffer);
        
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 7, DeviceError::RecvFormatErr);
        /* 解析收到的数据 */
        _sendBuffer.Clear();
        byte status = UnpackRecvData(_recvBuffer, &_sendBuffer);
        ASSERT_FuncErrRet(status == 0x00, GetErrorCode((ushort)status));
        // 取ATR
        if(pAtr != NULL)
        {
            pAtr->Append(_sendBuffer);
        }

        LOGGER(
        _log.WriteLine("ATR:");
        _log.WriteLine(_sendBuffer));

        _hasPowerOn = true;

        return _logRetValue(true);
    }
    /**
     * @brief 发送指令 
     */
    virtual bool TransCommand(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ASSERT_FuncErr(!sendBcd.IsEmpty(), DeviceError::ArgIsNullErr);

        LOGGER(
        _log.WriteLine("Send Apdu :");
        _log.WriteLine(sendBcd));

        _sendBuffer.Clear();
        _recvBuffer.Clear();
        recvBcd.Clear();

        byte status = 0x00;

        if(sendBcd.GetLength() * 2 > 254)
        {
            ByteArray frontBcd = sendBcd.SubArray(0, 254 / 2);
            ByteArray backBcd = sendBcd.SubArray(254 / 2);

            // 先发送APDU数据包中最后超过254字节的部分 
            PackSendCmd(_apdu_cmd, backBcd, _sendBuffer);
            ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
            ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
            status = UnpackRecvData(_recvBuffer, &recvBcd);
            ASSERT_FuncErrRet(status == 0x00, GetErrorCode((ushort)status));
            // 发送第一个254字节块 
            _sendBuffer.Clear();
            _recvBuffer.Clear();
            PackSendCmd(0x1D, frontBcd, _sendBuffer);
            ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
            ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

            status = UnpackRecvData(_recvBuffer, &recvBcd);
            ASSERT_FuncErrRet(status == 0x00 || status == 0xE7, GetErrorCode((ushort)status));
        }
        else 
        {
            PackSendCmd(_apdu_cmd, sendBcd, _sendBuffer);

            ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
            ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

            status = UnpackRecvData(_recvBuffer, &recvBcd);
            ASSERT_FuncErrRet(status == 0x00 || status == 0xE7, GetErrorCode((ushort)status));
        }

        LOGGER(
        _log.WriteLine("Recv Apdu :");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    /**
     * @brief 交换APDU
     */
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        /* 验证是否可操作 */
        ASSERT_Device();

        ASSERT_FuncErr(HasPowerOn(), CardNoPowerOnErr);

        LOGGER(
        _log.WriteLine("发送 Apdu:");
        _log.WriteLine(sendBcd));

        ASSERT_FuncErrInfoRet(ICCardDevice::Apdu(sendBcd, recvBcd), 
            DeviceError::TransceiveErr, "交换Apdu失败");

        LOGGER(
        _log.WriteLine("接收 Apdu:");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    /**
     * @brief 接触式IC卡下电 
     */
    virtual bool PowerOff()
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();
        PackSendCmd(_poweroff_cmd, "", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        byte status = UnpackRecvData(_recvBuffer);
        ASSERT_FuncErrRet(status == 0x00, GetErrorCode((ushort)status));

        _hasPowerOn = false;

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 判断当前是否已经上电 
    inline virtual bool HasPowerOn() const
    {
        return _hasPowerOn;
    }
    //@}
    //-----------------------------------------------------
};
//---------------------------------------------------------
/// 非接读卡器 
class BOC_ComContactlessICCardDevAdapter :
    public BOC_ComContactICCardDevAdapter
{
protected:
    //----------------------------------------------------- 
    void _init_cmd()
    {
        _poweron_cmd = 0x1A;
        _poweroff_cmd = 0x19;
        _apdu_cmd = 0x15;
        _setcardtype_cmd = 0x22;
    }
    //-----------------------------------------------------
public:
    //----------------------------------------------------- 
    BOC_ComContactlessICCardDevAdapter() : BOC_ComContactICCardDevAdapter()
    {
        _init_cmd();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace iccard
} // namespace application
} // namespace zhou_yb
//=========================================================

