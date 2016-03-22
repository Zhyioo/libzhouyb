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
#include <basetyps.h>
#include <winioctl.h>
#include <commctrl.h>
#include <usbioctl.h>
#include <usbiodef.h>
#include <cfgmgr32.h>
#include <tchar.h>

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
/// USB Hub信息
struct HubDescriptor
{
    string DeviceKey;
    uint Power;
    uint NumberOfPoints;
    uint PowerSwitching;
    bool IsCompoundDevice;
    uint Protection;
};
/// 系统总线信息
struct BusDescriptor
{
    /// 
    string DriverKey;
    ///
    uint VendorID;
    ///
    uint DeviceID;
    uint SubSysID;
    uint Revision;
    list<HubDescriptor> RootHub;
};
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
    bool DriverNameToDeviceDesc(const char_t* driverName, string& deviceDesc, bool isDeviceId)
    {
        DEVINST     devInst;
        DEVINST     devInstNext;
        CONFIGRET   cr;
        ULONG       walkDone = 0;
        ULONG       len;
        TCHAR buf[MAX_DEVICE_ID_LEN];

        // Get Root DevNode
        //
        cr = CM_Locate_DevNode(&devInst, NULL, 0);
        if(cr != CR_SUCCESS)
            return false;

        // Do a depth first search for the DevNode with a matching
        // DriverName value
        //
        while(!walkDone)
        {
            // Get the DriverName value
            //
            len = sizeof(buf) / sizeof(buf[0]);
            cr = CM_Get_DevNode_Registry_Property(devInst, CM_DRP_DRIVER, NULL, buf, &len, 0);

            // If the DriverName value matches, return the DeviceDescription
            //
            if(cr == CR_SUCCESS && _tcsicmp(driverName, buf) == 0)
            {
                len = sizeof(buf) / sizeof(buf[0]);
                if(isDeviceId)
                {
                    cr = CM_Get_Device_ID(devInst, buf, len, 0);
                }
                else
                {
                    cr = CM_Get_DevNode_Registry_Property(devInst, CM_DRP_DEVICEDESC, NULL, buf, &len, 0);
                }

                
                return (cr == CR_SUCCESS);
            }

            // This DevNode didn't match, go down a level to the first child.
            //
            cr = CM_Get_Child(&devInstNext, devInst, 0);

            if(cr == CR_SUCCESS)
            {
                devInst = devInstNext;
                continue;
            }

            // Can't go down any further, go across to the next sibling.  If
            // there are no more siblings, go back up until there is a sibling.
            // If we can't go up any further, we're back at the root and we're
            // done.
            //
            for(;;)
            {
                cr = CM_Get_Sibling(&devInstNext, devInst, 0);

                if(cr == CR_SUCCESS)
                {
                    devInst = devInstNext;
                    break;
                }

                cr = CM_Get_Parent(&devInstNext, devInst, 0);

                if(cr == CR_SUCCESS)
                {
                    devInst = devInstNext;
                }
                else
                {
                    walkDone = 1;
                    break;
                }
            }
        }

        return false;
    }
    bool GetHCDDriverKeyName(HANDLE HCD, string& driverName)
    {
        BOOL                    success;
        ULONG                   nBytes;
        USB_HCD_DRIVERKEY_NAME  driverKeyName;
        PUSB_HCD_DRIVERKEY_NAME driverKeyNameW;
        PTSTR                   driverKeyNameA;

        driverKeyNameW = NULL;
        // Get the length of the name of the driver key of the HCD
        //
        success = DeviceIoControl(HCD,
            IOCTL_GET_HCD_DRIVERKEY_NAME,
            &driverKeyName,
            sizeof(driverKeyName),
            &driverKeyName,
            sizeof(driverKeyName),
            &nBytes,
            NULL);

        if(!success)
            return false;

        // Allocate space to hold the driver key name
        //
        nBytes = driverKeyName.ActualLength;
        if(nBytes <= sizeof(driverKeyName))
            return false;

        char_t* pBuff = simple_alloc<char_t>::allocate(nBytes);
        driverKeyNameW = reinterpret_cast<PUSB_HCD_DRIVERKEY_NAME>(pBuff);
        if(driverKeyNameW == NULL)
            return false;

        // Get the name of the driver key of the device attached to
        // the specified port.
        //
        success = DeviceIoControl(HCD,
            IOCTL_GET_HCD_DRIVERKEY_NAME,
            driverKeyNameW,
            nBytes,
            driverKeyNameW,
            nBytes,
            &nBytes,
            NULL);

        if(success)
        {
            CharConverter cvt;
            driverName = cvt.to_char(driverKeyNameW->DriverKeyName);
        }
        simple_alloc<char_t>::deallocate(pBuff);
        return Tobool(success);
    }
    bool GetHostController(HANDLE hHCDev, BusDescriptor& bus)
    {
        if(!GetHCDDriverKeyName(hHCDev, bus.DriverKey))
            return false;



        return true;
    }
public:
    size_t EnumUsbBus(list<BusDescriptor>& buslist)
    {
        LOG_FUNC_NAME();

        const size_t NUM_HCS_TO_CHECK = 10;

        CharConverter cvt;
        char_t HCName[16];
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
                buslist.push_back(BusDescriptor());

                buslist.back().DriverKey = cvt.to_char(HCName);
                buslist.back().DriverKey += "\\\\.\\";

                GetHostController(hHCDev, buslist.back());
                CloseHandle(hHCDev);
            }
        }

        return 0;
    }
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
        CharConverter cvt;

        for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
        {
            //get device description information
            if(!SetupDiGetDeviceInstanceId(hDevInfo, &DeviceInfoData, (PTSTR)did, buffersize, &req_bufsize))
                continue;
            if(!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DRIVER, &DataT, (LPBYTE)locid, buffersize, &req_bufsize))
                continue;

            devlist.push_back(cvt.to_char(locid));

            if(!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, &DataT, (LPBYTE)locdesc, buffersize, &req_bufsize))
                continue;

            devlist.push_back(cvt.to_char(locdesc));
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
//========================================================= 
