//========================================================= 
/**@file WinH002Driver.h
 * @brief Windows下H002驱动
 * 
 * @date 2016-05-15   16:50:10
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINH002DRIVER_H_
#define _LIBZHOUYB_WINH002DRIVER_H_
//--------------------------------------------------------- 
#include "H002CmdDriver.h"
#include "../WinDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// Windows下H002驱动
template<class TArgParser>
class WinH002Driver : public WinDriver<H002CmdDriver<TArgParser> > {};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINH002DRIVER_H_
//========================================================= 