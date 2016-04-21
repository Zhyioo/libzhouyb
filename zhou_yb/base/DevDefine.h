//========================================================= 
/**@file DevDefine.h 
 * @brief 设备定义,与设备相关的一些接口、错误码、宏定义,设备命令转换
 * 
 * @date 2011-10-17   15:21:04 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_DEVDEFINE_H_
#define _LIBZHOUYB_DEVDEFINE_H_
//--------------------------------------------------------- 
#include "ByteBuilder.h"

#include "../container/object.h"
using zhou_yb::container::Ref;
using zhou_yb::container::RefObject;

#include <string>
using std::string;
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//---------------------------------------------------------
/// 接口封装,将类识别为一个接口的标识类 
struct IInterface {}; 
template<class T>
/// IInterface的默认实现 
struct Interface : T {};
//--------------------------------------------------------- 
/**
 * @brief 工厂接口 
 *
 * 定义为 virtual 函数类型,这样,一个工厂类可以支持多种数据类型的创建 
 */  
template<class T>
struct IFactory
{
    /// 创建对象 
    virtual bool Create(T& obj, const char* sArg = NULL) = 0;
    /// 返回对象是否有效 
    virtual bool IsValid(const T& obj) = 0;
    /// 销毁对象 
    virtual void Dispose(T& obj) = 0;
};
//--------------------------------------------------------- 
/// 中断接口 
struct IInterrupter
{
    /// 中断点,返回是否需要进行中断 
    virtual bool InterruptionPoint() = 0;
    /// 中断,返回是否成功中断 
    virtual bool Interrupt() = 0;
    /// 重置中断为初始状态
    virtual bool Reset() = 0;
};
//--------------------------------------------------------- 
/// 通过bool标志位来决定是否需要进行中断的中断器 
class BoolInterrupter : public IInterrupter, public RefObject
{
protected:
    /// 中断标志 
    bool _interruptVal;
public:
    BoolInterrupter()
    {
        _interruptVal = false;
    }
    /// 中断点,返回是否需要进行中断 
    virtual bool InterruptionPoint()
    {
        return _interruptVal;
    }
    /// 中断
    virtual bool Interrupt()
    {
        _interruptVal = true;
        return true;
    }
    /// 重置 
    virtual bool Reset()
    {
        _interruptVal = false;
        return true;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 设备的基础接口 
 *
 * 提供打开、关闭功能(参数为void*)
 */
struct IBaseDevice 
{
    /// 打开设备
    virtual bool Open(const char* sArg = NULL) = 0;
    /// 返回设备是否已经打开(后面不加const以支持写入日志)
    virtual bool IsOpen() = 0;
    /// 关闭(如果不可以的话强行关闭)
    virtual void Close() = 0;
};
//---------------------------------------------------------
/**
 * @brief 设备交互方式(可单独收发)
 * 
 * 提供设备读写的基础功能,给设备发数据可以不接收返回的数据
 */
struct IInteractiveTrans 
{
    /// 读数据
    virtual bool Read(ByteBuilder& data) = 0;
    /// 写数据
    virtual bool Write(const ByteArray& data) = 0;
};
//---------------------------------------------------------
/**
 * @brief 设备交互方式(发送数据必须接收)
 *
 * 发数据必须有接收的基础设备接口
 */
struct ITransceiveTrans
{
    /// 交换数据
    virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv) = 0;
};
//--------------------------------------------------------- 
/**
 * @brief IC卡设备接口 
 */
struct IICCardDevice : public ITransceiveTrans
{
    /// 上电
    virtual bool PowerOn(const char* readerName, ByteBuilder* pAtr = NULL) = 0;
    /// 下电
    virtual bool PowerOff() = 0;
    /// 是否已经上电 
    virtual bool HasPowerOn() const = 0;
    /// 交换APDU(提供默认实现) 
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd) = 0;
    // 只提供交互单独的一条指令 
    // virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv) = 0;
};
/// IICCardDevice对APDU的默认适配实现(自动处理61 6C等) 
struct ICCardDevice : public IICCardDevice
{
    /// 自动处理需要再次发送命令接收数据的命令 61XX 6CXX 
    static bool AutoApdu(ITransceiveTrans& dev, const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        size_t lastlen = recvBcd.GetLength();
        bool iRet = dev.TransCommand(sendBcd, recvBcd);
        // 发送命令失败
        if(iRet == false)
            return false;

        size_t len = recvBcd.GetLength();
        if(len < 2)
            return false;

        // 读取卡中准备好的数据命令
        byte cmdArray[] = { 0x00, 0xC0, 0x00, 0x00, 0x00 };
        ByteArray cmd(cmdArray, 5);

        // 比较最后两个字节，判断是否需要继续读取数据
        if(recvBcd[len - 2] == 0x90 &&
            recvBcd[len - 1] == 0x00)
        {
            ;
        }
        else if(recvBcd[len - 2] == 0x6C)
        {            
            // 保存更改前的值以还原数据
            byte last = sendBcd[sendBcd.GetLength() - 1];
            // 强制转换
            byte* ptr = const_cast<byte*>(sendBcd.GetBuffer());
            ptr[sendBcd.GetLength() - 1] = recvBcd[len - 1];
            recvBcd.RemoveTail(recvBcd.GetLength() - lastlen);
            // 继续读取数据
            iRet = AutoApdu(dev, sendBcd, recvBcd);

            ptr[sendBcd.GetLength() - 1] = last;
        }
        else if(recvBcd[len - 2] == 0x61)
        {
            cmd[4] = recvBcd[len - 1];
            recvBcd.RemoveTail(recvBcd.GetLength() - lastlen);
            // 继续读取数据
            iRet = AutoApdu(dev, cmd, recvBcd);
        }

        return iRet;
    }

    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        return ICCardDevice::AutoApdu(*this, sendBcd, recvBcd);
    }
};
//--------------------------------------------------------- 
/**
 * @brief 设备错误码
 */
class DeviceError
{
protected:
    DeviceError() {}
public:
    //-----------------------------------------------------
    /// 设备错误码定义
    enum ErrorCode
    {
        /// 操作成功  0
        Success = 0,
        /// 未知错误  1
        UnKnownErr,
        // 设备错误   
        /// 设备打开失败  2
        DevOpenErr,
        /// 设备已经打开  3
        DevAlreadyOpenErr,
        /// 设备已经关闭  4
        DevAlreadyCloseErr,
        /// 设备忙碌      5
        DevIsBusyErr,
        /// 设备关闭失败  6
        DevCloseErr,
        /// 设备无效      7
        DevInvalidErr,
        /// 设备连接错误  8
        DevConnectErr,
        /// 设备验证失败  9
        DevVerifyErr,
        /// 设备不支持    10
        DevNotSupportErr,
        /// 设备返回操作失败  11
        DevReturnErr,
        /// 设备不存在        12
        DevNotExistErr,
        /// 设备状态错误      13
        DevStateErr,
        /// 设备初始化失败    14
        DevInitErr,
        /// 设备释放资源失败  15
        DevReleaseErr,
        // 参数错误 
        /// 参数错误          16
        ArgErr,
        /// 参数为空          17
        ArgIsNullErr,
        /// 参数个数错误      18
        ArgCountErr,
        /// 参数值范围错误    19
        ArgRangeErr,
        /// 参数长度错误      20
        ArgLengthErr,
        /// 参数格式错误      21
        ArgFormatErr,
        // 操作错误 
        /// 等待超时          22
        WaitTimeOutErr,
        /// 操作被取消        23
        OperatorInterruptErr,
        /// 操作错误          24
        OperatorErr, 
        /// 操作状态错误      25
        OperatorStatusErr,
        /// 不支持该操作      26
        OperatorNoSupportErr,
        /// 数据发送失败      27
        SendErr,
        /// 数据接收失败      28
        RecvErr,
        /// 发送数据格式错误  29
        SendFormatErr,
        /// 接收的数据格式错误 30
        RecvFormatErr,
        /// 交换数据失败       31
        TransceiveErr,
        /// 交换的数据格式错误 32
        TransceiveFormatErr,

        /// 错误码的最大值
        ENUM_MAX(ErrorCode)
    };
    //-----------------------------------------------------
    /// 获取对应的错误字符串 
    static const char* ToString(int errCode)
    {
        switch(errCode)
        {
        case ArgErr:
            return "ArgErr";
        case ArgIsNullErr:
            return "ArgIsNullErr";
        case ArgRangeErr:
            return "ArgRangeErr";
        case ArgCountErr:
            return "ArgCountErr";
        case ArgFormatErr:
            return "ArgFormatErr";
        case ArgLengthErr:
            return "ArgLengthErr";
        case DevCloseErr:
            return "DevCloseErr";
        case DevConnectErr:
            return "DevConnectErr";
        case DevNotExistErr:
            return "DevNotExistErr";
        case DevNotSupportErr:
            return "DevNotSupportErr";
        case DevInvalidErr:
            return "DevInvalidErr";
        case DevOpenErr:
            return "DevOpenErr";
        case DevAlreadyOpenErr:
            return "DevAlreadyOpenErr";
        case DevAlreadyCloseErr:
            return "DevAlreadyCloseErr";
        case DevIsBusyErr:
            return "DevIsBusyErr";
        case DevInitErr:
            return "DevInitErr";
        case DevReleaseErr:
            return "DevReleaseErr";
        case DevReturnErr:
            return "DevReturnErr";
        case DevStateErr:
            return "DevStateErr";
        case DevVerifyErr:
            return "DevVerificationErr";
        case OperatorInterruptErr:
            return "OperatorInterruptErr";
        case OperatorErr:
            return "OperatorErr";
        case OperatorNoSupportErr:
            return "OperatorNoSupportErr";
        case OperatorStatusErr:
            return "OperatorStatusErr";
        case RecvErr:
            return "RecvErr";
        case RecvFormatErr:
            return "RecvFormatErr";
        case SendErr:
            return "SendErr";
        case SendFormatErr:
            return "SendFormatErr";
        case TransceiveErr:
            return "TransceiveErr";
        case TransceiveFormatErr:
            return "TransceiveFormatErr";
        case Success:
            return "Success";
        case WaitTimeOutErr:
            return "WaitTimeOutErr";
        case UnKnownErr:
        default:
            return "UnKnownErr";
        }
    }
    /// 获取对应错误码的详细描述信息
    static const char* TransErrToString(int errCode)
    {
        switch(errCode)
        {
        case ArgErr:
            return "参数错误";
        case ArgIsNullErr:
            return "参数为空";
        case ArgRangeErr:
            return "参数范围错误";
        case ArgCountErr:
            return "参数数目错误";
        case ArgFormatErr:
            return "参数格式错误";
        case ArgLengthErr:
            return "参数长度错误";
        case DevCloseErr:
            return "设备关闭失败";
        case DevConnectErr:
            return "设备连接失败";
        case DevNotExistErr:
            return "设备不存在";
        case DevNotSupportErr:
            return "设备不支持";
        case DevInvalidErr:
            return "设备无效";
        case DevOpenErr:
            return "设备打开失败";
        case DevAlreadyOpenErr:
            return "设备已经打开";
        case DevAlreadyCloseErr:
            return "设备已经关闭";
        case DevIsBusyErr:
            return "设备忙碌";
        case DevInitErr:
            return "设备初始化失败";
        case DevReleaseErr:
            return "设备释放资源失败";
        case DevReturnErr:
            return "设备返回错误";
        case DevStateErr:
            return "设备状态错误";
        case DevVerifyErr:
            return "设备验证失败";
        case OperatorInterruptErr:
            return "操作被取消";
        case OperatorErr:
            return "操作错误";
        case OperatorNoSupportErr:
            return "不支持的操作";
        case OperatorStatusErr:
            return "操作条件不满足";
        case RecvErr:
            return "接收数据失败";
        case RecvFormatErr:
            return "接收的数据格式错误";
        case SendErr:
            return "发送数据失败";
        case SendFormatErr:
            return "发送的数据格式错误";
        case TransceiveErr:
            return "交换数据失败";
        case TransceiveFormatErr:
            return "交换数据的格式错误";
        case Success:
            return "操作成功";
        case WaitTimeOutErr:
            return "等待超时";
        case UnKnownErr:
        default:
            return "未知错误";
        }
    }
};
//---------------------------------------------------------
/// 设备默认缓冲区大小
#ifndef DEV_BUFFER_SIZE
#   define DEV_BUFFER_SIZE 256
#endif
/// 设备默认超时时间(毫秒) 
#ifndef DEV_WAIT_TIMEOUT
#   define DEV_WAIT_TIMEOUT 10000
#endif 
/// 设备默认轮寻间隔时间(毫秒)
#ifndef DEV_OPERATOR_INTERVAL
#   define DEV_OPERATOR_INTERVAL 20
#endif
/// 设备默认操作尝试次数
#ifndef DEV_RETRY_COUNT
#   define DEV_RETRY_COUNT 3
#endif
/// APDU指令大小 
#ifndef MAX_APDU_LEN 
#   define MAX_APDU_LEN 256
#endif 
//---------------------------------------------------------
// 常用重复代码宏定义
//---------------------------------------------------------
 /**
  * @brief 是否启用日志
  *
  * 日志FileLogger可以用本身的开关进行关闭,
  * 该宏的作用在于直接移除代码中的日志代码,使得代码更高效
  */
//#define OPEN_LOGGER
//---------------------------------------------------------
/// 日志间隔行 
#define LOGGER_SPLIT_LINE "//---------------------------------------------------------"
//@{
/**@name
 * @brief 设备操作辅助宏 
 * 
 * 简化代码 
 */
/// 简写的函数断言宏(func为真时继续执行，否则执行定义中的代码)
#define ASSERT_Function(func,code) \
if(!(func)) \
{\
    code;\
}
/// 断言适配器的有效性 
#define ASSERT_Device() ASSERT_DeviceValid(IsValid())
//---------------------------------------------------------
/// 日志开启选项(指示是否开启日志功能)
#ifdef OPEN_LOGGER
//---------------------------------------------------------
    /// 将信息记录到日志中
    #define LOGGER(code) code
    /// 获取对象的日志引用 
    #define LOG_OBJ_INIT(obj) LoggerAdapter _log = obj.GetLogger()
    /// 记录函数信息
    #define LOG_FUNC_NAME() LoggerFunction __logFunc(_log, __FUNCTION__)
    /// 记录返回值
    #define return_Val(val) {_log<<"返回:<"<<(val)<<">\n";return (val);}
    /// 开始计时 
    #define LOG_INIT_TICK() Timer __logTickTimer
    /// 停止计时并输出消耗时间 
    #define LOG_TICK() {_log<<"耗时:<"<<(__logTickTimer.TickCount())<<"ms>\n";}
    /// 析构时自动的输出消耗时间
    #define LOG_AUTO_TICK() LoggerTimer __logTimer(_log)
    /// 断言设备的有效性,设备无效时返回，记录返回值
    #define ASSERT_DeviceValid(func)             \
        ASSERT_Function(func, _logErr(DeviceError::DevInvalidErr);return _logRetValue(false))
    #define ASSERT_Func(func)                    ASSERT_Function(func,return false)
    #define ASSERT_FuncRet(func)                 ASSERT_Function(func,return _logRetValue(false))
    #define ASSERT_FuncInfo(func,info)           \
        ASSERT_Function(func,_errinfo += ' ';_errinfo += info;_log.WriteLine(info);return false)
    #define ASSERT_FuncInfoRet(func,info)        \
        ASSERT_Function(func,_errinfo += ' ';_errinfo += info;_log.WriteLine(info);return _logRetValue(false))
    #define ASSERT_FuncErr(func,err)             ASSERT_Function(func,_logErr(err);return false)
    #define ASSERT_FuncErrRet(func,err)          ASSERT_Function(func,_logErr(err);return _logRetValue(false))
    #define ASSERT_FuncErrInfo(func,err,info)    ASSERT_Function(func,_logErr(err,info);return false)
    #define ASSERT_FuncErrInfoRet(func,err,info) ASSERT_Function(func,_logErr(err,info);return _logRetValue(false))
//---------------------------------------------------------
#else
//---------------------------------------------------------
    /// 将信息记录到日志中
    #define LOGGER(code)
    /// 获取对象的日志引用 
    #define LOG_OBJ_INIT(obj)
    /// 记录函数信息
    #define LOG_FUNC_NAME()
    /// 记录返回信息
    #define return_Val(val) return (Val)
    /// 开始计时 
    #define LOG_INIT_TICK() 
    /// 停止计时并输出消耗时间 
    #define LOG_TICK() 
    /// 析构时自动的输出消耗时间
    #define LOG_AUTO_TICK() 
    /// 断言设备的有效性,设备无效时返回，记录返回值
    #define ASSERT_DeviceValid(func)             \
        ASSERT_Function(func, _logErr(DeviceError::DevInvalidErr);return false)
    #define ASSERT_Func(func)                    ASSERT_Function(func,return false)
    #define ASSERT_FuncRet(func)                 ASSERT_Func(func)
    #define ASSERT_FuncInfo(func,info)           \
        ASSERT_Function(func,_errinfo += ' ';_errinfo += info;return false)
    #define ASSERT_FuncInfoRet(func,info)        ASSERT_FuncInfo(func,info)
    #define ASSERT_FuncErr(func,err)             ASSERT_Function(func,_logErr(err);return false)
    #define ASSERT_FuncErrRet(func,err)          ASSERT_FuncErr(func,err)
    #define ASSERT_FuncErrInfo(func,err,info)    ASSERT_Function(func,_logErr(err,info);return false)
    #define ASSERT_FuncErrInfoRet(func,err,info) ASSERT_FuncErrInfo(func,err,info)
//---------------------------------------------------------
//@}
#endif
//--------------------------------------------------------- 
} // namespace base
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_DEVDEFINE_H_
//=========================================================
