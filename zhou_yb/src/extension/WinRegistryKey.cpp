//========================================================= 
/**@file WinRegistryKey.cpp 
 * @brief 注册表全局键值 
 * 
 * @date 2013-09-08   10:03:54 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINREGISTRYKEY_CPP_
#define _LIBZHOUYB_WINREGISTRYKEY_CPP_
//--------------------------------------------------------- 
#include "../../extension/ability/win_helper/WinRegistryKey.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/* 系统中默认的注册表项 */
RegistryKey RegistryKey::NullRegistryKey;
RegistryKey RegistryKey::CurrentUser(HKEY_CURRENT_USER, "HKEY_CURRENT_USER");
RegistryKey RegistryKey::ClassesRoot(HKEY_CLASSES_ROOT, "HKEY_CLASSES_ROOT");
RegistryKey RegistryKey::LocalMachine(HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE");
RegistryKey RegistryKey::Users(HKEY_USERS, "HKEY_USERS");
RegistryKey RegistryKey::CurrentConfig(HKEY_CURRENT_CONFIG, "HKEY_CURRENT_CONFIG");
RegistryKey RegistryKey::CurrentUserLocalSetting(HKEY_CURRENT_USER_LOCAL_SETTINGS, "HKEY_CURRENT_USER_LOCAL_SETTINGS");
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINREGISTRYKEY_CPP_
//========================================================= 