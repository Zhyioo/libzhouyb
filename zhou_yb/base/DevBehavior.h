//========================================================= 
/**@file DevBehavior.h 
 * @brief 设备行为描述
 * 
 * @date 2011-10-17   15:19:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "Timer.h"
#include "DevDefine.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//---------------------------------------------------------
/// 行为封装,将类识别为一个行为的标识类 
struct IBehavior {}; 
/**
 * @brief 基础Behavior的默认实现,将行为规定为同一对象 
 */ 
struct Behavior {};
//--------------------------------------------------------- 
/// 操作中具有超时时间的行为 
struct ITimeoutBehavior : public Behavior
{
    /// 设置超时时间(ms),返回上次的超时时间  
    virtual uint SetWaitTimeout(uint timeoutMS) = 0;
    /// 设置轮询的间隔时间(ms),返回上次的超时时间  
    virtual uint SetOperatorInterval(uint intervalMS) = 0;
};
/// 默认的行为 
class TimeoutBehavior: public ITimeoutBehavior
{
protected:
    /// 读写的最大超时时间 
    uint _waitTimeout;
    /// 连续两次读取之间的间隔时间 
    uint _waitInterval;
public:
    TimeoutBehavior()
    {
        // 默认为15秒 
        _waitTimeout = DEV_WAIT_TIMEOUT;
        // 默认为15毫秒
        _waitInterval = DEV_OPERATOR_INTERVAL;
    }
    /// 设置超时时间(ms)
    virtual uint SetWaitTimeout(uint timeoutMS)
    {
        uint lastVal = _waitTimeout;
        _waitTimeout = timeoutMS;

        return lastVal;
    }
    /// 设置连续两次读或写之间的间隔时间 
    virtual uint SetOperatorInterval(uint intervalMS)
    {
        uint lastVal = _waitInterval;
        _waitInterval = intervalMS;

        return lastVal;
    }
};
//--------------------------------------------------------- 
/// 操作中具有中断的行为 
struct InterruptBehavior : public Behavior
{
    Ref<IInterrupter> Interrupter;

    /// 中断操作(设置中断器为中断状态),返回是否成功中断 
    static bool Interrupt(InterruptBehavior& dev)
    {
        /* 中断当前的操作 */
        BoolInterrupter interrupter;
        // 局部变量析构后,Interrupter会被重置为NULL
        if(dev.Interrupter.IsNull())
            dev.Interrupter = interrupter;
        return dev.Interrupter->Interrupt();
    }
    /// 取消操作(不会修改中断器状态),返回是否成功取消   
    static bool Cancel(InterruptBehavior& dev)
    {
        /* 中断当前的读写操作 */
        Ref<IInterrupter> interrupter = dev.Interrupter;
        dev.Interrupter = BoolInterrupter();
        bool bCancel = dev.Interrupter->Interrupt();
        dev.Interrupter = interrupter;
        return bCancel;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 带获取上次错误码的行为
 */
struct ILastErrBehavior : public Behavior
{
    /// 错误信息
    virtual int GetLastErr() const = 0;
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage() = 0;
};
//---------------------------------------------------------
/**
 * @brief ILastErrBehavior的默认实现
 */
class LastErrBehavior : public ILastErrBehavior
{
protected:
    /// 上一次的错误码
    int _lasterr;
    /// 错误码的详细描述信息
    string _errinfo;
public:
    LastErrBehavior() : _lasterr(DeviceError::Success), _errinfo("") {}
    /// 获取上一次的错误码
    virtual int GetLastErr() const
    {
        return _lasterr;
    }
    /// 获取错误的描述信息
    virtual const char* GetErrMessage()
    {
        return _errinfo.c_str();
    }
    /// 获取错误码的描述信息
    virtual const char* TransErrToString(int errCode) const
    {
        return DeviceError::TransErrToString(static_cast<int>(errCode));
    }
};
//---------------------------------------------------------
/**
 * @brief 带日志记录功能的行为
 * 
 * 提供日志选择功能
 */
struct ILoggerBehavior : public Behavior
{
    /// 日志支持
    virtual void SelectLogger(const LoggerAdapter& log) = 0;
    /// 获取所使用的日志系统(意味着日志可共享)
    virtual const LoggerAdapter& GetLogger() = 0;
    /// 释放对日志系统的占用
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL) = 0;
};
//---------------------------------------------------------
#ifdef OPEN_LOGGER

/// 在析构时自动输出时间 
class LoggerTimer
{
protected:
    Timer _timer;
    LoggerAdapter* _log;
public:
    LoggerTimer(const LoggerAdapter& log)
    {
        _log = const_cast<LoggerAdapter*>(&log);
    }
    ~LoggerTimer() { (*_log) << "耗时:<" << _timer.Elapsed() << "ms>\n"; (*_log).Flush(); _log = NULL; }
};
/// 在析构时自动输出函数分隔符  
class LoggerFunction
{
protected:
    Timer _timer;
    LoggerAdapter* _log;
    string _funcName;
public:
    LoggerFunction(const LoggerAdapter& log, const char* funcName = NULL)
    {
        _log = const_cast<LoggerAdapter*>(&log);
        (*_log).WriteLine(LOGGER_SPLIT_LINE);
        (*_log).WriteTime();

        if(funcName != NULL)
        {
            (*_log).WriteLine(funcName);
            _funcName = funcName;
        }
    }
    ~LoggerFunction() 
    { 
        (*_log)<<"/=------------\n";
        if(_funcName.length() > 0)
            (*_log)<<"~"<<_funcName<<endl;
        (*_log)<<"耗时:<"<<_timer.Elapsed()<<"ms>\n";
        (*_log).WriteLine(LOGGER_SPLIT_LINE);
        (*_log).Flush();

        _log = NULL;
    }
};
//---------------------------------------------------------
/**
 * @brief ILoggerBehavior的默认实现(定义OPEN_LOGGER时为空类) 
 */
class LoggerBehavior : public ILoggerBehavior
{
protected:
    /// 默认的日志的对象 
    LoggerAdapter _log;
    /// 记录函数返回值信息到日志中
    template<class T>
    inline const T& _logRetValue(const T& val)
    {
        _log<<"返回:<"<<val<<">\n";
        return val;
    }
public:
    //----------------------------------------------------- 
    virtual ~LoggerBehavior()
    {
        ReleaseLogger();
    }
    /// 日志支持
    virtual void SelectLogger(const LoggerAdapter& log)
    {
        _log.Select(log);
    }
    /// 日志可共享
    virtual const LoggerAdapter& GetLogger() 
    {
        return _log;
    }
    /// 释放日志 
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL)
    {
        if(plog != NULL)
            _log.Release(*plog);
        else
            _log.Release();
    }
    //----------------------------------------------------- 
};
#else
//--------------------------------------------------------- 
/// 定义为空的实现 
class LoggerBehavior : public ILoggerBehavior
{
protected:
    /// 记录函数返回值信息到日志中
    template<class T>
    inline const T& _logRetValue(const T& val)
    {
        return val;
    }
public:
    /// 日志支持
    virtual void SelectLogger(const LoggerAdapter&) {}
    /// 日志可共享
    virtual const LoggerAdapter& GetLogger() { return LoggerAdapter::Null; }
    /// 释放日志 
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL) {}
};
//--------------------------------------------------------- 
#endif
//--------------------------------------------------------- 
/// 禁用类的拷贝构造函数 
#define UnenableObjConsturctCopyBehavior(T) \
protected: \
    T(const T&) {} \
    void operator =(const T&) {} \
//---------------------------------------------------------
/**
 * @brief 设备适配器支持行为
 * 
 * 提供设备选择功能,设备运行期间不允许共享,故没有GetDevice()
 */
template<class IDeviceType>
struct IBaseDevAdapterBehavior : public Behavior
{
    /// 适配设备  
    virtual void SelectDevice(const Ref<IDeviceType>& dev) = 0;
    /// 释放设备  
    virtual void ReleaseDevice() = 0;
    /// 适配的设备是否有效  
    virtual bool IsValid() const = 0;
};
//---------------------------------------------------------
/**
 * @brief IBaseDevAdapterBehavior的默认实现
 */
template<class IDeviceType>
class BaseDevAdapterBehavior : public IBaseDevAdapterBehavior<IDeviceType>
{
protected:
    Ref<IDeviceType> _pDev;
public:
    virtual ~BaseDevAdapterBehavior()
    {
        ReleaseDevice();
    }

    /// 返回当前适配的串口设备是否有效 
    virtual bool IsValid() const
    {
        return !(_pDev.IsNull());
    }
    /// 选择适配的串口设备 
    virtual void SelectDevice(const Ref<IDeviceType>& dev)
    {
        _pDev = dev;
    }
    /// 释放对设备的占用 
    virtual void ReleaseDevice()
    {
        _pDev = Ref<IDeviceType>();
    }
};
//--------------------------------------------------------- 
/* 设备行为集合 */
//---------------------------------------------------------
/**
 * @brief 设备行为 
 * 
 * - 功能:
 *  - 适配设备
 *  - 带日志功能(未定义OPEN_LOGGER时不从LoggerBehavior继承,这样类中
 *    不带SelectLogger,ReleaseLogger,GetLogger接口,通过编译器提示来
 *    确保调用时确实去掉日志功能)
 *  - 带记录错误信息的功能
 *  - 带记录异常描述信息的功能 
 */
#ifdef OPEN_LOGGER
class DeviceBehavior : public LastErrBehavior , public LoggerBehavior
#else
class DeviceBehavior : public LastErrBehavior
#endif
{
protected:
    //----------------------------------------------------- 
    /// 标记用户调用的函数是否已经返回,以便区分是否还需要累加错误信息 
    bool _lastHasRet;
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 保存上次错误信息
     */
    /// 记录错误信息(内部需保证IsValid()=True)
    virtual void _logErr(int errCode, const char* errinfo = NULL)
    {
        _lasterr = errCode;
        // 如果是新进入的函数,清除错误信息以防累加 
        if(_lastHasRet)
        {
            _lastHasRet = false;
            if(IsAutoResetErr)
                _errinfo = "";
        }
        // 格式化 错误信息 
        _errinfo += "{ERR:[";
        _errinfo += ArgConvert::ToString(_lasterr);
        _errinfo += ",";
        _errinfo += TransErrToString(_lasterr);
        _errinfo += "],MSG:[";
        _errinfo += _strput(errinfo);
        _errinfo += "]}";

        LOGGER(_log<<"Error 错误码:<"<<_lasterr<<","<<TransErrToString(_lasterr)<<">,描述:<"<<_strput(errinfo)<<">\n");
    }
    /// 记录函数返回值信息到日志中(覆盖LoggerBehavior中的函数)
    template<class T>
    const T& _logRetValue(const T& val)
    {
        LOGGER(LoggerBehavior::_logRetValue(val));
        // 操作成功时清除错误状态
        if(val) 
        {
            _lasterr = DeviceError::Success;
            if(IsAutoResetErr)
                _errinfo = "";
        }
        _lastHasRet = true;
        return val;
    }
    /// 初始化
    inline void _init()
    {
        _lastHasRet = true;
        IsAutoResetErr = true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    DeviceBehavior() { _init(); }
    //----------------------------------------------------- 
    /// 重置错误信息(错误信息默认在失败的情况下会一直累加,需要手动清除) 
    inline void ResetErr()
    {
        _lasterr = DeviceError::Success;
        _errinfo = "";
    }
    
    /// 是否自动清除上次的错误信息  
    bool IsAutoResetErr;
    //----------------------------------------------------- 
};
//---------------------------------------------------------
/**
 * @brief 设备适配器行为
 * 
 * 操作下层设备的操作逻辑流程
 */
template<class IDeviceType>
class DevAdapterBehavior : public DeviceBehavior, public BaseDevAdapterBehavior<IDeviceType>
{
public:
    DevAdapterBehavior() : DeviceBehavior(), BaseDevAdapterBehavior<IDeviceType>() {}
};
//--------------------------------------------------------- 
} // namespace base
} // namespace zhou_yb
//=========================================================
