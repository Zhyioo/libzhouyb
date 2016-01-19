//========================================================= 
/**@file SrcExtension.h 
 * @brief 包含扩展类部分的资源文件
 * 
 * @date 2013-09-08   10:56:32 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#ifdef _WIN32
#   include "../src/extension/WinRegistryKey.cpp"

//#   include "../src/extension/WmiInvoker.cpp"
#endif

#ifndef NO_INCLUDE_SECURITY_SOURCE
#   include "../src/extension/MD5_Provider.cpp"
#   include "../src/extension/DES_Provider.cpp"
#   include "../src/extension/Base64_Provider.cpp"
#   include "../src/extension/SM3_Provider.cpp"
#   include "../src/extension/SM4_Provider.cpp"
#endif
//========================================================= 