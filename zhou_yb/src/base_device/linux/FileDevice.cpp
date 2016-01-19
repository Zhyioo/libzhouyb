//========================================================= 
/**@file FileDevice.cpp 
 * @brief base_device目录下FileDevice库中全局函数、变量的实现
 * 
 * 时间: 2015-06-13   11:18:19 
 * 作者: Zhyioo  
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../base_device/linux/FileDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// FileDevice.Open打开设备的路径键值
const char FileHandlerFactory::DevPathKey[] = "DevPath";
/// FileDevice.Open打开设备权限的键值(int值)
const char FileHandlerFactory::AccessKey[] = "Access";
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device
} // namespace zhou_yb
//=========================================================
