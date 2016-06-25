//========================================================= 
/**@file WinICBC_MT_Driver.h
 * @brief 
 * 
 * @date 2016-06-25   14:29:12
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINICBC_MT_DRIVER_H_
#define _LIBZHOUYB_WINICBC_MT_DRIVER_H_
//--------------------------------------------------------- 
#include "ICBC_MT_CmdDriver.h"
#include "../WinDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// Windows下H002驱动
template<class TArgParser>
class WinICBC_MT_Driver : public WinDriver<ICBC_MT_CmdDriver<TArgParser> >
{
public:
    WinICBC_MT_Driver()
    {
        this->_Registe("SaveFingerImage", (*this), &WinICBC_MT_Driver::SaveFingerImage);
    }
    /// 获取指纹图片时不打日志
    LC_CMD_METHOD(SaveFingerImage)
    {
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINICBC_MT_DRIVER_H_
//========================================================= 