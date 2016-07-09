//========================================================= 
/**@file Command_Simple.h
 * @brief 
 * 
 * @date 2016-04-23   23:35:16
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMMAND_SIMPLE_H_
#define _LIBZHOUYB_COMMAND_SIMPLE_H_
//--------------------------------------------------------- 
#include <application/driver/CommandDriver.h>
#include <application/driver/CommandDriver.cpp>
#include <application/driver/CommonCmdDriver.h>
using namespace zhou_yb::application::driver;

#include <extension/ability/SplitArgParser.h>
using zhou_yb::extension::ability::SplitArgParser;
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct Command_Simple : public LoggerBehavior, public RefObject
{
    LC_CMD_METHOD(OK_Method)
    {
        _log.WriteLine("Call OK_Method");
        return true;
    }
    LC_CMD_METHOD(FailedMethod)
    {
        _log.WriteLine("Call FailedMethod");
        return false;
    }
    LC_CMD_METHOD(PutMessage)
    {
        _log.WriteLine("Call PutMessage");
        _log.WriteLine(arg["Msg"].To<string>());
        return true;
    }

    static void Main(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        LOG_FUNC_NAME();

        Command_Simple cmd;
        CommandDriver<SplitArgParser> cmddriver;

        cmd.SelectLogger(_log);
        cmddriver.SelectLogger(_log);

        Ref<Command> okCmd = Command::Make(cmd, &Command_Simple::OK_Method);
        Ref<Command> failedCmd = Command::Make(cmd, &Command_Simple::FailedMethod);
        Ref<Command> msgCmd1 = Command::Make(cmd, &Command_Simple::PutMessage);
        Ref<Command> msgCmd2 = Command::Make(cmd, &Command_Simple::PutMessage);
        Ref<Command> msgCmd3 = Command::Make(cmd, &Command_Simple::PutMessage);
        Ref<Command> msgCmd4 = Command::Make(cmd, &Command_Simple::PutMessage);

        char msgId[] = "Test";
        Ref<ComplexCommand> complexCmd = cmddriver.Registe(msgId);
        complexCmd->Bind(okCmd, ComplexCommand::RunOnSuccess);
        complexCmd->Bind(msgCmd1, ComplexCommand::RunOnSuccess, "Message1");
        complexCmd->Bind(failedCmd, ComplexCommand::RunOnSuccess);
        complexCmd->Bind(msgCmd2, ComplexCommand::RunOnSuccess, "Message2");
        complexCmd->Bind(msgCmd3, ComplexCommand::RunOnFailed, "Message3");
        complexCmd->Bind(failedCmd, ComplexCommand::RunOnFinal);
        complexCmd->Bind(msgCmd4, ComplexCommand::RunOnSuccess, "Message4");

        ByteBuilder recv(8);
        cmddriver.TransmitCommand(msgId, "", recv);
    }
};
//--------------------------------------------------------- 
} // namepscae simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMAND_SIMPLE_H_
//========================================================= 