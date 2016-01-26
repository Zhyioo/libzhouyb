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
    size_t EnumDevice(list<string>& devlist)
    {
        GUID USB_GUID = { 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED };
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&USB_GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        DWORD required_size = 0;
        int i;
        SP_DEVINFO_DATA DeviceInfoData = { sizeof(DeviceInfoData) };
        DWORD DataT;
        DWORD buffersize = 1024;
        DWORD req_bufsize = 0;

        char_t did[1024] = { 0 };
        char_t locid[1024] = { 0 };
        char_t locdesc[1024] = { 0 };

        for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
        {
            //get device description information
            if(!SetupDiGetDeviceInstanceId(hDevInfo, &DeviceInfoData, (PTSTR)did, buffersize, &req_bufsize))
                continue;
            if(!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DRIVER, &DataT, (LPBYTE)locid, buffersize, &req_bufsize))
                continue;

            devlist.push_back(locid);

            if(!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, &DataT, (LPBYTE)locdesc, buffersize, &req_bufsize))
                continue;

            devlist.push_back(locdesc);
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);

        return 0;
    }
};
//--------------------------------------------------------- 
} // namesapce env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#pragma warning(default:4996)
//========================================================= 
