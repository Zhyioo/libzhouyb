//========================================================= 
/**@file SrcDevice.h 
 * @brief 包含device库中的一些全局变量、函数 
 * 
 * @date 2011-10-17   19:33:15 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
// device
#include "../src/device/MagneticDevAdapter.cpp"

#ifndef NO_INCLUDE_ICCARD_SOURCE
#   include "../src/device/ComICCardCmdAdapter.cpp"
#   include "../src/device/ComICCardDevAdapter.cpp"
#   include "../src/device/ComStorageCardDevAdapter.cpp"
#   include "../src/device/TlvElement.cpp"
#endif

#ifndef NO_INCLUDE_HEALTHCARD_SOURCE
#   include "../src/device/HealthCardAppAdapter.cpp"
#endif 

#ifndef NO_INCLUDE_PBOC_SOURCE
#   include "../src/device/PBOC_v2_0_TagMap.cpp"
#endif 
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_IDCARD_SOURCE
#   include "../src/device/IDCardCharsetConvert.cpp"
#   include "../src/device/SdtApiDevAdapter.cpp"
#   include "../src/device/IDCardParser.cpp"
#endif
//--------------------------------------------------------- 
//=========================================================