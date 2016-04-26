//========================================================= 
/**@file SrcExtension.h 
 * @brief 包含扩展类部分的资源文件
 * 
 * @date 2013-09-08   10:56:32 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SRCEXTENSION_H_
#define _LIBZHOUYB_SRCEXTENSION_H_
//--------------------------------------------------------- 
#ifdef _MSC_VER
#   include "../src/extension/WinRegistryKey.cpp"

//#   include "../src/extension/WmiInvoker.cpp"
#endif

#include "../src/extension/JniInvokerDevice.cpp"

#ifndef NO_INCLUDE_SECURITY_SOURCE
#   include "../src/extension/MD5_Provider.cpp"
#   include "../src/extension/DES_Provider.cpp"
#   include "../src/extension/Base64_Provider.cpp"
#   include "../src/extension/SM3_Provider.cpp"
#   include "../src/extension/SM4_Provider.cpp"
#endif
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SRCEXTENSION_H_
//========================================================= 
