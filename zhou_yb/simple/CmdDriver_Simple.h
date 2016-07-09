//========================================================= 
/**@file CmdDriver_Simple.h
 * @brief 命令式驱动示例
 * 
 * @date 2016-05-06   18:02:12
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CMDDRIVER_SIMPLE_H_
#define _LIBZHOUYB_CMDDRIVER_SIMPLE_H_
//--------------------------------------------------------- 
#include "../zhouyb_lib.h"

#include "../application/driver/CommandDriver.h"
#include "../application/driver/CommandDriver.cpp"
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct CmdDriver_Simple
{
    static void Main(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {

    }
};
//--------------------------------------------------------- 
} // namespace simple 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CMDDRIVER_SIMPLE_H_
//========================================================= 