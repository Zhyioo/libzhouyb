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
/// 类中命令函数定义
#define LC_CMD_METHOD(methodName) bool methodName(const ByteArray& send, ByteBuilder& recv)
//--------------------------------------------------------- 
/// 类成员函数的命令形式
template<class T>
class CommandHandler : public ITransceiveTrans, public RefObject
{
public:
    typedef bool(T::*fpOnCommand)(const ByteArray&, ByteBuilder&);
protected:
    Ref<T> _pObj;
    fpOnCommand _fpOnCommand;
public:
    CommandHandler(T& obj, const fpOnCommand cmdHandle)
    {
        _pObj = Ref<T>(obj);
        _fpOnCommand = cmdHandle;
    }
    virtual bool TransCommand(const ByteArray& sArg, ByteBuilder& recv)
    {
        if(_pObj.IsNull() || _fpOnCommand == NULL)
            return false;
        return ((*_pObj).*_fpOnCommand)(sArg, recv);
    }
};
/// C全局函数的命令方式
template<>
class CommandHandler<void> : public ITransceiveTrans, public RefObject
{
public:
    typedef bool(*fpOnCommand)(const ByteArray&, ByteBuilder&);
protected:
    fpOnCommand _fpOnCommand;
public:
    CommandHandler(const fpOnCommand cmdHandle)
    {
        _fpOnCommand = cmdHandle;
    }
    virtual bool TransCommand(const ByteArray& sArg, ByteBuilder& recv)
    {
        if(_fpOnCommand == NULL)
            return false;
        return _fpOnCommand(sArg, recv);
    }
};
//--------------------------------------------------------- 
/// 命令
class Command : public ITransceiveTrans, public RefObject
{
protected:
    //----------------------------------------------------- 
    static list<Command> _Cmds;

    shared_obj<string> _name;
    shared_obj<string> _argument;
    shared_obj<ITransceiveTrans*> _handle;

    Command()
    {
        _name.obj() = "";
        _argument.obj() = "";
        _handle.obj() = NULL;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    template<class T>
    static Ref<Command> Bind(const char* cmdId, T& obj, const typename CommandHandler<T>::fpOnCommand cmdHandler, const char* sArg = NULL)
    {
        Command cmd;
        cmd._name = _strput(cmdId);
        cmd._argument = _strput(sArg);
        cmd._handle = new CommandHandler<T>(obj, cmdHandler);
        Command::_Cmds.push_back(cmd);
        return Command::_Cmds.back();
    }
    static Ref<Command> Bind(const char* cmdId, const CommandHandler<void>::fpOnCommand cmdHandler, const char* sArg = NULL)
    {
        Command cmd;
        cmd._name = _strput(cmdId);
        cmd._argument = _strput(sArg);
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
    inline const char* Name() const
    {
        return _name.obj().c_str();
    }
    inline const char* Argument() const
    {
        return _argument.obj().c_str();
    }
    inline const ITransceiveTrans* Handle() const
    {
        return _handle.obj();
    }
    virtual bool TransCommand(const ByteArray& sArg, ByteBuilder& recv)
    {
        if(_handle.obj() == NULL)
            return true;
        ByteArray send = sArg;
        if(sArg.IsEmpty() && _argument.obj().length() > 0)
            send = ByteArray(_argument.obj().c_str(), _argument.obj().length());
        return _handle.obj()->TransCommand(send, recv);
    }
    bool operator==(const Command& other)
    {
        return (_name == other._name) && 
            (_argument == other._argument) && 
            (_handle == other._handle);
    }
    bool operator!=(const Command& other)
    {
        return !(operator==(other));
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 命令集合
class CommandCollection
{
protected:
    /// 命令
    list<Ref<Command> > _cmd_collection;
    /// 内部注册命令
    template<class T>
    Ref<Command> _Bind(const char* name, T&obj, const typename CommandHandler<T>::fpOnCommand cmdhandler)
    {
        Ref<Command> cmd = Command::Bind(name, obj, cmdhandler);
        _cmd_collection.push_back(cmd);
        return cmd;
    }
public:
    /// 获取所有支持的命令
    inline virtual list<Ref<Command> > GetCommand() { return _cmd_collection; }
};
//--------------------------------------------------------- 
/// 基于命令方式的驱动
class CommandDriver : 
    public DeviceBehavior,
    public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 命令结构
    struct CmdNode
    {
        /// 命令名称
        shared_obj<string> Cmd;
        /// 命令
        list<Ref<Command> > CmdHandle;
    };
    /// 调用命令表
    list<CmdNode> _cmds;
    /// 注册命令
    list<CmdNode>::iterator _RegisteCommand(const char* cmdName)
    {
        // 查找现有的命令
        list<CmdNode>::iterator itr;
        for(itr = _cmds.begin();itr != _cmds.end(); ++itr)
        {
            if(StringConvert::Compare(
                ByteArray(itr->Cmd.obj().c_str(), itr->Cmd.obj().length()),
                cmdName, true))
            {
                return itr;
            }
        }
        _cmds.push_back(CmdNode());
        itr = _cmds.end();
        --itr;
        _cmds.back().Cmd = _strput(cmdName);
        return itr;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 解析参数
    template<class TArg>
    static TArg Arg(list<string>& arglist, size_t index, TArg defaultVal = TArg())
    {
        list<string>::iterator itr = list_helper<string>::index_of(arglist, index);
        if(itr == arglist.end())
            return defaultVal;

        return ArgConvert::FromString<TArg>(itr->c_str());
    }
    //----------------------------------------------------- 
    CommandDriver()
    {
        RegisteCommand(Command::Bind("EnumCommand", (*this), &CommandDriver::EnumCommand));
    }
    //----------------------------------------------------- 
    /// 枚举所有支持的命令
    LC_CMD_METHOD(EnumCommand)
    {
        list<CmdNode>::iterator itr;
        for(itr = _cmds.begin();itr != _cmds.end(); ++itr)
        {
            recv += itr->Cmd.obj().c_str();
            recv += SPLIT_CHAR;
        }
        if(_cmds.size() > 0)
            recv.RemoveTail();

        return true;
    }
    //----------------------------------------------------- 
    /// 消息分发函数
    virtual bool TransmitCommand(const ByteArray& sCmd, const ByteArray& send, ByteBuilder& recv)
    {
        // 查找命令表
        list<CmdNode>::iterator itr;
        for(itr = _cmds.begin();itr != _cmds.end(); ++itr)
        {
            // 依次执行相同名称的命令
            if(StringConvert::Compare(ByteArray(itr->Cmd.obj().c_str(), itr->Cmd.obj().length()), sCmd, true))
            {
                LOGGER(_log << "Run Command:<" << itr->Cmd.obj() << ">\n");

                list<Ref<Command> >::iterator cmdItr;
                for(cmdItr = itr->CmdHandle.begin();cmdItr != itr->CmdHandle.end(); ++cmdItr)
                {
                    Ref<Command> cmd = (*cmdItr);
                    if(cmd.IsNull())
                        continue;
                    LOGGER(_log << "Sub Command:<" << cmd->Name() << ">\n");
                    LOGGER(_log << "Cmd Handle:<" << _hex_num(cmd->Handle()) << ">\n");

                    // 需要所有命令都执行成功才返回成功
                    if(!(cmd->TransCommand(send, recv)))
                        return false;
                }
            }
        }
        return true;
    }
    //----------------------------------------------------- 
    /// 消息回调函数注册
    bool RegisteCommand(const Ref<Command>& cmd, const char* cmdName = NULL)
    {
        if(cmd.IsNull())
            return false;
        // 查找现有的命令
        if(_is_empty_or_null(cmdName))
            cmdName = cmd->Name();
        list<CmdNode>::iterator itr = _RegisteCommand(cmdName);
        itr->CmdHandle.push_front(cmd);
        return true;
    }
    /// 注册命令集合
    size_t RegisteCommand(CommandCollection& cmdCollection,  
        const Ref<Command>& preCmd,
        const Ref<Command>& endCmd)
    {
        list<Ref<Command> > cmds = cmdCollection.GetCommand();
        list<Ref<Command> >::iterator itr;
        for(itr = cmds.begin();itr != cmds.end(); ++itr)
        {
            if(itr->IsNull())
                continue;

            list<CmdNode>::iterator cmdItr = _RegisteCommand((*itr)->Name());
            if(!preCmd.IsNull())
            {
                cmdItr->CmdHandle.push_back(preCmd);
            }
            cmdItr->CmdHandle.push_back(*itr);
            if(!endCmd.IsNull())
            {
                cmdItr->CmdHandle.push_back(endCmd);
            }
        }
        return cmds.size();
    }
    /// 注册命令集合
    size_t RegisteCommand(CommandCollection& cmdCollection)
    {
        Ref<Command> NullCmd;
        return RegisteCommand(cmdCollection, NullCmd, NullCmd);
    }
    /// 消息回调函数注销
    bool UnregisteCommand(const char* cmdName)
    {
        bool bRemove = false;
        list<CmdNode>::iterator itr;
        for(itr = _cmds.begin();itr != _cmds.end(); ++itr)
        {
            if(StringConvert::Contains(
                ByteArray(itr->Cmd.obj().c_str(), itr->Cmd.obj().length()), 
                cmdName, true))
            {
                itr = list_helper<CmdNode>::erase(_cmds, itr);
                bRemove = true;
            }
        }
        return bRemove;
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
