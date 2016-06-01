//========================================================= 
/**@file Extension.h 
 * @brief 包含扩展类目录下的文件
 * 
 * @date 2011-10-17   19:23:35 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_EXTENSION_H_
#define _LIBZHOUYB_EXTENSION_H_
//--------------------------------------------------------- 
/* ability和环境无关 */
#include "../extension/ability/StringHelper.h"
#include "../extension/ability/TextPrinter.h"
#include "../extension/ability/SharedDevice.h"

#ifndef NO_INCLUDE_JNI_SOURCE
#   include "../extension/ability/JniInvoker.h"
#   include "../extension/ability/JniInvokerDevice.h"
#   include "../extension/ability/JavaIInteractiveTransInvoker.h"
#endif

/* dev_helper */
#include "../extension/ability/dev_helper/DevHelper.h"
#include "../extension/ability/dev_helper/LastErrExtractor.h"
/* logger */
#include "../extension/ability/logger/FolderLogger.h"
#include "../extension/ability/logger/StringLogger.h"
#include "../extension/ability/logger/StringCallBackLogger.h"

/* ability和环境相关 */
#ifdef _MSC_VER
/* dev_helper */
#   include "../extension/ability/dev_helper/DevHelperForWin32.h"
/* logger */
#   include "../extension/ability/logger/HwndLogger.h"
/* idcard */
#ifndef NO_INCLUDE_IDCARD_SOURCE
#   include "../extension/ability/idcard/WinWltDecoder.h"
#   include "../extension/ability/idcard/WinSdtApiInvoker.h"
#   include "../extension/ability/idcard/WinIDCardImagePrinter.h"
#endif
/* win_helper */
#   include "../extension/ability/win_helper/WinHelper.h"
#   include "../extension/ability/win_helper/WinRegistryKey.h"
#   include "../extension/ability/win_helper/WinCCID_EscapeCommandHelper.h"
#   include "../extension/ability/win_helper/WinTestHelper.h"
#   include "../extension/ability/win_helper/WinCharsetConvert.h"
#else
/* dev_helper */
#    include "../extension/ability/dev_helper/DevHelperForLinux.h"
#endif

/* 和环境相关,根据配置按需添加 */
/* win_helper */
//#include "../extension/ability/win_helper/WmiInvoker.h"
//#include "../extension/ability/IconvCharsetConvert.h"
/* logger */
//#include "../extension/ability/logger/JniStringLogger.h"
/* idcard */
//#include "../extension/ability/idcard/AndroidWltDecoder.h"

using namespace zhou_yb::extension::ability;
//--------------------------------------------------------- 
/* security */
#ifndef NO_INCLUDE_SECURITY_SOURCE
#   include "../extension/security/DES_Provider.h"
#   include "../extension/security/MD5_Provider.h"
#   include "../extension/security/Base64_Provider.h"

#ifndef NO_INCLUDE_PBOC_SOURCE
#   include "../extension/security/PBOC_Provider.h"
#endif

/* 国密算法 */
#   include "../extension/security/SM3_Provider.h"
#   include "../extension/security/SM4_Provider.h"

/* 是否引用OpenSSL的封装库 */
#ifndef NO_INCLUDE_OPENSSL_SOURCE
#   include "../extension/security/openssl/OpenSSL_Base.h"
/* 该SM2需要依赖OpenSSL实现 */
#   include "../extension/security/openssl/SM2_Provider.h"

#endif 

using namespace zhou_yb::extension::security;
#endif
//--------------------------------------------------------- 
/* special */
#include "../extension/special/AutoAdapter.h"
#include "../extension/special/AdapterDevice.h"

using namespace zhou_yb::extension::special;
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_EXTENSION_H_
//=========================================================
