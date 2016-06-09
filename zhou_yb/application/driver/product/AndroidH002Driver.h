//========================================================= 
/**@file AndroidH002Driver.h
 * @brief Android下H002驱动
 * 
 * @date 2016-06-08   18:37:52
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ANDROIDH002DRIVER_H_
#define _LIBZHOUYB_ANDROIDH002DRIVER_H_
//--------------------------------------------------------- 
#include "../CommonCmdDriver.h"
#include "../H002CmdDriver.h"

#include "../../tools/IJniCmdDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// Android下H002驱动
template<class TArgParser>
class AndroidH002Driver : public CommandDriver<TArgParser>
{
protected:
    LOGGER(FolderLogger _folder);

    JniEnvCmdDriver _dev;
    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;

    H002CmdDriver<TArgParser> _h002;
public:
    AndroidH002Driver() : CommandDriver<TArgParser>()
    {
        _dev.Interrupter = _h002.Interrupter;

        _h002.SelectDevice(_dev);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_dev, "DEV");
        _lastErr.Select(_h002, "H002");
        _objErr.Invoke(CommandDriver<TArgParser>::_lasterr, CommandDriver<TArgParser>::_errinfo);
        _lastErr.Select(_objErr, "APP");

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker), _dev, _h002;

        _Registe("NativeInit", (*this), &AndroidH002Driver::NativeInit);
        _Registe("NativeDestory", (*this), &AndroidH002Driver::NativeDestory);

        _Registe("EnumCommand", (*this), &AndroidH002Driver::EnumCommand);
        _Registe("LastError", (*this), &AndroidH002Driver::LastError);

        list<Ref<ComplaxCommand> > cmds = _h002.GetCommand("");
        Registe(cmds);
    }
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_LOGGER(_logInvoker);
    /**
     * @brief 初始化JNI调用
     * @date 2016-06-09 10:57
     * 
     * @param [in] Path : string 需要设置的日志目录
     */
    LC_CMD_METHOD(NativeInit)
    {
        LOGGER(string dir = arg["Path"].To<string>();
        _folder.Open(dir.c_str(), "driver", 2, FILE_K(256));
        CommandDriver<TArgParser>::_log.Select(_folder));

        return true;
    }
    LC_CMD_METHOD(NativeDestory)
    {
        LOGGER(_folder.Close();
        CommandDriver<TArgParser>::_log.Release());

        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ANDROIDH002DRIVER_H_
//========================================================= 