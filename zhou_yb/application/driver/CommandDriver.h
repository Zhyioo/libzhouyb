//========================================================= 
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
namespace driver {
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
        return _Registe(cmdName, Command::Make(obj, cmdHandler), bindArg);
    }
    /// 注册自己的内部命令
    Ref<ComplexCommand> _Registe(const char* cmdName, Ref<Command> cmd, const char* bindArg = NULL)
    {
        Ref<ComplexCommand> complexCmd = Registe(cmdName);
        // 第一个命令总是RunOnSuccess
        complexCmd->Bind(cmd, bindArg);
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
    /// 获取指定名称的命令
    Ref<ComplexCommand> LookUp(const char* cmdName)
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
        return Ref<ComplexCommand>();
    }
    /// 注册新的命令
    Ref<ComplexCommand> Registe(const char* cmdName)
    {
        // 查找现有的命令
        Ref<ComplexCommand> cmd = LookUp(cmdName);
        if(!cmd.IsNull())
            return cmd;

        list<ComplexCommand>::iterator itr = _cmd_collection.push_back();
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
} // namespace driver
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMANDDRIVER_H_
//========================================================= 
