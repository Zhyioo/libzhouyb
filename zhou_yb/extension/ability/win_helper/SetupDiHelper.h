//========================================================= 
/**@file SetupDiHelper.h
 * @brief �豸������API
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
/// �豸������API��������
class SetupDiHelper
{
public:
    /// ���û�����豸
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