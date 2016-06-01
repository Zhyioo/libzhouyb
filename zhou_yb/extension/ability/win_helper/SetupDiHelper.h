//========================================================= 
/**@file SetupDiHelper.h
 * @brief 设备管理器API
 * 
 * @date 2016-04-18   15:51:16
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SETUPDIHELPER_H_
#define _LIBZHOUYB_SETUPDIHELPER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

extern "C"
{
#   include <setupapi.h>
#   pragma comment(lib, "setupapi.lib")
}
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 设备管理器API辅助函数
class SetupDiHelper
{
public:
    /// 启用或禁用设备
    static bool EnableDevice(const char* friendlyName, BOOL isEnable = TRUE)
    {

    }
    static bool StatusChange(DWORD newStatus)
    {

    }
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SETUPDIHELPER_H_
//========================================================= 