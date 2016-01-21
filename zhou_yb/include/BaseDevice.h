//========================================================= 
/**@file BaseDevice.h 
 * @brief 包含base_device目录下的头文件
 * 
 * @date 2011-10-17   19:23:00 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../base_device/Handler.h"

#ifndef NO_INCLUDE_PCSC_SOURCE
#    include "../base_device/CCID_Device.h"
#endif
//---------------------------------------------------------
#ifdef _MSC_VER
//---------------------------------------------------------
/* Win32平台 */
#include "../base_device/win32/WinHandler.h"
#include "../base_device/win32/FileDevice.h"

#ifndef NO_INCLUDE_COM_SOURCE
#   include "../base_device/win32/ComDevice.h"
#endif

#ifdef INCLUDE_SOCKET_SOURCE
#   include "../base_device/win32/SocketDevice.h"
#   include "../base_device/win32/BluetoothDevice.h"
#endif

/// 是否使用HidDevice设备(需要安装DDK,在不使用的时候将代码注释防止由于没有DDK头文件而报错) 
#ifndef NO_INCLUDE_USB_SOURCE
#   include "../base_device/win32/HidDevice.h"
#endif 
//---------------------------------------------------------
#else
//---------------------------------------------------------
/* Linux平台 */
#include "../base_device/linux/FileDevice.h"

#ifndef NO_INCLUDE_COM_SOURCE
#   include "../base_device/linux/ComDevice.h"
#endif

#ifndef NO_INCLUDE_USB_SOURCE
#   include "../base_device/linux/UsbDevice.h"
#   include "../base_device/linux/HidDevice.h"
#endif
//---------------------------------------------------------
#endif // _MSC_VER

using namespace zhou_yb::base_device;

#ifdef _MSC_VER
using namespace zhou_yb::base_device::env_win32;
#else
using namespace zhou_yb::base_device::env_linux;
#endif
//=========================================================
