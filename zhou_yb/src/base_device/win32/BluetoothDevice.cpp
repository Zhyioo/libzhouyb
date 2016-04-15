//========================================================= 
/**@file BluetoothDevice.cpp
 * @brief Windows下蓝牙设备资源文件 
 * 
 * @date 2015-07-13   20:35:30
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BLUETOOTHDEVICE_CPP_
#define _LIBZHOUYB_BLUETOOTHDEVICE_CPP_
//--------------------------------------------------------- 
#include "../../../base_device/win32/BluetoothDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// BluetoothParam生成参数配置项时的主键 RemoteNameKey
const char BluetoothParam::RemoteNameKey[] = "RemoteName";
/// BluetoothParam生成参数配置项时的主键 PasswordKey
const char BluetoothParam::PasswordKey[] = "Password";
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BLUETOOTHDEVICE_CPP_
//=========================================================