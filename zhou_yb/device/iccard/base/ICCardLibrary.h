//========================================================= 
/**@file ICCardLibrary.h 
 * @brief 
 * 
 * 时间: 2012-11-20   21:04 
 * 作者: Zhyioo  
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICCARDLIBRARY_H_
#define _LIBZHOUYB_ICCARDLIBRARY_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace base {
//--------------------------------------------------------- 
/// IC卡常用基础类操作 
class ICCardLibrary 
{
protected:
    ICCardLibrary() {}
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// APDU参数索引 
    enum ENUM_CMD_INDEX
    {
        CLS_INDEX = 0x00,
        INS_INDEX,
        P1_INDEX,
        P2_INDEX,
        LC_INDEX 
    };
    /// IC卡错误码 
    enum SW
    {
        /// 无效的SW(交换APDU失败) 
        UnValidSW = 0x0000,
        /// 61 XX 正常处理 
        NormalHandled = 0x06100,
        /// 无信息可提供 
        NoMoreInformation = 0x06200,
        /// 回送的数据可能有错 
        ReturnDataMayBeMistake = 0x06281,
        /// 文件长度<le
        FileLenLessThenLe = 0x06282,
        /// 选择的文件无效 
        SelectedFileNotVaild = 0x06283,
        /// FCI格式与P2不符合 
        FCI_ArgFormatIsNotP2 = 0x06284,
        /// 认证失败 
        AuthFail = 0x06300,
        /// 认证失败,63 CX还有X次机会 
        AuthFailNumber = 0x063C0,
        /// 状态标志位未改变 
        StatusFlagNotChange = 0x06400,
        /// 内存错误 
        MemoryErr = 0x06581,
        /// 长度错误 
        LengthErr = 0x06700,
        /// 不支持安全报文 
        NotSupportSafetyMessage = 0x06882,
        /// 不能处理 
        CannotHandle = 0x06900,
        /// 命令不接受 
        CommandDoesNotAccept = 0x06901,
        /// 命令与文件结构不相容 
        CommandIsNotCompatible = 0x06981,
        /// 不满足安全状态 
        SafetyStateErr = 0x06982,
        /// 验证方法锁定 
        VerificationMethodOfLocking = 0x06983,
        /// 引用数据无效 
        RefDataNotVaild = 0x06984,
        /// 使用条件不满足 
        UsingConditionIsNotSatisfied = 0x06985,
        /// 不满足命令执行的条件 
        CommandConditionsErr = 0x06986,
        /// 安全报文数据项丢失 
        SafetyDataIsMissing = 0x06987,
        /// 安全信息数据对象不正确 
        MAC_IsWrong = 0x06988,
        /// 数据域参数不正确 
        DataArgIsWrong = 0x06A80,
        /// 功能不支持 
        FunctionIsNotSupported = 0x06A81,
        /// 未找到文件
        FileNotFound = 0x06A82,
        /// 未找到记录 
        RecodeNotFound = 0x06A83,
        /// 文件存储空间不足 
        InsufficientFileStorageSpace = 0x06A84,
        /// P1和P2参数不正确
        P1_P2ArgErr = 0x06A86,
        /// 引用数据找不到 
        RefDataNotFound = 0x06A88,
        /// 参数错误 
        ArgErr = 0x06B00,
        /// 长度错误 
        LeLengthErr = 0x06C00,
        /// 数据无效 
        DataIsNotVaild = 0x06F00,
        /// 成功执行 
        SuccessSW = 0x09000,
        /// 金额不足
        InsufficientAmounts = 0x09301,
        /// MAC无效 
        MAC_IsNotVaild = 0x09302,
        /// 应用永久锁定 
        AppLockedForever = 0x09303,
        /// 金额不足 
        InsufficientBalance = 0x09401,
        /// 交易计数器达到最大值 
        TransactionCounterMaximum = 0x09402,
        /// 密钥索引不支持 
        KeyIndexNotSupport = 0x09403,
        /// 所需MAC不可用 
        MAC_CanNotBeUsed = 0x09406,
        /// 不支持的类,CLA错误 
        CLA_Err = 0x06E00,
        /// 不支持的指令代码 
        NotSupportCommand = 0x06D00,
        /// 接收通讯超时 
        RecvTimeout = 0x06600,
        /// 接收字符奇偶校验错 
        RecvCharCheckErr = 0x06601,
        /// 校验和不对 
        CheckValueErr = 0x06602,
        /// 当前DF文件无FCI
        DF_NotContainsFCI = 0x06603,
        /// 当前DF文件无SF或KF
        DF_NotContainsSF_KF = 0x06604
    };
    /// IC卡错误码描述信息 
    static const char* GetSW_MSG(ushort sw)
    {
        SW _sw = static_cast<SW>(sw);
        switch(_sw)
        {
        case NoMoreInformation:
            return "无信息可提供";
        case ReturnDataMayBeMistake:
            return "回送的数据可能有错";
        case FileLenLessThenLe:
            return "文件长度<le";
        case SelectedFileNotVaild:
            return "选择的文件无效";
        case FCI_ArgFormatIsNotP2:
            return "FCI格式与P2不符合";
        case StatusFlagNotChange:
            return "状态标志位未改变";
        case MemoryErr:
            return "内存错误";
        case LengthErr:
            return "长度错误";
        case NotSupportSafetyMessage:
            return "不支持安全报文";
        case CannotHandle:
            return "不能处理";
        case CommandDoesNotAccept:
            return "命令不接受";
        case CommandIsNotCompatible:
            return "命令与文件结构不相容";
        case SafetyStateErr:
            return "不满足安全状态";
        case VerificationMethodOfLocking:
            return "验证方法锁定";
        case RefDataNotVaild:
            return "引用数据无效";
        case UsingConditionIsNotSatisfied:
            return "使用条件不满足";
        case CommandConditionsErr:
            return "不满足命令执行的条件";
        case SafetyDataIsMissing:
            return "安全报文数据项丢失";
        case MAC_IsWrong:
            return "安全信息数据对象不正确";
        case DataArgIsWrong:
            return "数据域参数不正确";
        case FunctionIsNotSupported:
            return "功能不支持";
        case FileNotFound:
            return "未找到文件";
        case RecodeNotFound:
            return "未找到记录";
        case InsufficientFileStorageSpace:
            return "文件存储空间不足";
        case P1_P2ArgErr:
            return "P1和P2参数不正确";
        case RefDataNotFound:
            return "引用数据找不到";
        case ArgErr:
            return "参数错误";
        case LeLengthErr:
            return "LE长度错误";
        case DataIsNotVaild:
            return "数据无效";
        case SuccessSW:
            return "成功执行";
        case InsufficientAmounts:
            return "金额不足";
        case MAC_IsNotVaild:
            return "MAC无效";
        case AppLockedForever:
            return "应用永久锁定";
        case InsufficientBalance:
            return "金额不足";
        case TransactionCounterMaximum:
            return "交易计数器达到最大值";
        case KeyIndexNotSupport:
            return "密钥索引不支持";
        case MAC_CanNotBeUsed:
            return "所需MAC不可用";
        case CLA_Err:
            return "不支持的类,CLA错误";
        case NotSupportCommand:
            return "不支持的指令代码";
        case RecvTimeout:
            return "接收通讯超时";
        case RecvCharCheckErr:
            return "接收字符奇偶校验错";
        case CheckValueErr:
            return "校验和不正确";
        case DF_NotContainsFCI:
            return "当前DF文件无FCI";
        case DF_NotContainsSF_KF:
            return "当前DF文件无SF或KF";
        default:
            /// 63 XX
            if((sw & AuthFailNumber) == AuthFailNumber)
                return "认证错误,还有X次机会";
            else if((sw & AuthFail) == AuthFail)
                return "认证失败";
            else if((sw & LeLengthErr) == LeLengthErr)
                return "长度错误(Le错误,XX为实际长度)";
            break;
        }
        return "未知的错误码";
    }
    /// 返回状态码是否为0x9000
    static inline bool IsSuccessSW(ushort sw)
    {
        return sw == SuccessSW;
    }
    /// 删除数据的SW码
    static inline void RemoveSW(ByteBuilder& recv)
    {
        // 删除状态字(2字节)
        recv.RemoveTail(2);
    }
    /// 获取数据的SW
    static ushort GetSW(const ByteArray& recv)
    {
        if(recv.GetLength() < 2)
            return 0x00;
        ushort sw = static_cast<ushort>(recv[recv.GetLength() - 2] & 0x0FF);
        sw <<= BIT_OFFSET;
        sw += static_cast<ushort>(recv[recv.GetLength() - 1] & 0x0FF);

        return sw;
    }
    /// 将SW转为16进制 
    static void SetSW(ushort sw, ByteBuilder& recv)
    {
        recv += _itobyte(sw >> BIT_OFFSET);
        recv += _itobyte(sw);
    }
    /// 返回命令是否带LE参数 
    static bool HasLE(const ByteArray& cmdApdu)
    {
        if(cmdApdu.GetLength() > 5 && (cmdApdu[LC_INDEX] < cmdApdu.GetLength() - 5))
            return true;
        return false;
    }
    /// 返回APDU中LC的长度 
    static byte GetLC(const ByteArray& cmdApdu)
    {
        size_t len = cmdApdu.GetLength();
        if(len < 5)
            return 0;
        return _itobyte(len - 5);
    }
    /// 获取认证剩余次数,返回状态是否为63CX 
    static bool GetAuthCount(ushort sw, size_t* pErrCount = NULL)
    {
        if((sw & ICCardLibrary::AuthFailNumber) != ICCardLibrary::AuthFailNumber)
            return false;
        if(pErrCount != NULL)
        {
            (*pErrCount) = static_cast<size_t>(sw & 0x0F);
        }
        return true;
    }
    //----------------------------------------------------- 
}; // namespace ICCardLibrary
//--------------------------------------------------------- 
} // namespace base 
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDLIBRARY_H_
//========================================================= 
