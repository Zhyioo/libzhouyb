//========================================================= 
/**@file SharedDevice.h 
 * @brief 在适配器之间提供共享的设备
 * 
 * 多个操作某设备的适配器,只要其中的一个适配器操作使得其他的
 * 适配器将不会再重复操作  
 *
 * @date 2013-08-28   23:00:11 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SHAREDDEVICE_H_
#define _LIBZHOUYB_SHAREDDEVICE_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 在多个适配器之间共享的设备 
template<class T>
class SharedDevice
{
protected:
    shared_obj<T> _obj;
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SHAREDDEVICE_H_
//========================================================= 