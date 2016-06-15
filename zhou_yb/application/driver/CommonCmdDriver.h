//========================================================= 
/**@file CommonCmdDriver.h
 * @brief 一些基本的公共命令
 * 
 * @date 2016-05-11   21:10:19
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMMONCMDDRIVER_H_
#define _LIBZHOUYB_COMMONCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 类中命令函数定义
#define LC_CMD_METHOD(methodName) bool methodName(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
/// 类中导出ILastErrBehavior接口
#define LC_CMD_LASTERR(lastErrInvoker) \
    virtual int GetLastErr() const \
    { \
        return lastErrInvoker.GetLastErr(); \
    } \
    virtual const char* GetErrMessage() \
    { \
        return lastErrInvoker.GetErrMessage(); \
    } \
    virtual void ResetErr() \
    { \
        return lastErrInvoker.ResetErr(); \
    }
/// 类中导出InterruptBehavior接口
#define LC_CMD_INTERRUPT(interruptInvoker) \
    LC_CMD_METHOD(UpdateInterrupter) \
    { \
        interruptInvoker.SetInterrupter(Interrupter); \
        return true; \
    } \
    void SetInterrupter(const Ref<IInterrupter>& interrupter) \
    { \
        Interrupter = interrupter; \
        interruptInvoker.SetInterrupter(interrupter); \
    }
/// 类中导出IBaseDevAdapterBehavior接口
#define LC_CMD_ADAPTER(deviceType, adapterInvoker) \
    virtual void SelectDevice(const Ref<deviceType>& dev) \
    { \
        BaseDevAdapterBehavior::SelectDevice(dev); \
        adapterInvoker.SelectDevice(dev); \
    } \
    virtual void ReleaseDevice() \
    { \
        BaseDevAdapterBehavior::ReleaseDevice(); \
        adapterInvoker.ReleaseDevice(); \
    }
/// 类中导出ILoggerBehavior接口
#ifdef OPEN_LOGGER
#   define LC_CMD_LOGGER(loggerInvoker) \
    virtual void SelectLogger(const LoggerAdapter& log) \
    { \
        LoggerBehavior::SelectLogger(log); \
        loggerInvoker.SelectLogger(log); \
    } \
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL) \
    { \
        LoggerBehavior::ReleaseLogger(plog); \
        loggerInvoker.ReleaseLogger(plog); \
    }
#else
#   define LC_CMD_LOGGER(loggerInvoker) 
#endif
//--------------------------------------------------------- 
/// 托管两个变量实现ILastErrBehavior
class LastErrInvoker : public ILastErrBehavior, public RefObject
{
protected:
    int* _pErr;
    string* _pMsg;
public:
    LastErrInvoker()
    {
        _pErr = NULL;
        _pMsg = NULL;
    }
    inline void Invoke(int& err, string& msg)
    {
        _pErr = &err;
        _pMsg = &msg;
    }
    inline bool IsValid() const
    {
        return _pErr != NULL && _pMsg != NULL;
    }
    inline void Uninvoke()
    {
        _pErr = NULL;
        _pMsg = NULL;
    }
    virtual int GetLastErr() const
    {
        if(_pErr != NULL)
            return (*_pErr);
        return DeviceError::Success;
    }
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage()
    {
        if(_pMsg != NULL)
            return _pMsg->c_str();
        return "";
    }
    /// 重置错误信息
    virtual void ResetErr()
    {
        (*_pErr) = DeviceError::Success;
        (*_pMsg) = "";
    }
};
//--------------------------------------------------------- 
/// 托管InterruptBehavior
class InterruptInvoker : public selecter<Ref<InterruptBehavior> >, public InterruptBehavior
{
public:
    /// 选择类型定义
    typedef selecter<Ref<InterruptBehavior> > SelecterType;
    /// 设置中断器
    virtual void SetInterrupter(const Ref<IInterrupter>& interrupter)
    {
        InterruptBehavior::Interrupter = interrupter;

        list<Ref<InterruptBehavior> >::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->Interrupter = interrupter;
        }
    }
};
//--------------------------------------------------------- 
/// 托管IBaseDevAdapterBehavior
template<class IDeviceType>
class DevAdapterInvoker : public selecter<Ref<IBaseDevAdapterBehavior<IDeviceType> > >
{
public:
    /// 选择类型定义
    typedef selecter<Ref<IBaseDevAdapterBehavior<IDeviceType> > > SelecterType;
    /// 适配设备
    void SelectDevice(const Ref<IDeviceType>& dev)
    {
        typename list<Ref<IBaseDevAdapterBehavior<IDeviceType> > >::iterator itr;
        for(itr = SelecterType::_linkList.begin();itr != SelecterType::_linkList.end(); ++itr)
        {
            (*itr)->SelectDevice(dev);
        }
    }
    /// 释放设备 
    void ReleaseDevice()
    {
        typename list<Ref<IBaseDevAdapterBehavior<IDeviceType> > >::iterator itr;
        for(itr = SelecterType::_linkList.begin();itr != SelecterType::_linkList.end(); ++itr)
        {
            (*itr)->ReleaseDevice();
        }
    }
};
//--------------------------------------------------------- 
#ifdef OPEN_LOGGER
/// 托管ILoggerBehavior
class LoggerInvoker : public selecter<Ref<ILoggerBehavior> >
{
public:
    /// 选择类型定义
    typedef selecter<Ref<ILoggerBehavior> > SelecterType;
    /// 适配日志
    void SelectLogger(const LoggerAdapter& logger)
    {
        list<Ref<ILoggerBehavior> >::iterator itr;
        for(itr = SelecterType::_linkList.begin();itr != SelecterType::_linkList.end(); ++itr)
        {
            (*itr)->SelectLogger(logger);
        }
    }
    /// 释放日志  
    void ReleaseLogger(const LoggerAdapter* plog = NULL)
    {
        list<Ref<ILoggerBehavior> >::iterator itr;
        for(itr = SelecterType::_linkList.begin();itr != SelecterType::_linkList.end(); ++itr)
        {
            (*itr)->ReleaseLogger(plog);
        }
    }
};
#else
/// 托管ILoggerBehavior(空类)
class LoggerInvoker : public selecter<Ref<RefObject> >
{
public:
    /// 选择类型定义
    typedef selecter<Ref<RefObject> > SelecterType;
};
#endif
//--------------------------------------------------------- 
/// 命令驱动接口
struct ICommandDriver
{
    /// 执行命令
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv) = 0;
};
//--------------------------------------------------------- 
/// 命令驱动辅助函数
class CommandDriverHelper
{
protected:
    CommandDriverHelper() {}
public:
    /// 转换二进制数据格式
    static bool Encoding(const char* encode, const ByteArray& data, ByteBuilder& buff)
    {
        ByteArray enc(encode);
        if(StringConvert::Compare(enc, "HEX", true))
        {
            ByteConvert::ToAscii(data, buff);
            return true;
        }
        else if(StringConvert::Compare(enc, "Base64", true))
        {
            Base64_Provider::Encode(data, buff);
            return true;
        }

        return false;
    }
};
//--------------------------------------------------------- 
/// 基于命令方式的驱动
template<class TArgParser>
class CommandDriver :
    public DeviceBehavior,
    public ICommandDriver,
    public CommandCollection,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// 参数转换器类型
    typedef TArgParser ArgParserType;
    //----------------------------------------------------- 
    /// 生成需要绑定的命令参数
    static string Arg(const string& key, const string& val)
    {
        TArgParser arg;
        arg.PushValue(key, val);

        ByteBuilder argMsg(8);
        arg.ToString(argMsg);

        return argMsg.GetString();
    }
    /// 生成需要绑定的命令参数
    template<class T>
    static string Arg(const string& key, const T& val)
    {
        string sVal = ArgConvert::ToString<T>(val);
        return Arg(key, sVal);
    }
    //----------------------------------------------------- 
    /* 日志相关接口重写 */
    LOGGER(virtual void SelectLogger(const LoggerAdapter& log)
    {
        _log.Select(log);
        list<ComplexCommand>::iterator itr;
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            itr->SelectLogger(_log);
        }
    }
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL)
    {
        if(plog != NULL)
            _log.Release(*plog);
        else
            _log.Release();
        list<ComplexCommand>::iterator itr;
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            itr->ReleaseLogger(&_log);
        }
    });
    //----------------------------------------------------- 
    /**
     * @brief 枚举所有支持的命令
     * @date 2016-05-07 11:11
     * 
     * @retval CMD : string
     */
    LC_CMD_METHOD(EnumCommand)
    {
        list<ComplexCommand>::iterator itr;
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            rlt.PushValue("CMD", (*itr).Name);
        }
        return true;
    }
    /**
     * @brief 执行指定的命令
     * @date 2016-05-07 12:14
     * 
     * @param [in] CMD : string 命令
     * @param [in] ARG : string 参数
     * 
     * @retval RLT : string 结果
     */
    LC_CMD_METHOD(OnCommand)
    {
        string cmd = arg["CMD"].To<string>();
        string send = arg["ARG"].To<string>();
        ByteBuilder recv(32);
        bool bRet = bRet = TransmitCommand(cmd.c_str(), send.c_str(), recv);
        if(bRet) rlt.PushValue("RLT", recv.GetString());
        return bRet;
    }
    /**
     * @brief 获取上次错误码和错误信息
     * @date 2016-05-07 13:39
     * 
     * @retval CODE : int 错误码
     * @retval MSG : string 错误信息
     */
    LC_CMD_METHOD(LastError)
    {
        rlt.PushValue("CODE", ArgConvert::ToString<int>(GetLastErr()));
        rlt.PushValue("MSG", GetErrMessage());
        ResetErr();
        return true;
    }
    //----------------------------------------------------- 
    /// 消息分发函数
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv)
    {
        LOG_FUNC_NAME();
        LOGGER(ByteBuilder callName = sCmd;
        _log << "Call Command:<" << callName.GetString();
        callName = send;
        if(!callName.IsEmpty()) _log << " : " << callName.GetString();
        _log << ">\n");
        // 查找命令表
        list<ComplexCommand>::iterator itr;
        TArgParser arg;
        TArgParser rlt;
        arg.Parse(send);

        bool bCommand = false;
        bool bHasHandled = false;
        LOGGER(size_t index = 0;
        size_t subIndex = 0);
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            LOGGER(++index);
            // 依次执行相同名称的命令
            ByteArray cmdName(itr->Name.c_str(), itr->Name.length());
            if(StringConvert::Compare(cmdName, sCmd, true))
            {
                LOGGER(_log << "Run Command:[" << index << "]\n");
                if(!itr->RunCommand<TArgParser>(arg, rlt))
                {
                    return _logRetValue(false);
                }
                bHasHandled = true;
            }
        }
        ASSERT_FuncErrInfoRet(bHasHandled, DeviceError::ArgErr, "命令未注册");

        ByteBuilder rltBuff(8);
        rlt.ToString(rltBuff);
        recv.Append(rltBuff);
        LOGGER(_log << "RLT:<" << rltBuff.GetString() << ">\n");
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMONCMDDRIVER_H_
//========================================================= 