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
class SetupDiHelper
{
public:
    void SetComPort()
    {
        GUID comGuid;
    }
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SETUPDIHELPER_H_
//========================================================= 