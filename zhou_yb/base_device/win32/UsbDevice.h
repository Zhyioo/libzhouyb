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
/// USB设备信息
struct UsbDescriptor
{
    /// 设备描述类型
    uint DescriptorType;
    /// 设备的VID
    ushort Vid;
    /// 设备的PID
    ushort Pid;
    /// 系统编号
    uint SysId;
    /// 
    uint Revision;
    /// 设备名称
    string Name;
    /// 设备文件路径
    string Path;
};
//--------------------------------------------------------- 
/// USB设备
class UsbDevice : public DeviceBehavior
{
public:
    typedef UsbDescriptor device_info;
protected:
    size_t EnumerateHostControllers()
    {

    }
    void EnumerateHostController(HANDLE hHCDev, const char_t* leafName)
    {

    }
public:
    size_t EnumUsbTree(list<UsbDescriptor>& devlist)
    {
        LOG_FUNC_NAME();

        const size_t NUM_HCS_TO_CHECK = 10;

        char_t HCName[16];
        string_t leafName;
        HANDLE hHCDev;

        for(size_t HCNum = 0;HCNum < NUM_HCS_TO_CHECK; ++HCNum)
        {
            sprintf_t(HCName, _T("\\\\.\\HCD%d"), HCNum);
            hHCDev = CreateFile(HCName,
                GENERIC_WRITE,
                FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
            if(hHCDev != INVALID_HANDLE_VALUE)
            {
                leafName = HCName;
                leafName +=_T("\\\\.\\");

                EnumerateHostController(hHCDev, leafName.c_str());
                CloseHandle(hHCDev);
            }
        }


        return 0;
    }
    size_t EnumDeviceOld(list<string>& devlist)
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
    bool IsExistOnHUB()
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
