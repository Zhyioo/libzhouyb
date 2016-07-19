//========================================================= 
/**@file ZZ_FingerDevAdapter.h
 * @brief 中正指纹仪驱动
 * 
 * @date 2016-07-18   15:03:16
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ZZ_FINGERDEVADAPTER_H_
#define _LIBZHOUYB_ZZ_FINGERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../device/cmd_adapter/ComICCardCmdAdapter.h"
using zhou_yb::device::cmd_adapter::ComICCardCmdAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace finger {
//--------------------------------------------------------- 
/// 中正指纹仪驱动
class ZZ_FingerDevAdapter :
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
public:

};
//--------------------------------------------------------- 
} // namespace finger
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ZZ_FINGERDEVADAPTER_H_
//========================================================= 