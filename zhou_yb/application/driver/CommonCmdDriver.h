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
        return true; \
    } \
    void SetInterrupter(const Ref<IInterrupter>& interrupter) \
    { \
        Interrupter = interrupter; \
        interruptInvoker.SetInterrupter(interrupter); \
    }
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
class InterruptInvoker : public selecter<Ref<InterruptBehavior> >, public InterruptBehavior
{
public:
    /// ѡ�����Ͷ���
    typedef selecter<Ref<InterruptBehavior> > SelecterType;
    /// �����ж���
    virtual void SetInterrupter(const Ref<IInterrupter>& interrupter)
    {
        InterruptBehavior::SetInterrupter(interrupter);

        list<Ref<InterruptBehavior> >::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            (*itr)->SetInterrupter(interrupter);
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
/// ���������ӿ�
struct ICommandDriver
{
    /// ִ������
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv) = 0;
};
//--------------------------------------------------------- 
/// �������ʽ������
template<class TArgParser>
class CommandDriver : 
    public DeviceBehavior, 
    public ICommandDriver,
    public CommandCollection, 
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// ����ת��������
    typedef TArgParser ArgParserType;
    //----------------------------------------------------- 
    /* ��־��ؽӿ���д */
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
     * @brief ö������֧�ֵ�����
     * @date 2016-05-07 11:11
     * 
     * @retval CMD
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
     * @brief ִ��ָ��������
     * @date 2016-05-07 12:14
     * 
     * @param [in] arglist
     * - ����
     *  - CMD ����
     *  - ARG ����
     * .
     * @retval RLT ���
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
     * @brief ��ȡ�ϴδ�����ʹ�����Ϣ
     * @date 2016-05-07 13:39
     * 
     * @retval CODE ������
     * @retval MSG ������Ϣ
     */
    LC_CMD_METHOD(LastError)
    {
        rlt.PushValue("CODE", ArgConvert::ToString<int>(GetLastErr()));
        rlt.PushValue("MSG", GetErrMessage());
        ResetErr();
        return true;
    }
    //----------------------------------------------------- 
    /// ��Ϣ�ַ�����
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv)
    {
        LOG_FUNC_NAME();
        LOGGER(ByteBuilder callName = sCmd;
        _log << "Call Command:<" << callName.GetString();
        callName = send;
        if(!callName.IsEmpty()) _log << " : " << callName.GetString();
        _log << ">\n");
        // ���������
        list<ComplexCommand>::iterator itr;
        TArgParser arg;
        TArgParser rlt;
        arg.Parse(send);

        bool bCommand = true;
        LOGGER(size_t index = 0;
        size_t subIndex = 0);
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            LOGGER(++index);
            // ����ִ����ͬ���Ƶ�����
            ByteArray cmdName(itr->Name.c_str(), itr->Name.length());
            if(StringConvert::Compare(cmdName, sCmd, true))
            {
                LOGGER(_log << "Run Command:[" << index << "]\n");
                if(!itr->RunCommand<TArgParser>(arg, rlt))
                {
                    return _logRetValue(false);
                }
            }
        }
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