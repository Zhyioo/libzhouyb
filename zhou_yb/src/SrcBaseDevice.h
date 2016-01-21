//========================================================= 
/**@file SrcBaseDevice.h 
 * @brief 包含base_device库中的一些全局变量、函数
 * 
 * @date 2011-10-17   19:32:48 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
// base_device

#ifndef NO_INCLUDE_PCSC_SOURCE
/* Linux下和Windows下CCID协议接口相同使用相同的代码 */
#	include "../src/base_device/CCID_Device.cpp"
#endif
//--------------------------------------------------------- 
#ifdef _MSC_VER
//--------------------------------------------------------- 
#include "../src/base_device/win32/FileDevice.cpp"

#ifdef INCLUDE_SOCKET_SOURCE
#   include "../src/base_device/win32/SocketDevice.cpp"
#   include "../src/base_device/win32/BluetoothDevice.cpp"
#endif
//--------------------------------------------------------- 
#else
//---------------------------------------------------------
#include "../src/base_device/linux/FileDevice.cpp"

#ifndef NO_INCLUDE_USB_SOURCE
#   include "../src/base_device/linux/UsbDevice.cpp"
#endif
//--------------------------------------------------------- 
#endif // _MSC_VER
//--------------------------------------------------------- 
//=========================================================
