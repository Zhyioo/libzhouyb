//========================================================= 
/**@file UsbDevice.h
 * @brief Windows下 USB设备通信实现 
 * 
 * @date 2015-10-09   16:57:29
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "FileDevice.h"

#include <stdlib.h>
extern "C"
{
    #include <hidsdi.h>
    #include <setupapi.h>
    #pragma comment(lib, "setupapi.lib")
    #pragma comment(lib, "hid.lib")
}
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// USB设备
class UsbDevice
{
protected:
public:
    size_t EnumDevice()
    {

    }
};
//--------------------------------------------------------- 
} // namesapce env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#pragma warning(default:4996)
//========================================================= 
