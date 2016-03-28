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

#ifdef _MP
#undef _MP
#endif

#include <stdlib.h>
#include <windowsx.h>
#include <winioctl.h>
#include <initguid.h>
#include <devioctl.h>
#include <usbioctl.h>
#include <basetyps.h>
#include <dbt.h>
#include <tchar.h>

#include <commctrl.h>
#include <usbioctl.h>
#include <usbiodef.h>

#include <cfgmgr32.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "cfgmgr32.lib")

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
/* 相关USB数据结构定义 */
typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE *Next;
    UCHAR                           DescriptorIndex;
    USHORT                          LanguageID;
    USB_STRING_DESCRIPTOR           StringDescriptor[0];
} STRING_DESCRIPTOR_NODE, *PSTRING_DESCRIPTOR_NODE;
typedef enum _USBDEVICEINFOTYPE
{
    HostControllerInfo,
    RootHubInfo,
    ExternalHubInfo,
    DeviceInfo
} USBDEVICEINFOTYPE, *PUSBDEVICEINFOTYPE;
typedef struct _USBHOSTCONTROLLERINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;
    LIST_ENTRY                          ListEntry;
    PTSTR                               DriverKey;
    ULONG                               VendorID;
    ULONG                               DeviceID;
    ULONG                               SubSysID;
    ULONG                               Revision;
} USBHOSTCONTROLLERINFO, *PUSBHOSTCONTROLLERINFO;
typedef struct _USBROOTHUBINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;
    PUSB_NODE_INFORMATION               HubInfo;
    PTSTR                               HubName;
    PUSB_HUB_CAPABILITIES               HubCaps;
    PUSB_HUB_CAPABILITIES_EX            HubCapsEx;
} USBROOTHUBINFO, *PUSBROOTHUBINFO;
typedef struct _USBEXTERNALHUBINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;
    PUSB_NODE_INFORMATION               HubInfo;
    PTSTR                               HubName;
    PUSB_HUB_CAPABILITIES               HubCaps;
    PUSB_HUB_CAPABILITIES_EX            HubCapsEx;
    PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo;
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc;
    PSTRING_DESCRIPTOR_NODE             StringDescs;
} USBEXTERNALHUBINFO, *PUSBEXTERNALHUBINFO;
typedef struct _USBDEVICEINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;
    PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo;
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc;
    PSTRING_DESCRIPTOR_NODE             StringDescs;
} USBDEVICEINFO, *PUSBDEVICEINFO;
typedef struct _USB_INTERFACE_DESCRIPTOR2
{
    UCHAR  bLength;             // offset 0, size 1
    UCHAR  bDescriptorType;     // offset 1, size 1
    UCHAR  bInterfaceNumber;    // offset 2, size 1
    UCHAR  bAlternateSetting;   // offset 3, size 1
    UCHAR  bNumEndpoints;       // offset 4, size 1
    UCHAR  bInterfaceClass;     // offset 5, size 1
    UCHAR  bInterfaceSubClass;  // offset 6, size 1
    UCHAR  bInterfaceProtocol;  // offset 7, size 1
    UCHAR  iInterface;          // offset 8, size 1
    USHORT wNumClasses;         // offset 9, size 2
} USB_INTERFACE_DESCRIPTOR2, *PUSB_INTERFACE_DESCRIPTOR2;
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
/// USB Hub信息
struct HubDescriptor
{
    string DeviceKey;
    string Name;
    string Path;
    bool IsExternalHub;
    bool IsBusPowered;
    bool IsCompoundDevice;
    uint NumberOfPoints;
    string PowerSwitching;
    string Protection;
    bool Is2xCapable;
    /* 仅WinVista以上版本支持 */
    uint CapabilityFlags;
    bool IsHighSpeed;
    bool IsHighSpeedCapable;
    bool IsMulitTransCapable;
    bool IsMulitTransON;
    bool IsArmedWakeOnConnect;
    uint ReservedMBZ;
    /// 挂载的扩展HUB
    list<HubDescriptor> ExternalHub;
    /// 挂载的USB设备
    list<UsbDescriptor> ExteranlUsb;
};
/// 系统总线信息
struct BusDescriptor
{ 
    string DriverKey;
    string Name;
    uint VendorID;
    uint DeviceID;
    uint SubSysID;
    uint Revision;
    HubDescriptor RootHub;
};
//--------------------------------------------------------- 
/// USB设备
class UsbDevice : public DeviceBehavior
{
public:
    typedef UsbDescriptor device_info;
protected:
    bool DriverNameToDeviceDesc(const char* driverName, string& deviceDesc, BOOL isDeviceId)
    {
        DEVINST     devInst;
        DEVINST     devInstNext;
        CONFIGRET   cr;
        ULONG       walkDone = 0;
        ULONG       len;
        TCHAR buf[MAX_DEVICE_ID_LEN];
        CharConverter cvt;

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
            if(cr == CR_SUCCESS && _tcsicmp(cvt.to_char_t(driverName), buf) == 0)
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
                deviceDesc = cvt.to_char(buf);
                
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

        WCHAR* pBuff = simple_alloc<WCHAR>::allocate(nBytes);
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
        simple_alloc<WCHAR>::deallocate(pBuff, nBytes);
        return Tobool(success);
    }
    bool GetDriverKeyName(HANDLE Hub, ULONG ConnectionIndex, string& drvName)
    {
        BOOL                                success;
        ULONG                               nBytes;
        USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
        PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;

        driverKeyNameW = NULL;

        // Get the length of the name of the driver key of the device attached to
        // the specified port.
        //
        driverKeyName.ConnectionIndex = ConnectionIndex;

        success = DeviceIoControl(Hub,
            IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
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

        WCHAR* pBuff = simple_alloc<WCHAR>::allocate(nBytes);
        driverKeyNameW = reinterpret_cast<PUSB_NODE_CONNECTION_DRIVERKEY_NAME>(pBuff);
        if(driverKeyNameW == NULL)
            return false;

        // Get the name of the driver key of the device attached to
        // the specified port.
        //
        driverKeyNameW->ConnectionIndex = ConnectionIndex;

        success = DeviceIoControl(Hub,
            IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
            driverKeyNameW,
            nBytes,
            driverKeyNameW,
            nBytes,
            &nBytes,
            NULL);

        if(success)
        {
            CharConverter cvt;
            drvName = cvt.to_char(reinterpret_cast<WCHAR*>(driverKeyNameW->DriverKeyName));
        }
        simple_alloc<WCHAR>::deallocate(pBuff, nBytes);
        return true;
    }
    bool GetRootHubName(HANDLE HostController, string& hubName)
    {
        BOOL                success;
        ULONG               nBytes;
        USB_ROOT_HUB_NAME   rootHubName;
        PUSB_ROOT_HUB_NAME  rootHubNameW = NULL;

        success = DeviceIoControl(HostController,
            IOCTL_USB_GET_ROOT_HUB_NAME,
            0,
            0,
            &rootHubName,
            sizeof(rootHubName),
            &nBytes,
            NULL);

        if(!success)
            return false;

        nBytes = rootHubName.ActualLength;
        WCHAR* pBuff = simple_alloc<WCHAR>::allocate(nBytes);
        rootHubNameW = reinterpret_cast<PUSB_ROOT_HUB_NAME>(pBuff);
        if(rootHubNameW == NULL)
            return false;
        success = DeviceIoControl(HostController,
            IOCTL_USB_GET_ROOT_HUB_NAME,
            NULL,
            0,
            rootHubNameW,
            nBytes,
            &nBytes,
            NULL);
        if(success)
        {
            CharConverter cvt;
            hubName = cvt.to_char(reinterpret_cast<WCHAR*>(rootHubNameW->RootHubName));
        }

        simple_alloc<WCHAR>::deallocate(pBuff, nBytes);
        return true;
    }
    bool GetExternalHubName(HANDLE Hub, ULONG ConnectionIndex, string& hubName)
    {
        BOOL                        success;
        ULONG                       nBytes;
        USB_NODE_CONNECTION_NAME	extHubName;
        PUSB_NODE_CONNECTION_NAME   extHubNameW;

        extHubNameW = NULL;

        // Get the length of the name of the external hub attached to the
        // specified port.
        //
        extHubName.ConnectionIndex = ConnectionIndex;

        success = DeviceIoControl(Hub,
            IOCTL_USB_GET_NODE_CONNECTION_NAME,
            &extHubName,
            sizeof(extHubName),
            &extHubName,
            sizeof(extHubName),
            &nBytes,
            NULL);

        if(!success)
            return false;

        // Allocate space to hold the external hub name
        //
        nBytes = extHubName.ActualLength;
        if(nBytes <= sizeof(extHubName))
            return false;

        WCHAR* pBuff = reinterpret_cast<WCHAR*>(simple_alloc<WCHAR>::allocate(nBytes));
        extHubNameW = reinterpret_cast<PUSB_NODE_CONNECTION_NAME>(pBuff);
        if(extHubNameW == NULL)
            return false;

        // Get the name of the external hub attached to the specified port
        //
        extHubNameW->ConnectionIndex = ConnectionIndex;

        success = DeviceIoControl(Hub,
            IOCTL_USB_GET_NODE_CONNECTION_NAME,
            extHubNameW,
            nBytes,
            extHubNameW,
            nBytes,
            &nBytes,
            NULL);

        if(success)
        {
            CharConverter cvt;
            hubName = cvt.to_char(reinterpret_cast<WCHAR*>(extHubNameW->NodeName));
        }
        simple_alloc<WCHAR>::deallocate(pBuff, nBytes);
        return true;
    }
    PUSB_DESCRIPTOR_REQUEST GetConfigDescriptor(
        HANDLE  hHubDevice,
        ULONG   ConnectionIndex,
        UCHAR   DescriptorIndex)
    {
        BOOL    success;
        ULONG   nBytes;
        ULONG   nBytesReturned;

        UCHAR   configDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
            sizeof(USB_CONFIGURATION_DESCRIPTOR)];

        PUSB_DESCRIPTOR_REQUEST         configDescReq;
        PUSB_CONFIGURATION_DESCRIPTOR   configDesc;

        // Request the Configuration Descriptor the first time using our
        // local buffer, which is just big enough for the Cofiguration
        // Descriptor itself.
        //
        nBytes = sizeof(configDescReqBuf);

        configDescReq = (PUSB_DESCRIPTOR_REQUEST)configDescReqBuf;
        configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq + 1);

        // Zero fill the entire request structure
        //
        memset(configDescReq, 0, nBytes);

        // Indicate the port from which the descriptor will be requested
        //
        configDescReq->ConnectionIndex = ConnectionIndex;

        //
        // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
        // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
        //
        // USBD will automatically initialize these fields:
        //     bmRequest = 0x80
        //     bRequest  = 0x06
        //
        // We must inititialize these fields:
        //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
        //     wIndex    = Zero (or Language ID for String Descriptors)
        //     wLength   = Length of descriptor buffer
        //
        configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
            | DescriptorIndex;

        configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

        // Now issue the get descriptor request.
        //
        success = DeviceIoControl(hHubDevice,
            IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
            configDescReq,
            nBytes,
            configDescReq,
            nBytes,
            &nBytesReturned,
            NULL);

        if(!success)
        {
            return NULL;
        }

        if(nBytes != nBytesReturned)
        {
            return NULL;
        }

        if(configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
        {
            return NULL;
        }

        // Now request the entire Configuration Descriptor using a dynamically
        // allocated buffer which is sized big enough to hold the entire descriptor
        //
        nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + configDesc->wTotalLength;

        configDescReq = (PUSB_DESCRIPTOR_REQUEST)malloc(nBytes);

        if(configDescReq == NULL)
        {
            return NULL;
        }

        configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq + 1);

        // Indicate the port from which the descriptor will be requested
        //
        configDescReq->ConnectionIndex = ConnectionIndex;

        //
        // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
        // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
        //
        // USBD will automatically initialize these fields:
        //     bmRequest = 0x80
        //     bRequest  = 0x06
        //
        // We must inititialize these fields:
        //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
        //     wIndex    = Zero (or Language ID for String Descriptors)
        //     wLength   = Length of descriptor buffer
        //
        configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
            | DescriptorIndex;

        configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

        // Now issue the get descriptor request.
        //
        success = DeviceIoControl(hHubDevice,
            IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
            configDescReq,
            nBytes,
            configDescReq,
            nBytes,
            &nBytesReturned,
            NULL);

        if(!success)
        {
            free(configDescReq);
            return NULL;
        }

        if(nBytes != nBytesReturned)
        {
            free(configDescReq);
            return NULL;
        }

        if(configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
        {
            free(configDescReq);
            return NULL;
        }

        return configDescReq;
    }
    bool EnumerateHub(const char* HubName,
        HubDescriptor& hub,
        PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo,
        PUSB_DESCRIPTOR_REQUEST ConfigDesc,
        PSTRING_DESCRIPTOR_NODE StringDescs,
        PCSTR DeviceDesc)
    {
        LOG_FUNC_NAME();

        USB_NODE_INFORMATION   hubInfo;
        USB_HUB_CAPABILITIES   hubCaps;
        USB_HUB_CAPABILITIES_EX hubCapsEx;
        HANDLE                  hHubDevice;
        BOOL                    success;
        ULONG                   nBytes;
        PVOID                   info;
        CharConverter hubNameCvt;
        CharConverter cvt;
        PTCHAR pHubName = const_cast<char_t*>(hubNameCvt.to_char_t(HubName));

        if(DeviceDesc)
        {
            hub.Name = DeviceDesc;
        }
        else
        {
            hub.Name = HubName;
        }
        LOGGER(_log << "Hub.Name: " << hub.Name << endl);

        hHubDevice = INVALID_HANDLE_VALUE;
        hub.IsExternalHub = (ConnectionInfo != NULL);
        LOGGER(_log << "IsRootHub: " << (hub.IsExternalHub ? "Yes" : "No") << endl);
        // Create the full hub device name
        //
        hub.Path = "\\\\.\\";
        hub.Path += HubName;
        // Try to hub the open device
        //
        LOGGER(_log << "Hub.Path: " << hub.Path << endl);
        hHubDevice = CreateFile(cvt.to_char_t(hub.Path.c_str()),
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if(hHubDevice == INVALID_HANDLE_VALUE)
        {
            LOGGER(_log.WriteLine("Hub Handle is INVALID_HANDLE_VALUE"));
            return false;
        }

        // USB_HUB_CAPABILITIES_EX is only available in Vista and later headers
#if (_WIN32_WINNT >= 0x0600) 

        //
        // Now query USBHUB for the USB_HUB_CAPABILTIES_EX structure for this hub.
        //
        success = DeviceIoControl(hHubDevice,
            IOCTL_USB_GET_HUB_CAPABILITIES_EX,
            &hubCapsEx,
            sizeof(USB_HUB_CAPABILITIES_EX),
            &hubCapsEx,
            sizeof(USB_HUB_CAPABILITIES_EX),
            &nBytes,
            NULL);

        if(success)
        {
            PUSB_HUB_CAP_FLAGS HubCapFlags = ctype_cast(PUSB_HUB_CAP_FLAGS)&(hubCapsEx.CapabilityFlags);
            LOGGER(_log << "Extended Hub Capability Flags: " << _hex_num(HubCapFlags->ul) << endl;
            _log << "High speed Capable: " << (HubCapFlags->HubIsHighSpeedCapable ? "Yes" : "No") << endl;
            _log << "High speed: " << (HubCapFlags->HubIsHighSpeed ? "Yes" : "No") << endl;
            _log << "Mulit-transaction Capable: " << (HubCapFlags->HubIsMultiTtCapable ? "Yes" : "No") << endl;
            _log << "Mulit-transaction ON: " << (HubCapFlags->HubIsMultiTt ? "Yes" : "No") << endl;
            _log << "Root hub: " << (HubCapFlags->HubIsRoot ? "Yes" : "No") << endl;
            _log << "Armed for wake on connect: " << (HubCapFlags->HubIsArmedWakeOnConnect ? "Yes" : "No") << endl;
            _log << "Reserved (26 bits): " << _hex_num(HubCapFlags->ReservedMBZ) << endl);

            hub.CapabilityFlags = HubCapFlags->ul;
            hub.IsHighSpeedCapable = HubCapFlags->HubIsHighSpeedCapable;
            hub.IsHighSpeed = HubCapFlags->HubIsHighSpeed;
            hub.IsMulitTransCapable = HubCapFlags->HubIsMultiTtCapable;
            hub.IsMulitTransON = HubCapFlags->HubIsMultiTt;
            hub.IsExternalHub = (HubCapFlags->HubIsRoot == FALSE);
            hub.IsArmedWakeOnConnect = HubCapFlags->HubIsArmedWakeOnConnect;
            hub.ReservedMBZ = HubCapFlags->ReservedMBZ;
        }
#endif
        //
        // Now query USBHUB for the USB_HUB_CAPABILTIES structure for this hub.
        //
        success = DeviceIoControl(hHubDevice,
            IOCTL_USB_GET_HUB_CAPABILITIES,
            &hubCaps,
            sizeof(USB_HUB_CAPABILITIES),
            &hubCaps,
            sizeof(USB_HUB_CAPABILITIES),
            &nBytes,
            NULL);
        
        if(success)
        {
            LOGGER(_log << "Hub Capabilities: " << _hex_num(hubCaps.HubIs2xCapable) << ' '
                << (hubCaps.HubIs2xCapable ? "High speed" : "Not high speed") << endl);
            hub.Is2xCapable = hubCaps.HubIs2xCapable;
        }
        //
        // Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
        // This will tell us the number of downstream ports to enumerate, among
        // other things.
        //
        success = DeviceIoControl(hHubDevice,
            IOCTL_USB_GET_NODE_INFORMATION,
            &hubInfo,
            sizeof(USB_NODE_INFORMATION),
            &hubInfo,
            sizeof(USB_NODE_INFORMATION),
            &nBytes,
            NULL);

        if(!success)
        {
            LOGGER(_log.WriteLine("IOCTL_USB_GET_NODE_INFORMATION Failed"));
            CloseHandle(hHubDevice);
            return false;
        }

        hub.IsBusPowered = Tobool(hubInfo.u.HubInformation.HubIsBusPowered);
        hub.NumberOfPoints = hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts;

        LOGGER(_log << "Hub Power: " << (hub.IsBusPowered ? "Bus Power" : "Self Power") << endl;
        _log << "Number of Ports: " << static_cast<int>(hub.NumberOfPoints) << endl);

        USHORT wHubChar = hubInfo.u.HubInformation.HubDescriptor.wHubCharacteristics;
        switch(wHubChar & 0x0003)
        {
        case 0x0000:
            hub.PowerSwitching = "Ganged";
            break;
        case 0x0001:
            hub.PowerSwitching = "Individual";
            break;
        case 0x0002:
        case 0x0003:
            hub.PowerSwitching = "None";
            break;
        }
        LOGGER(_log << "Power switching: " << hub.PowerSwitching << endl);

        switch(wHubChar & 0x0004)
        {
        case 0x0000:
            hub.IsCompoundDevice = false;
            break;
        case 0x0004:
            hub.IsCompoundDevice = true;
            break;
        }
        LOGGER(_log << "Compound device: " << (hub.IsCompoundDevice ? "Yes" : "No") << endl);

        switch(wHubChar & 0x0018)
        {
        case 0x0000:
            hub.Protection = "Global";
            break;
        case 0x0008:
            hub.Protection = "Individual";
            break;
        case 0x0010:
        case 0x0018:
            hub.Protection = "None";
            break;
        }
        LOGGER(_log << "Over-current Protection: " << hub.Protection << endl);

        // Now recursively enumrate the ports of this hub.
        //
        EnumerateHubPorts(hHubDevice, hub, hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts);
        CloseHandle(hHubDevice);
        return true;
    }
    PSTRING_DESCRIPTOR_NODE GetStringDescriptor(
        HANDLE  hHubDevice,
        ULONG   ConnectionIndex,
        UCHAR   DescriptorIndex,
        USHORT  LanguageID)
    {
        BOOL    success;
        ULONG   nBytes;
        ULONG   nBytesReturned;

        UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
            MAXIMUM_USB_STRING_LENGTH];

        PUSB_DESCRIPTOR_REQUEST stringDescReq;
        PUSB_STRING_DESCRIPTOR  stringDesc;
        PSTRING_DESCRIPTOR_NODE stringDescNode;

        nBytes = sizeof(stringDescReqBuf);

        stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
        stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq + 1);

        // Zero fill the entire request structure
        //
        memset(stringDescReq, 0, nBytes);

        // Indicate the port from which the descriptor will be requested
        //
        stringDescReq->ConnectionIndex = ConnectionIndex;

        //
        // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
        // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
        //
        // USBD will automatically initialize these fields:
        //     bmRequest = 0x80
        //     bRequest  = 0x06
        //
        // We must inititialize these fields:
        //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
        //     wIndex    = Zero (or Language ID for String Descriptors)
        //     wLength   = Length of descriptor buffer
        //
        stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8) | DescriptorIndex;
        stringDescReq->SetupPacket.wIndex = LanguageID;
        stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

        // Now issue the get descriptor request.
        //
        success = DeviceIoControl(hHubDevice,
            IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
            stringDescReq,
            nBytes,
            stringDescReq,
            nBytes,
            &nBytesReturned,
            NULL);

        //
        // Do some sanity checks on the return from the get descriptor request.
        //

        if(!success)
        {
            return NULL;
        }

        if(nBytesReturned < 2)
        {
            return NULL;
        }

        if(stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
        {
            return NULL;
        }

        if(stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
        {
            return NULL;
        }

        if(stringDesc->bLength % 2 != 0)
        {
            return NULL;
        }

        //
        // Looks good, allocate some (zero filled) space for the string descriptor
        // node and copy the string descriptor to it.
        //

        stringDescNode = (PSTRING_DESCRIPTOR_NODE)malloc(sizeof(STRING_DESCRIPTOR_NODE) +
            stringDesc->bLength);

        if(stringDescNode == NULL)
        {
            return NULL;
        }

        stringDescNode->DescriptorIndex = DescriptorIndex;
        stringDescNode->LanguageID = LanguageID;

        memcpy(stringDescNode->StringDescriptor, stringDesc, stringDesc->bLength);
        return stringDescNode;
    }
    BOOL AreThereStringDescriptors(PUSB_DEVICE_DESCRIPTOR DeviceDesc,
        PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc)
    {
        PUCHAR                  descEnd;
        PUSB_COMMON_DESCRIPTOR  commonDesc;

        //
        // Check Device Descriptor strings
        //

        if(DeviceDesc->iManufacturer ||
            DeviceDesc->iProduct ||
            DeviceDesc->iSerialNumber
            )
        {
            return TRUE;
        }

        //
        // Check the Configuration and Interface Descriptor strings
        //
        descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

        commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;
        /*
        while((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
            (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
        {
            switch(commonDesc->bDescriptorType)
            {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if(commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    break;
                }
                if(((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    return TRUE;
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if(commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    break;
                }
                if(((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    return TRUE;
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            default:
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;
            }
            break;
        }*/

        return FALSE;
    }
    PSTRING_DESCRIPTOR_NODE GetStringDescriptors(
        HANDLE  hHubDevice,
        ULONG   ConnectionIndex,
        UCHAR   DescriptorIndex,
        ULONG   NumLanguageIDs,
        USHORT  *LanguageIDs,
        PSTRING_DESCRIPTOR_NODE StringDescNodeTail
        )
    {
        ULONG i;

        for(i = 0; i < NumLanguageIDs; i++)
        {
            StringDescNodeTail->Next = GetStringDescriptor(hHubDevice,
                ConnectionIndex,
                DescriptorIndex,
                *LanguageIDs);

            if(StringDescNodeTail->Next)
            {
                StringDescNodeTail = StringDescNodeTail->Next;
            }

            LanguageIDs++;
        }

        return StringDescNodeTail;
    }
    PSTRING_DESCRIPTOR_NODE GetAllStringDescriptors(
        HANDLE hHubDevice,
        ULONG ConnectionIndex,
        PUSB_DEVICE_DESCRIPTOR DeviceDesc,
        PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc)
    {
        PSTRING_DESCRIPTOR_NODE supportedLanguagesString;
        PSTRING_DESCRIPTOR_NODE stringDescNodeTail;
        ULONG                   numLanguageIDs;
        USHORT                  *languageIDs;

        PUCHAR                  descEnd;
        PUSB_COMMON_DESCRIPTOR  commonDesc;

        //
        // Get the array of supported Language IDs, which is returned
        // in String Descriptor 0
        //
        supportedLanguagesString = GetStringDescriptor(hHubDevice,
            ConnectionIndex,
            0,
            0);

        if(supportedLanguagesString == NULL)
        {
            return NULL;
        }

        numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;

        languageIDs = ctype_cast(USHORT*)(&supportedLanguagesString->StringDescriptor->bString[0]);

        stringDescNodeTail = supportedLanguagesString;

        //
        // Get the Device Descriptor strings
        //

        if(DeviceDesc->iManufacturer)
        {
            stringDescNodeTail = GetStringDescriptors(hHubDevice,
                ConnectionIndex,
                DeviceDesc->iManufacturer,
                numLanguageIDs,
                languageIDs,
                stringDescNodeTail);
        }

        if(DeviceDesc->iProduct)
        {
            stringDescNodeTail = GetStringDescriptors(hHubDevice,
                ConnectionIndex,
                DeviceDesc->iProduct,
                numLanguageIDs,
                languageIDs,
                stringDescNodeTail);
        }

        if(DeviceDesc->iSerialNumber)
        {
            stringDescNodeTail = GetStringDescriptors(hHubDevice,
                ConnectionIndex,
                DeviceDesc->iSerialNumber,
                numLanguageIDs,
                languageIDs,
                stringDescNodeTail);
        }


        //
        // Get the Configuration and Interface Descriptor strings
        //

        descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

        commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;
        /*
        while((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
            (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
        {
            switch(commonDesc->bDescriptorType)
            {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if(commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    break;
                }
                if(((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    stringDescNodeTail = GetStringDescriptors(
                        hHubDevice,
                        ConnectionIndex,
                        ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
                        numLanguageIDs,
                        languageIDs,
                        stringDescNodeTail);
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if(commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    break;
                }
                if(((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    stringDescNodeTail = GetStringDescriptors(
                        hHubDevice,
                        ConnectionIndex,
                        ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface,
                        numLanguageIDs,
                        languageIDs,
                        stringDescNodeTail);
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            default:
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;
            }
            break;
        }
        */

        return supportedLanguagesString;
    }
    void EnumerateHubPorts(HANDLE hHubDevice, HubDescriptor& hub, ULONG NumPorts)
    {
        LOG_FUNC_NAME();

        ULONG       index;
        BOOL        success;

        PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;
        PUSB_DESCRIPTOR_REQUEST             configDesc;
        PSTRING_DESCRIPTOR_NODE             stringDescs;
        PUSBDEVICEINFO                      info;

        string driverKeyName;
        string deviceDesc;

        // Loop over all ports of the hub.
        //
        // Port indices are 1 based, not 0 based.
        //
        for(index = 1; index <= NumPorts; index++)
        {
            ULONG nBytesEx;

            // Allocate space to hold the connection info for this port.
            // For now, allocate it big enough to hold info for 30 pipes.
            //
            // Endpoint numbers are 0-15.  Endpoint number 0 is the standard
            // control endpoint which is not explicitly listed in the Configuration
            // Descriptor.  There can be an IN endpoint and an OUT endpoint at
            // endpoint numbers 1-15 so there can be a maximum of 30 endpoints
            // per device configuration.
            //
            // Should probably size this dynamically at some point.
            //
            nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) +
                sizeof(USB_PIPE_INFO) * 30;

            connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)malloc(nBytesEx);

            if(connectionInfoEx == NULL)
            {
                break;
            }

            //
            // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION_EX structure
            // for this port.  This will tell us if a device is attached to this
            // port, among other things.
            //
            connectionInfoEx->ConnectionIndex = index;

            success = DeviceIoControl(hHubDevice,
                IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
                connectionInfoEx,
                nBytesEx,
                connectionInfoEx,
                nBytesEx,
                &nBytesEx,
                NULL);

            if(!success)
            {
                PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
                ULONG                               nBytes;

                // Try using IOCTL_USB_GET_NODE_CONNECTION_INFORMATION
                // instead of IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
                //
                nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
                    sizeof(USB_PIPE_INFO) * 30;

                connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)malloc(nBytes);
                connectionInfo->ConnectionIndex = index;

                success = DeviceIoControl(hHubDevice,
                    IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                    connectionInfo,
                    nBytes,
                    connectionInfo,
                    nBytes,
                    &nBytes,
                    NULL);

                if(!success)
                {
                    free(connectionInfo);
                    free(connectionInfoEx);
                    continue;
                }

                // Copy IOCTL_USB_GET_NODE_CONNECTION_INFORMATION into
                // IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX structure.
                //
                connectionInfoEx->ConnectionIndex =
                    connectionInfo->ConnectionIndex;

                connectionInfoEx->DeviceDescriptor =
                    connectionInfo->DeviceDescriptor;

                connectionInfoEx->CurrentConfigurationValue =
                    connectionInfo->CurrentConfigurationValue;

                connectionInfoEx->Speed =
                    connectionInfo->LowSpeed ? UsbLowSpeed : UsbFullSpeed;

                connectionInfoEx->DeviceIsHub =
                    connectionInfo->DeviceIsHub;

                connectionInfoEx->DeviceAddress =
                    connectionInfo->DeviceAddress;

                connectionInfoEx->NumberOfOpenPipes =
                    connectionInfo->NumberOfOpenPipes;

                connectionInfoEx->ConnectionStatus =
                    connectionInfo->ConnectionStatus;

                memcpy(&connectionInfoEx->PipeList[0],
                    &connectionInfo->PipeList[0],
                    sizeof(USB_PIPE_INFO) * 30);

                free(connectionInfo);
            }

            // Update the count of connected devices
            //
            if(connectionInfoEx->ConnectionStatus == DeviceConnected)
            {
                //TotalDevicesConnected++;
            }

            if(connectionInfoEx->DeviceIsHub)
            {
                //TotalHubs++;
            }

            // If there is a device connected, get the Device Description
            //
            deviceDesc = "";
            if(connectionInfoEx->ConnectionStatus != NoDeviceConnected)
            {
                if(GetDriverKeyName(hHubDevice, index, driverKeyName))
                {
                    DriverNameToDeviceDesc(driverKeyName.c_str(), deviceDesc, FALSE);
                }
            }

            // If there is a device connected to the port, try to retrieve the
            // Configuration Descriptor from the device.
            //
            if(connectionInfoEx->ConnectionStatus == DeviceConnected)
            {
                configDesc = GetConfigDescriptor(hHubDevice, index, 0);
            }
            else
            {
                configDesc = NULL;
            }

            if(configDesc != NULL &&
                AreThereStringDescriptors(&connectionInfoEx->DeviceDescriptor,
                (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc + 1)))
            {
                stringDescs = GetAllStringDescriptors(
                    hHubDevice,
                    index,
                    &connectionInfoEx->DeviceDescriptor,
                    (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc + 1));
            }
            else
            {
                stringDescs = NULL;
            }

            // If the device connected to the port is an external hub, get the
            // name of the external hub and recursively enumerate it.
            //
            if(connectionInfoEx->DeviceIsHub)
            {
                string extHubName;
                if(GetExternalHubName(hHubDevice, index, extHubName))
                {
                    hub.ExternalHub.push_back(HubDescriptor());
                    hub.ExternalHub.back().Name = extHubName;

                    if(EnumerateHub(extHubName.c_str(),
                        hub.ExternalHub.back(),
                        connectionInfoEx,
                        configDesc,
                        stringDescs,
                        deviceDesc.c_str()) == FALSE)
                    {
                        free(connectionInfoEx);
                        if(configDesc)
                        {
                            free(configDesc);
                        }

                        if(stringDescs != NULL)
                        {
                            PSTRING_DESCRIPTOR_NODE Next;

                            do
                            {
                                Next = stringDescs->Next;
                                free(stringDescs);
                                stringDescs = Next;

                            } while(stringDescs != NULL);
                        }
                    }
                }
            }
            else
            {
                // Allocate some space for a USBDEVICEINFO structure to hold the
                // Config Descriptors, Strings Descriptors, and connection info
                // pointers.  GPTR zero initializes the structure for us.
                //
                info = (PUSBDEVICEINFO)malloc(sizeof(USBDEVICEINFO));

                if(info == NULL)
                {
                    if(configDesc != NULL)
                    {
                        free(configDesc);
                    }
                    free(connectionInfoEx);
                    break;
                }

                info->DeviceInfoType = DeviceInfo;
                info->ConnectionInfo = connectionInfoEx;
                info->ConfigDesc = configDesc;
                info->StringDescs = stringDescs;

                if(connectionInfoEx->ConnectionStatus == NoDeviceConnected)
                {
                    //icon = NoDeviceIcon;
                }
                else if(connectionInfoEx->CurrentConfigurationValue)
                {
                    //icon = GoodDeviceIcon;
                }
                else
                {
                    //icon = BadDeviceIcon;
                }
            }
        }
    }
    bool EnumerateHostController(HANDLE hHCDev, BusDescriptor& bus)
    {
        LOG_FUNC_NAME();
        LOGGER(_log.WriteLine("GetHCDDriverKeyName..."));
        if(!GetHCDDriverKeyName(hHCDev, bus.DriverKey))
            return _logRetValue(false);

        string deviceId;
        CharConverter cvt;
        LOGGER(_log.WriteLine("DriverNameToDeviceDesc..."));
        if(!DriverNameToDeviceDesc(bus.DriverKey.c_str(), deviceId, true))
            return _logRetValue(false);

        LOGGER(_log.WriteLine("UnFormat Hub ID..."));
        ULONG ven, dev, subsys, rev;
        if(_stscanf_s(cvt.to_char_t(deviceId.c_str()),
            _T("PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x"),
            &ven, &dev, &subsys, &rev) != 4)
        {
            return _logRetValue(false);
        }
        bus.VendorID = ven;
        bus.DeviceID = dev;
        bus.SubSysID = subsys;
        bus.Revision = rev;

        LOGGER(_log.WriteLine("DriverNameToDeviceDesc..."));
        if(!DriverNameToDeviceDesc(bus.DriverKey.c_str(), bus.Name, FALSE))
            return _logRetValue(false);

        LOGGER(_log.WriteLine("GetRootHubName..."));
        if(!GetRootHubName(hHCDev, bus.RootHub.DeviceKey))
        {
            LOGGER(_log.WriteLine("GetRootHubName Failed..."));
            return _logRetValue(true);
        }
        EnumerateHub(bus.RootHub.DeviceKey.c_str(), bus.RootHub, NULL, NULL, NULL, "RootHub");
        return _logRetValue(true);
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

                EnumerateHostController(hHCDev, buslist.back());
                CloseHandle(hHCDev);
            }
        }

        HDEVINFO deviceInfo = SetupDiGetClassDevs((LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
            NULL, NULL,
            (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
        if(deviceInfo == INVALID_HANDLE_VALUE)
            return 0;

        return 0;
    }
    size_t EnumUsbName(list<string>& devlist)
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
};
//--------------------------------------------------------- 
} // namesapce env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 
