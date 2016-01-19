//========================================================= 
/**@file ComICCardCmdAdapter.h 
 * @brief 串口集成设备命令适配器
 * 
 * @date 2011-08-25   14:15:50 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//---------------------------------------------------------
/**
 * @brief 串口相关设备指令适配器  
 *
 * 将单独指令拆分成0x3x格式的数据，并且在开头加上0x02,结尾加上校验码和0x03
 */ 
class ComICCardCmdAdapter :
    public IInteractiveTrans,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// 按照格式接收数据 
    static bool RecvByFormat(IInteractiveTrans& dev, ByteBuilder& emptyBuffer)
    {
        // 前导码最短长度  
        const size_t CmdHeaderLength = 5;
        bool bRet = false;
        // 接收前导码长度错误,尝试多次读取 
        do
        {
            bRet = dev.Read(emptyBuffer);
            if(!bRet) return false;
        } while(emptyBuffer.GetLength() < CmdHeaderLength);

        // 是否以02开头 
        if(emptyBuffer[0] != ComICCardCmdAdapter::STX)
            return false;

        // 计算长度,data长度至少为5  
        ByteBuilder tmpBuffer(2);
        ByteConvert::Fold(emptyBuffer.SubArray(1, 4), tmpBuffer);

        // 总的数据长度 
        size_t len = static_cast<size_t>(tmpBuffer[0]);
        len <<= BIT_OFFSET;
        len += static_cast<size_t>(tmpBuffer[1]);
        len *= 2;
        // 1字节STX,4字节长度位 2字节状态码,1字节ETX 
        len += 8;

        size_t count = emptyBuffer.GetLength();
        size_t lastlen = 0;
        while(count < len)
        {
            lastlen = emptyBuffer.GetLength();
            if(!dev.Read(emptyBuffer))
                return false;

            count += (emptyBuffer.GetLength() - lastlen);
        }

        // 如果收到的数据大于标识的长度则截断 
        if(len < count)
            emptyBuffer.RemoveTail(count - len);

        return true;
    }
    //----------------------------------------------------- 
protected:
    //-----------------------------------------------------
    /// 收发数据的缓冲区(减小空间分配的性能开销)
    ByteBuilder _tmpBuffer;
    /// 上次的状态码  
    ushort _statusCode;
    //----------------------------------------------------- 
    /// 初始化数据成员 
    inline void _init()
    {
        _statusCode = 0x00;
        IsChecksum = true;
    }
    /// 组包给设备发送的数据
    bool _PackSendData(const ByteArray& cmd, ByteBuilder& send)
    {
        // 报文头
        send += STX;
        // 报文长度(是否过长ushort(-1))
        if(cmd.GetLength() > 0xFFFF)
            return false;

        ushort len = static_cast<ushort>(cmd.GetLength());
        ByteBuilder lenBuffer(2);
        ByteConvert::FromObject(len, lenBuffer);

        ByteConvert::Expand(lenBuffer, send, ExBYTE);
        // 报文数据
        ByteConvert::Expand(cmd, send, ExBYTE);
        // 校验和 
        byte xorVal = ByteConvert::XorVal(cmd);
        ByteConvert::Expand(ByteArray(&xorVal, 1), send, ExBYTE);
        // 报文尾 
        send += ETX;

        return true;
    }
    /// 解包接收到的数据
    int _UnPackRecvData(const ByteArray& recv, ByteBuilder& data)
    {
        ByteBuilder tmpBuffer(8);
        // STX(1) 长度(4) 状态码(4) 数据(0-2*n) 校验和(2) ETX(1)
        size_t len = recv.GetLength();
        if(len < 12 || (len % 2 != 0))
        {
            LOGGER(_log<<"长度:<"<<len<<">\n");
            return UnPackRecvFormatErr;
        }

        // 以STX开头并且以ETX结尾
        if(recv[0] != STX || recv[len - 1] != ETX)
        {
            LOGGER(_log<<"开头:<"<<_hex(recv[0])<<">,结尾:<"<<_hex(recv[len - 1])<<">\n");
            return UnPackRecvHeadTailErr;
        }
        // 提取出长度的数据(+1跳过STX,-len跳过数据域,-3跳过校验值和ETX)
        ByteConvert::Fold(recv.SubArray(1, 4), tmpBuffer);
        len = tmpBuffer[0];
        len <<= BIT_OFFSET;
        len += tmpBuffer[1];
        tmpBuffer.Clear();
        // 压缩后的与原始的比较
        len *= 2;
        size_t recvLen = recv.GetLength() - 8;
        // 判断标识长度是否和实际长度相同 
        if(len != recvLen)
        {
            LOGGER(_log<<"标识长度:<"<<len<<">,实际长度:<"<<recvLen<<">\n");
            return UnPackRecvLengthErr;
        }

        // 提取状态码
        tmpBuffer.Clear();
        ByteConvert::Fold(recv.SubArray(5, 4), tmpBuffer);
        _statusCode = tmpBuffer[0];
        _statusCode <<= BIT_OFFSET;
        _statusCode += tmpBuffer[1];
        tmpBuffer.Clear();
        // 判断状态码是否正确
        int errCode = GetErrorCode(_statusCode);
        if(errCode != static_cast<int>(DeviceError::Success))
        {
            LOGGER(_log<<"状态码:<"<<_hex(_statusCode)<<">\n");
            return errCode;
        }

        // 取数据 
        len = recv.GetLength() - 12;// 数据段的长度(去掉状态码)
        ByteConvert::Fold(recv.SubArray(9, len + 2), data);

        byte devXorVal = data[data.GetLength() - 1];
        // 剔除状态码 
        data.RemoveTail();

        if(!IsChecksum)
            return DeviceError::Success;

        byte xorVal = 0;
        xorVal = ByteConvert::XorVal(data);
        if(devXorVal != xorVal)
        {
            data.Clear();
            LOGGER(_log<<"计算值:<"<<_hex(xorVal)<<">,设备返回值:<"<<_hex(devXorVal)<<">\n");
            return UnPackRecvXorValErr;
        }

        return DeviceError::Success;
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    ComICCardCmdAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //-----------------------------------------------------
    /// 串口错误码枚举 
    enum ComICCardDevErr
    {
        /* 上电部分 */
        /// 不支持接触式卡 1001
        NotSupportContactCardErr = DeviceError::ENUM_MAX(ErrorCode),
        /// 接触式卡卡未到位 1002
        ContactCardNotAlreadyErr,
        /// 接触式卡已上电 1003
        ContactCardAlreadyPowerOnErr,
        /// 接触式卡未上电 1004
        ContactCardNoPowerOnErr,
        /// 接触式卡上电失败 1005
        ContactCardPowerOnErr,
        /// 操作接触式卡无回应 1006
        ContactCardNoRecvErr,
        /// 操作接触式操作卡数据出现错误 1007
        ContactCardOperatorErr,
        /// 不支持PSAM卡 2001
        NoSupportPsamCardErr,
        /// PSAM卡已上电 2003
        PsamCardAlreadyPowerOnErr,
        /// PSAM卡未上电 2004
        PsamCardNoPowerOnErr,
        /// PSAM卡上电失败 2005
        PsamCardPowerOnErr,
        /// PSAM卡无回应 2006
        PsamCardNoRecvErr,
        /// 操作PSAM卡数据出现错误 2007
        PsamCardOperatorErr,
        /// 不支持非接触式卡 3001
        NotSupportContactlessCardErr,
        /// 非接触式卡未放到位 3002
        ContactlessCardNotAlreadyErr,
        /// 非接触式卡已上电 3003
        ContactlessCardAlreadyPowerOnErr,
        /// 非接触式卡未激活 3004
        ContactlessCardNotActiveErr,
        /// 非接触式卡激活失败 3005
        ContactlessCardActiveFailErr,
        /// 非接触式卡等待感应超时 3006 
        ContactlessCardWaitErr,
        /// 非接触卡无回应 3006
        ContactlessCardNoRecvErr,
        /// 操作非接触卡数据出现错误 3007
        ContactlessCardOperatorErr,
        /// 设置卡Halt状态失败 3008
        SetHaltStatusErr,
        /// 有多张卡在感应区 3009
        ContactlessCardCountErr,
        //
        /// 发送的数据包长度错误 
        PackSendLengthErr,
        /// 接收的数据包格式错误
        UnPackRecvFormatErr,
        /// 接收的数据包STX ETX错误
        UnPackRecvHeadTailErr,
        /// 接收的数据包标识长度与实际长度不匹配
        UnPackRecvLengthErr,
        /// 接收的数据包校验和错误 
        UnPackRecvXorValErr,

        MaxComICCardDevErr
    };
    //-----------------------------------------------------
    /// 数据头标识 
    static const byte STX/* = 0x02*/;
    /// 报文尾标识 
    static const byte ETX/* = 0x03*/;
    /// 扩展填充的字节 
    static const byte ExBYTE/* = 0x30*/;
    //-----------------------------------------------------
    /// 获取错误码的文字描述
    virtual const char* TransErrToString(int errCode) const 
    {
        if(errCode < DeviceError::ENUM_MAX(ErrorCode))
            return DeviceError::TransErrToString(errCode);
        switch(errCode)
        {
            /* 接触式IC卡 */
        case NotSupportContactCardErr:
            return "不支持接触式IC卡(10H,01H)";
        case ContactCardNotAlreadyErr:
            return "接触式卡未插到位(10H,02H)";
        case ContactCardAlreadyPowerOnErr:
            return "接触式卡已经上电(10H,03H)";
        case ContactCardNoPowerOnErr:
            return "接触式卡未上电(10H,04H)";
        case ContactCardPowerOnErr:
            return "接触式卡上电失败(10H,05H)";
        case ContactCardNoRecvErr:
            return "操作接触式卡无回应(10H,06H)";
        case ContactCardOperatorErr:
            return "操作接触式操作卡数据出现错误(10H,07H)";
            /* PSAM卡 */
        case NoSupportPsamCardErr:
            return "不支持PSAM卡(20H,01H)";
        case PsamCardAlreadyPowerOnErr:
            return "PSAM卡已上电(20H,03H)";
        case PsamCardNoPowerOnErr:
            return "PSAM卡未上电(20H,04H)";
        case PsamCardPowerOnErr:
            return "PSAM卡上电失败(20H,05H)";
        case PsamCardNoRecvErr:
            return "操作PSAM卡数据无回应(20H,06H)";
        case PsamCardOperatorErr:
            return "操作PSAM卡数据错误(20H,07H)";
            /* 非接触式IC卡 */
        case NotSupportContactlessCardErr:
            return "不支持非接触式卡(30H,01H)";
        case ContactlessCardNotAlreadyErr:
            return "非接触式卡未放到位(30H,02H)";
        case ContactlessCardAlreadyPowerOnErr:
            return "非接触式卡未放到位(30H,03H)";
        case ContactlessCardNotActiveErr:
            return "非接触式卡未激活(30H,04H)";
        case ContactlessCardActiveFailErr:
            return "非接触式卡激活失败(30H,05H)";
        case ContactlessCardWaitErr:
            return "非接触式卡感应区等待超时(30H,06H)";
        case ContactlessCardNoRecvErr:
            return "操作非接触式卡数据无回应(30H,06H)";
        case ContactlessCardOperatorErr:
            return "操作非接触卡数据出现错误(30H,07H)";
        case SetHaltStatusErr:
            return "设置卡Halt状态失败(30H,08H)";
        case ContactlessCardCountErr:
            return "有多张卡在感应区(30H,09H)";
        case PackSendLengthErr:
            return "发送的数据包长度错误";
        case UnPackRecvFormatErr:
            return "接收的数据包格式错误";
        case UnPackRecvHeadTailErr:
            return "接收的数据包STX ETX错误";
        case UnPackRecvLengthErr:
            return "接收的数据包标识长度与实际长度不匹配";
        case UnPackRecvXorValErr:
            return "接收的数据包校验和错误";
        default:
            return "无法识别的错误";
        }
    }
    /// 获取设备返回错误信息的对应错误码
    static int GetErrorCode(ushort errCode)
    {
        switch(errCode)
        {
            // 成功
        case 0x0000:
            return DeviceError::Success;
        case 0x1001:
            return NotSupportContactCardErr;
        case 0x1002:
            return ContactCardNotAlreadyErr;
        case 0x1003:
            return ContactCardAlreadyPowerOnErr;
        case 0x1004:
            return ContactCardNoPowerOnErr;
        case 0x1005:
            return ContactCardPowerOnErr;
        case 0x1006:
            return ContactCardNoRecvErr;
        case 0x1007:
            return ContactCardOperatorErr;
        case 0x2001:
            return NoSupportPsamCardErr;
        case 0x2003:
            return PsamCardAlreadyPowerOnErr;
        case 0x2004:
            return PsamCardNoPowerOnErr;
        case 0x2005:
            return PsamCardPowerOnErr;
        case 0x2006:
            return PsamCardNoRecvErr;
        case 0x2007:
            return PsamCardOperatorErr;
        case 0x3001:
            return NotSupportContactlessCardErr;
        case 0x3002:
            return ContactlessCardNotActiveErr;
        case 0x3003:
            return ContactlessCardAlreadyPowerOnErr;
        case 0x3004:
            return ContactlessCardNotActiveErr;
        case 0x3005:
            return ContactlessCardActiveFailErr;
        case 0x3006:
            return ContactlessCardWaitErr;
        case 0x3007:
            return ContactlessCardOperatorErr;
        case 0x3008:
            return SetHaltStatusErr;
        case 0x3009:
            return ContactlessCardCountErr;
        default:
            return DeviceError::UnKnownErr;
        }
    }
    //-----------------------------------------------------
    //@{
    /**@name 
     * @brief IInteractiveTrans接口成员
     */ 
    /// 发数据 
    virtual bool Write(const ByteArray& data)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log.WriteLine("Send:");
        _log.WriteLine(data));

        if(_tmpBuffer.GetSize() < 1)
        {
            _tmpBuffer.Resize(DEV_BUFFER_SIZE);
        }
        _tmpBuffer.Clear();

        /* 把命令组包 */
        if(_PackSendData(data, _tmpBuffer) == false)
        {
            string msg = ArgConvert::ToString(data.GetLength());
            _logErr(PackSendLengthErr, msg.c_str());
            return _logRetValue(false);
        }

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
        int errCode = _UnPackRecvData(_tmpBuffer, data);

        ASSERT_FuncErrRet(errCode == static_cast<int>(DeviceError::Success), errCode);

        LOGGER(
        _log.WriteLine("Recv:");
        _log.WriteLine(data));

        return _logRetValue(true);
    }
    //@}
    //-----------------------------------------------------
    /// 获取设备返回的原始状态码 
    inline ushort GetStatusCode() const
    {
        return _statusCode;
    }
    //----------------------------------------------------- 
    /// 是否强制比较校验和
    bool IsChecksum;
    //-----------------------------------------------------
};
//---------------------------------------------------------
} // namespace cmd_adapter 
} // namespace device
} // namespace zhou_yb
//=========================================================