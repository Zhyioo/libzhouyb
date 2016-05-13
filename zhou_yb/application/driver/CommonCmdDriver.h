//========================================================= 
/**@file CommonCmdDriver.h
 * @brief һЩ�����Ĺ�������
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
/// �������������
#define LC_CMD_METHOD(methodName) bool methodName(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
/// ���е���ILastErrBehavior�ӿ�
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
/// ���е���InterruptBehavior�ӿ�
#define LC_CMD_INTERRUPT(interruptInvoker) \
    LC_CMD_METHOD(UpdateInterrupter) \
    { \
        interruptInvoker.SetInterrupter(Interrupter); \
    }
/// ע���ж�������
#define RegisteInterrupter(className) _Registe("UpdateInterrupter", (*this), &className::UpdateInterrupter)

/// ���е���IBaseDevAdapterBehavior�ӿ�
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
/// ���е���ILoggerBehavior�ӿ�
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
/// �й���������ʵ��ILastErrBehavior
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
    /// ��ȡ�����������Ϣ(string�ַ�������)
    virtual const char* GetErrMessage()
    {
        if(_pMsg != NULL)
            return _pMsg->c_str();
        return "";
    }
    /// ���ô�����Ϣ
    virtual void ResetErr()
    {
        (*_pErr) = DeviceError::Success;
        (*_pMsg) = "";
    }
};
//--------------------------------------------------------- 
/// �й�InterruptBehavior
class InterruptInvoker : public selecter<Ref<InterruptBehavior> >
{
public:
    /// ѡ�����Ͷ���
    typedef selecter<Ref<InterruptBehavior> > SelecterType;
    /// �����ж���
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
/// �й�IBaseDevAdapterBehavior
template<class IDeviceType>
class DevAdapterInvoker : public selecter<Ref<IBaseDevAdapterBehavior<IDeviceType> > >
{
public:
    /// ѡ�����Ͷ���
    typedef selecter<Ref<IBaseDevAdapterBehavior<IDeviceType> > > SelecterType;
    /// �����豸
    void SelectDevice(const Ref<IDeviceType>& dev)
    {
        typename list<Ref<IBaseDevAdapterBehavior<IDeviceType> > >::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->SelectDevice(dev);
        }
    }
    /// �ͷ��豸 
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
/// �й�ILoggerBehavior
class LoggerInvoker : public selecter<Ref<ILoggerBehavior> >
{
public:
    /// ѡ�����Ͷ���
    typedef selecter<Ref<ILoggerBehavior> > SelecterType;
    /// ������־
    void SelectLogger(const LoggerAdapter& logger)
    {
        list<Ref<ILoggerBehavior> >::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->SelectLogger(logger);
        }
    }
    /// �ͷ���־  
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