//========================================================= 
/**@file UsbDevice.cpp 
 * @brief base_device目录下UsbDevice库中全局函数、变量的实现
 * 
 * 时间: 2013-6-13   18:19 
 * 作者: Zhyioo  
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_USBDEVICE_CPP_
#define _LIBZHOUYB_USBDEVICE_CPP_
//--------------------------------------------------------- 
#include "../../../base_device/linux/UsbDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// UsbDevice.Open参数配置项主键 UsbDevicePtr
const char UsbHandlerFactory::UsbDevicePtrKey[] = "UsbDevicePtr";
/// UsbDevice.Open参数配置项主键 Config
const char UsbHandlerFactory::ConfigKey[] = "Config";
/// UsbDevice.Open参数配置项主键 Interface
const char UsbHandlerFactory::InterfaceKey[] = "Interface";
/// UsbDevice.Open参数配置项主键 AltSetting
const char UsbHandlerFactory::AltSettingKey[] = "AltSetting";
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_USBDEVICE_CPP_
//=========================================================
