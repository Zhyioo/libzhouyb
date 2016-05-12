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
using zhou_yb::application::driver::CommandDriver;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 带中断的命令
class InterruptCmdDriver
{
protected:
    Ref<IInterrupter> _interrupter;
public:
    LC_CMD_METHOD(Interrupt)
    {
        return _interrupter->Interrupt();
    }
    LC_CMD_METHOD(InterruptReset)
    {
        _interrupter->Reset();
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMONCMDDRIVER_H_
//========================================================= 