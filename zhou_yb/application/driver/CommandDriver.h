﻿//========================================================= 
/**@file LC_CommandDriver.h
 * @brief LC基于命令操作方式的驱动
 * 
 * @date 2016-04-02   14:48:29
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMMANDDRIVER_H_
#define _LIBZHOUYB_COMMANDDRIVER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver
{
//--------------------------------------------------------- 
/// 类中命令函数定义
#define LC_CMD_METHOD(methodName) bool methodName(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
/// 类中导出ILastErrBehavior接口
#define LC_CMD_LASTERR(lastErr) \
    virtual int GetLastErr() const \
    { \
        return lastErr.GetLastErr(); \
    } \
    virtual const char* GetErrMessage() \
    { \
        return lastErr.GetErrMessage(); \
    } \
    virtual void ResetErr() \
    { \
        return lastErr.ResetErr(); \
    }
/// 类中导出IBaseDevAdapterBehavior接口
#define LC_CMD_ADAPTER(deviceType, adapter) \
    virtual void SelectDevice(const Ref<deviceType>& dev) \
    { \
        BaseDevAdapterBehavior::SelectDevice(dev); \
        adapter.SelectDevice(dev); \
    } \
    virtual void ReleaseDevice() \
    { \
        BaseDevAdapterBehavior::ReleaseDevice(); \
        adapter.ReleaseDevice(); \
    }
/// 类中导出ILoggerBehavior接口
#ifdef OPEN_LOGGER
#   define LC_CMD_LOGGER(logger) \
    virtual void SelectLogger(const LoggerAdapter& log) \
    { \
        LoggerBehavior::SelectLogger(log); \
        logger.SelectLogger(log); \
    } \
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL) \
    { \
        LoggerBehavior::ReleaseLogger(plog); \
        logger.ReleaseLogger(plog); \
    }
#else
#   define LC_CMD_LOGGER(logger) 
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
/// Command接口
struct ICommandHandler
{
    /// 命令解析器类型
    typedef IArgParser<string, string> CmdArgParser;
    /// 执行命令
    virtual bool TransCommand(CmdArgParser&, CmdArgParser&) = 0;
};
//--------------------------------------------------------- 
/// 类成员函数的命令形式
template<class T>
class CommandHandler : public ICommandHandler
{
public:
    typedef bool(T::*fpOnCommand)(ICommandHandler::CmdArgParser&, ICommandHandler::CmdArgParser&);
protected:
    Ref<T> _pObj;
    fpOnCommand _fpOnCommand;
public:
    CommandHandler(T& obj, const fpOnCommand cmdHandle)
    {
        _pObj = Ref<T>(obj);
        _fpOnCommand = cmdHandle;
    }
    virtual bool TransCommand(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
    {
        if(_pObj.IsNull() || _fpOnCommand == NULL)
            return false;
        return ((*_pObj).*_fpOnCommand)(arg, rlt);
    }
};
/// C全局函数的命令方式
template<>
class CommandHandler<void> : public ICommandHandler
{
public:
    typedef bool(*fpOnCommand)(ICommandHandler::CmdArgParser&, ICommandHandler::CmdArgParser&);
protected:
    fpOnCommand _fpOnCommand;
public:
    CommandHandler(const fpOnCommand cmdHandle)
    {
        _fpOnCommand = cmdHandle;
    }
    virtual bool TransCommand(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
    {
        if(_fpOnCommand == NULL)
            return false;
        return _fpOnCommand(arg, rlt);
    }
};
//--------------------------------------------------------- 
/// 基本命令
class Command : public ICommandHandler, public RefObject
{
protected:
    //----------------------------------------------------- 
    static list<Command> _Cmds;

    shared_obj<ICommandHandler*> _handle;

    Command()
    {
        _handle.obj() = NULL;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    template<class T>
    static Ref<Command> Make(T& obj, const typename CommandHandler<T>::fpOnCommand cmdHandler)
    {
        Command cmd;
        cmd._handle = new CommandHandler<T>(obj, cmdHandler);
        Command::_Cmds.push_back(cmd);
        return Command::_Cmds.back();
    }
    static Ref<Command> Make(const CommandHandler<void>::fpOnCommand cmdHandler)
    {
        Command cmd;
        cmd._handle = new CommandHandler<void>(cmdHandler);
        Command::_Cmds.push_back(cmd);
        return Command::_Cmds.back();
    }
    //----------------------------------------------------- 
    virtual ~Command()
    {
        // 最后一个对象
        if(_handle.ref_count() < 2)
        {
            delete _handle.obj();
            _handle.obj() = NULL;

            list_helper<Command>::remove(Command::_Cmds, (*this));
        }
    }
    virtual bool TransCommand(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
    {
        if(_handle.obj() == NULL)
            return true;
        return _handle.obj()->TransCommand(arg, rlt);
    }
    //----------------------------------------------------- 
    bool operator==(const Command& obj)
    {
        return _handle == obj._handle;
    }
    bool operator!=(const Command& obj)
    {
        return !(operator==(obj));
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 复合命令
class ComplexCommand : public LoggerBehavior, public RefObject
{
public:
    //----------------------------------------------------- 
    /// 命令执行选项
    enum CommandOption
    {
        /// 上一个命令成功时执行
        RunOnSuccess,
        /// 上一个命令失败时执行
        RunOnFailed,
        /// 任何时候都执行命令,并且忽略该命令的执行结果
        RunOnFinal,
        /// 执行完该命令后不再继续执行后续的命令
        RunOnEOF
    };
    /// 命令名称
    string Name;
protected:
    struct CommandNode
    {
        Ref<Command> Cmd;
        Ref<ComplexCommand> ComplexCmd;
        bool IsBind;
        string Argument;
        CommandOption Option;
    };
    /// 命令
    shared_obj<list<CommandNode> > _cmds;
    /// 设置属性
    void _Bind(CommandNode& node, CommandOption option, const char* bindArg)
    {
        node.IsBind = (bindArg != NULL);
        if(node.IsBind)
            node.Argument = bindArg;
        node.Option = option;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 绑定基本命令到该组中(前置)
    void PreBind(Ref<Command> cmd, CommandOption option, const char* bindArg = NULL)
    {
        list<CommandNode>::iterator itr = _cmds.obj().push_front();
        itr->Cmd = cmd;
        itr->ComplexCmd = Ref<ComplexCommand>();
        _Bind(*itr, option, bindArg);
    }
    /// 绑定基本命令到该组中(前置)
    inline void PreBind(Ref<Command> cmd, const char* bindArg = NULL)
    {
        PreBind(cmd, ComplexCommand::RunOnSuccess, bindArg);
    }
    /// 绑定基本命令到该组中(后置)
    void Bind(Ref<Command> cmd, CommandOption option, const char* bindArg = NULL)
    {
        list<CommandNode>::iterator itr = _cmds.obj().push_back();
        itr->Cmd = cmd;
        itr->ComplexCmd = Ref<ComplexCommand>();
        _Bind(*itr, option, bindArg);
    }
    /// 绑定基本命令到该组中(后置)
    inline void Bind(Ref<Command> cmd, const char* bindArg = NULL)
    {
        Bind(cmd, ComplexCommand::RunOnSuccess, bindArg);
    }
    /// 绑定复合命令到该组中(前置)
    void PreBind(Ref<ComplexCommand> cmd, CommandOption option, const char* bindArg = NULL)
    {
        list<CommandNode>::iterator itr = _cmds.obj().push_front();
        itr->Cmd = Ref<Command>();
        itr->ComplexCmd = cmd;
        _Bind(*itr, option, bindArg);
    }
    inline void PreBind(Ref<ComplexCommand> cmd, const char* bindArg = NULL)
    {
        PreBind(cmd, ComplexCommand::RunOnSuccess, bindArg);
    }
    /// 绑定复合命令到该组中(后置)
    void Bind(Ref<ComplexCommand> cmd, CommandOption option, const char* bindArg = NULL)
    {
        list<CommandNode>::iterator itr = _cmds.obj().push_back();
        itr->Cmd = Ref<Command>();
        itr->ComplexCmd = cmd;
        _Bind(*itr, option, bindArg);
    }
    inline void Bind(Ref<ComplexCommand> cmd, const char* bindArg = NULL)
    {
        Bind(cmd, ComplexCommand::RunOnSuccess, bindArg);
    }
    /// 运行命令
    template<class TArgParser>
    bool RunCommand(ICommandHandler::CmdArgParser& arg, ICommandHandler::CmdArgParser& rlt)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "RunCommand:<" << Name << ">\n");
        list<CommandNode>::iterator itr;
        bool bCommand = true;
        for(itr = _cmds.obj().begin();itr != _cmds.obj().end(); ++itr)
        {
            if(itr->Cmd.IsNull() && itr->ComplexCmd.IsNull())
            {
                LOGGER(_log.WriteLine("Null Command"));
                continue;
            }
            // 上次执行失败
            if(!bCommand && ((itr->Option) == RunOnSuccess))
            {
                LOGGER(_log.WriteLine("RunOnSuccess Skipped"));
                continue;
            }
            if(bCommand && ((itr->Option) == RunOnFailed))
            {
                LOGGER(_log.WriteLine("RunOnFailed Skipped"));
                continue;
            }

            bool bSubCommand = true;
            ICommandHandler::CmdArgParser* pArg = &arg;
            TArgParser bindArg;

            if(itr->IsBind)
            {
                LOGGER(_log << "BindArg:<" << itr->Argument << ">\n");
                bindArg.Parse(itr->Argument.c_str());
                pArg = &bindArg;
            }
            if(itr->Cmd.IsNull())
            {
                bSubCommand = itr->ComplexCmd->RunCommand<TArgParser>(*pArg, rlt);
                LOGGER(_log << "ComplexCommand:<" << bSubCommand << ">\n");
            }
            else
            {
                bSubCommand = itr->Cmd->TransCommand(*pArg, rlt);
                LOGGER(_log << "SimpleCommand:<" << bSubCommand << ">\n");
            }

            if((itr->Option) != RunOnFinal)
                bCommand = bSubCommand;
            LOGGER(_log << "CommandStatus:<" << bCommand << ">\n");
            // 最后一个命令
            if((itr->Option == RunOnEOF))
                break;
        }
        return _logRetValue(bCommand);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 命令集合
class CommandCollection
{
protected:
    //----------------------------------------------------- 
    /// 命令
    list<ComplexCommand> _cmd_collection;
    /// 注册自己的内部命令
    template<class T>
    Ref<ComplexCommand> _Registe(const char* cmdName, T& obj, const typename CommandHandler<T>::fpOnCommand cmdHandler, const char* bindArg = NULL)
    {
        Ref<ComplexCommand> complexCmd = Registe(cmdName);
        // 第一个命令总是RunOnSuccess
        complexCmd->Bind(Command::Make(obj, cmdHandler), ComplexCommand::RunOnSuccess, bindArg);
        return complexCmd;
    }
    /// 绑定命令(后置)
    size_t _Bind(list<Ref<ComplexCommand> >& cmdCollection, Ref<Command> cmd,
        ComplexCommand::CommandOption option, const char* bindArg = NULL)
    {
        list<Ref<ComplexCommand> >::iterator itr;
        size_t count = 0;
        for(itr = cmdCollection.begin();itr != cmdCollection.end(); ++itr)
        {
            (*itr)->Bind(cmd, option, bindArg);
            ++count;
        }
        return count;
    }
    /// 绑定命令(后置)
    inline size_t _Bind(list<Ref<ComplexCommand> >& cmdCollection, Ref<Command> cmd, const char* bindArg = NULL)
    {
        return _Bind(cmdCollection, cmd, ComplexCommand::RunOnSuccess, bindArg);
    }
    /// 绑定命令(前置)
    size_t _PreBind(list<Ref<ComplexCommand> >& cmdCollection, Ref<Command> cmd,
        ComplexCommand::CommandOption option, const char* bindArg = NULL)
    {
        list<Ref<ComplexCommand> >::iterator itr;
        size_t count = 0;
        for(itr = cmdCollection.begin();itr != cmdCollection.end(); ++itr)
        {
            (*itr)->PreBind(cmd, option, bindArg);
            ++count;
        }
        return count;
    }
    /// 绑定命令(前置)
    inline size_t _PreBind(list<Ref<ComplexCommand> >& cmdCollection, Ref<Command> cmd, const char* bindArg = NULL)
    {
        return _PreBind(cmdCollection, cmd, ComplexCommand::RunOnSuccess, bindArg);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 获取所有支持的命令
    inline virtual list<Ref<ComplexCommand> > GetCommand(const char* cmdName)
    {
        list<Ref<ComplexCommand> > cmdlist;
        list<ComplexCommand>::iterator itr;
        ByteArray nameArray(cmdName);
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            if(nameArray.IsEmpty() || StringConvert::Contains(itr->Name.c_str(), nameArray, true))
            {
                cmdlist.push_back(*itr);
            }
        }
        return cmdlist;
    }
    /// 注册新的命令
    Ref<ComplexCommand> Registe(const char* cmdName)
    {
        // 查找现有的命令
        list<ComplexCommand>::iterator itr;
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            if(StringConvert::Compare(
                ByteArray(itr->Name.c_str(), itr->Name.length()),
                cmdName, true))
            {
                return (*itr);
            }
        }
        itr = _cmd_collection.push_back();
        itr->Name = _strput(cmdName);
        return (*itr);
    }
    /// 解注册命令
    size_t Unregiste(const char* cmdName)
    {
        size_t count = 0;
        list<ComplexCommand>::iterator itr;
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            if(StringConvert::Compare(
                ByteArray(itr->Name.c_str(), itr->Name.length()),
                cmdName, true))
            {
                itr = list_helper<ComplexCommand>::erase(_cmd_collection, itr);
                ++count;
            }
        }
        return count;
    }
    /// 注册命令组
    size_t Registe(list<Ref<ComplexCommand> >& cmdCollection, Ref<ComplexCommand> preCmd, Ref<ComplexCommand> endCmd)
    {
        list<Ref<ComplexCommand> >::iterator itr;
        size_t count = 0;
        for(itr = cmdCollection.begin();itr != cmdCollection.end(); ++itr)
        {
            ++count;
            Ref<ComplexCommand> complexCmd = Registe((*itr)->Name.c_str());
            if(!preCmd.IsNull())
            {
                complexCmd->PreBind(preCmd);
            }
            complexCmd->Bind(*itr);
            if(!endCmd.IsNull())
            {
                complexCmd->Bind(endCmd);
            }
        }
        return count;
    }
    /// 注册命令组
    inline size_t Registe(list<Ref<ComplexCommand> >& cmdCollection)
    {
        Ref<ComplexCommand> nullCmd;
        return Registe(cmdCollection, nullCmd, nullCmd);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 命令驱动接口
struct ICommandDriver
{
    /// 执行命令
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv) = 0;
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
     * @retval CMD
     */
    LC_CMD_METHOD(EnumCommand)
    {
        list<ComplexCommand>::iterator itr;
        for(itr = _cmd_collection.begin();itr != _cmd_collection.end(); ++itr)
        {
            rlt.PutValue("CMD", (*itr).Name);
        }
        return true;
    }
    /**
     * @brief 执行指定的命令
     * @date 2016-05-07 12:14
     * 
     * @param [in] arglist
     * - 参数
     *  - CMD 命令
     *  - ARG 参数
     * .
     * @retval RLT 结果
     */
    LC_CMD_METHOD(OnCommand)
    {
        string cmd = arg["CMD"].To<string>();
        string send = arg["ARG"].To<string>();
        ByteBuilder recv(32);
        bool bRet = bRet = TransmitCommand(cmd.c_str(), send.c_str(), recv);
        if(bRet) rlt.PutValue("RLT", recv.GetString());
        return bRet;
    }
    /**
     * @brief 获取上次错误码和错误信息
     * @date 2016-05-07 13:39
     * 
     * @retval CODE 错误码
     * @retval MSG 错误信息
     */
    LC_CMD_METHOD(LastError)
    {
        rlt.PutValue("CODE", ArgConvert::ToString<int>(GetLastErr()));
        rlt.PutValue("MSG", GetErrMessage());
        ResetErr();
        return true;
    }
    //----------------------------------------------------- 
    /// 消息分发函数
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv)
    {
        LOG_FUNC_NAME();
        // 查找命令表
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
            // 依次执行相同名称的命令
            ByteArray cmdName(itr->Name.c_str(), itr->Name.length());
            if(StringConvert::Compare(cmdName, sCmd, true))
            {
                LOGGER(_log << "Run Command:[" << index << "] <" << itr->Name << ">\n");
                if(!itr->RunCommand<TArgParser>(arg, rlt))
                    return false;
            }
        }
        rlt.ToString(recv);
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace driver
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMANDDRIVER_H_
//========================================================= 
