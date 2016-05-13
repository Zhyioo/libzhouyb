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
    }
/// 注册中断器命令
#define RegisteInterrupter(className) _Registe("UpdateInterrupter", (*this), &className::UpdateInterrupter)

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
class InterruptInvoker : public selecter<Ref<InterruptBehavior> >
{
public:
    /// 选择类型定义
    typedef selecter<Ref<InterruptBehavior> > SelecterType;
    /// 设置中断器
    void SetInterrupter(const Ref<IInterrupter>& interrupter)
    {
        typename list<Ref<InterruptBehavior> >::iterator itr;
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
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->SelectDevice(dev);
        }
    }
    /// 释放设备 
    void ReleaseDevice()
    {
        typename list<Ref<IBaseDevAdapterBehavior<IDeviceType> > >::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->ReleaseDevice();
        }
    }
};
//--------------------------------------------------------- 
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
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->SelectLogger(logger);
        }
    }
    /// 释放日志  
    void ReleaseLogger(const LoggerAdapter* plog = NULL)
    {
        list<Ref<ILoggerBehavior> >::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->ReleaseLogger(plog);
        }
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMONCMDDRIVER_H_
//========================================================= 