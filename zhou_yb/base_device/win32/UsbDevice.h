//========================================================= 
/**@file UsbDevice.h
 * @brief Windows下 USB设备通信实现 
 * 
 * @date 2015-10-09   16:57:29
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
// 忽略对结构体中数组大小为0的警告
#pragma warning(disable:4200) 
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
typedef struct _USB_HID_DESCRIPTOR
{
    UCHAR   bLength;
    UCHAR   bDescriptorType;
    USHORT  bcdHID;
    UCHAR   bCountryCode;
    UCHAR   bNumDescriptors;
    struct
    {
        UCHAR   bDescriptorType;
        USHORT  wDescriptorLength;
    } OptionalDescriptors[1];
} USB_HID_DESCRIPTOR, *PUSB_HID_DESCRIPTOR;
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
typedef struct _USB_ENDPOINT_DESCRIPTOR2
{
    UCHAR  bLength;             // offset 0, size 1
    UCHAR  bDescriptorType;     // offset 1, size 1
    UCHAR  bEndpointAddress;    // offset 2, size 1
    UCHAR  bmAttributes;        // offset 3, size 1
    USHORT wMaxPacketSize;      // offset 4, size 2
    USHORT wInterval;           // offset 6, size 2
    UCHAR  bSyncAddress;        // offset 8, size 1
} USB_ENDPOINT_DESCRIPTOR2, *PUSB_ENDPOINT_DESCRIPTOR2;
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
/// USB设备数据传输方式
enum UsbTransferType
{
    /// 未知的传输方式
    UnknownTransferType = 0,
    /// 控制传输
    ControlTransferType,
    /// 同步传输
    IsochronousTransferType,
    /// 块传输
    BulkTransferType,
    /// 中断传输
    InterruptTransferType
};
/// 字符串描述符号
struct StringDescriptor
{
    uint DescriptorIndex;
    uint LanguageID;
    ByteBuilder Description;
};
/// USB端点描述符
struct EndpointDescriptor
{
    bool IsInputEndpoint;
    bool IsAsyncEndpoint;
    byte bEndpointAddress;
    UsbTransferType TransferType;
    ushort wMaxPacketSize;
    byte bInterval;
    byte bSyncAddress;
    ushort wInterval;
};
/// USB接口描述符
struct InterfaceDescriptor
{
    byte bInterfaceNumber;
    byte bAlternateSetting;
    byte bNumEndpoints;
    byte bInterfaceClass;
    byte bInterfaceSubClass;
    byte bInterfaceProtocol;
    byte iInterface;
    string Interface;
    ushort wNumClasses;
};
/// USB配置描述符
struct ConfigurationDescriptor
{
    ushort wTotalLength;
    byte bNumInterfaces;
    byte bConfigurationValue;
    byte iConfiguration;
    string Configuration;
    byte bmAttributes;
    string PowerMode;
    uint MaxPower_mA;
    list<InterfaceDescriptor> Interfaces;
};
/// 未知的描述信息
struct UnknownDescriptor
{
    byte bDescriptorType;
    ByteBuilder Description;
};
/// HID可选描述
struct UsbHidOptionalDescriptor
{
    byte bDescriptorType;
    ushort wDescriptorLength;
};
/// HID设备信息
struct UsbHidDescriptor
{
    ushort bcdHID;
    byte bCountryCode;
    uint bNumDescriptors;
    list<UsbHidOptionalDescriptor> Descriptors;
};
/// USB设备信息
struct UsbDescriptor
{
    string Name;
    string Path;
    uint ConnectionIndex;
    ushort bcdUSB;
    byte bDeviceClass;
    byte bDeviceSubClass;
    byte bDeviceProtocol;
    byte bMaxPacketSize0;
    ushort Vid;
    ushort Pid;
    ushort bcdDevice;
    byte iManufacturer;
    string Manufacturer;
    byte iSerialNumber;
    string SerialNumber;
    byte iProduct;
    byte bNumConfigurations;
    string ConnectionStatus;
    byte CurrentConfigurationValue;
    string Speed;
    uint DeviceAddress;
    uint NumberOfOpenPipes;
    list<ConfigurationDescriptor> Configurations;
    list<InterfaceDescriptor> Interfaces;
    list<EndpointDescriptor> Pipes;
    list<StringDescriptor> Descriptions;
    list<UnknownDescriptor> UnknownDescriptions;
};
/// USB Hub信息
struct HubDescriptor
{
    string DeviceKey;
    string Name;
    string Path;
    uint ConnectionIndex;
    bool IsRootHub;
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
    /// 作为扩展Hub设备本身的信息,RootHub不具有
    list<UsbDescriptor> Description;
    /// 挂载的扩展HUB
    list<HubDescriptor> ExternalHub;
    /// 挂载的USB设备(HUB本身也是一种设备)
    list<UsbDescriptor> ExteranlUsb;
};
/// 系统总线信息
struct BusDescriptor
{
    string DriverKey;
    string Name;
    ushort VendorID;
    ushort DeviceID;
    uint SubSysID;
    uint Revision;
    HubDescriptor RootHub;
};
//--------------------------------------------------------- 
/// USB设备
class UsbDevice : public DeviceBehavior
{
public:
    //----------------------------------------------------- 
    /// 设备信息定义
    typedef UsbDescriptor device_info;
    /// 输出USB设备信息
    static void PrintUsbTree(const list<BusDescriptor>& bus, LoggerAdapter& logger)
    {
        list<BusDescriptor>::const_iterator itr;
        for(itr = bus.begin();itr != bus.end(); ++itr)
        {
            logger.WriteLine();
            _PrintBusDescriptor(*itr, logger);
        }
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 输出Bus信息
    static void _PrintBusDescriptor(const BusDescriptor& bus, LoggerAdapter& logger)
    {
        logger << "Bus Descriptor:" << endl;
        logger << "Bus.Name:      " << bus.Name << endl;
        logger << "Bus.DriverKey: " << bus.DriverKey << endl;
        logger << "VendorID: " << _hex(bus.VendorID) << endl;
        logger << "DeviceID: " << _hex(bus.DeviceID) << endl;
        logger << "SubSysID: " << _hex(bus.SubSysID) << endl;
        logger << "Revision: " << bus.Revision << endl;

        logger.WriteLine();
        _PrintHubDescriptor(bus.RootHub, logger);
    }
    /// 输出Usb Hub信息
    static void _PrintHubDescriptor(const HubDescriptor& hub, LoggerAdapter& logger)
    {
        logger << "Hub Descriptor:" << endl;
        logger << "Hub.Name: " << hub.Name << endl;
        logger << "Hub.Path: " << hub.Path << endl;
        logger << "ConnectionIndex:              " << hub.ConnectionIndex << endl;
        logger << "IsRootHub:                    " << (hub.IsRootHub ? "Yes" : "No") << endl;

        logger << "Hub Capabilities:             " << _hex_num(hub.Is2xCapable) << ' '
            << (hub.Is2xCapable ? "(High speed)" : "(Not high speed)") << endl;

        logger << "Extended Hub Capability Flags:" << _hex_num(hub.CapabilityFlags) << endl;
        logger << "High speed Capable:           " << (hub.IsHighSpeedCapable ? "Yes" : "No") << endl;
        logger << "High speed:                   " << (hub.IsHighSpeed ? "Yes" : "No") << endl;
        logger << "Mulit-transaction Capable:    " << (hub.IsMulitTransCapable ? "Yes" : "No") << endl;
        logger << "Mulit-transaction ON:         " << (hub.IsMulitTransON ? "Yes" : "No") << endl;
        logger << "Root hub:                     " << (hub.IsRootHub ? "Yes" : "No") << endl;
        logger << "Armed for wake on connect:    " << (hub.IsArmedWakeOnConnect ? "Yes" : "No") << endl;
        logger << "Reserved (26 bits):           " << _hex_num(hub.ReservedMBZ) << endl;

        logger << "Hub Power:                    " << (hub.IsBusPowered ? "Bus Power" : "Self Power") << endl;
        logger << "Number of Ports:              " << static_cast<int>(hub.NumberOfPoints) << endl;
        logger << "Power switching:              " << hub.PowerSwitching << endl;
        logger << "Compound device:              " << (hub.IsCompoundDevice ? "Yes" : "No") << endl;
        logger << "Over-current Protection:      " << hub.Protection << endl;
        logger << "Number of ExternalHubs:       " << hub.ExternalHub.size() << endl;
        logger << "Number of ExteranlUsbs:       " << hub.ExteranlUsb.size() << endl;

        list<HubDescriptor>::const_iterator hubItr;
        for(hubItr = hub.ExternalHub.begin();hubItr != hub.ExternalHub.end(); ++hubItr)
        {
            logger.WriteLine();
            if(hubItr->Description.size() > 0)
            {
                _PrintUsbDescriptor(hubItr->Description.front(), logger);
                logger.WriteLine();
            }
            _PrintHubDescriptor(*hubItr, logger);
        }

        list<UsbDescriptor>::const_iterator usbItr;
        for(usbItr = hub.ExteranlUsb.begin();usbItr != hub.ExteranlUsb.end(); ++usbItr)
        {
            logger.WriteLine();
            _PrintUsbDescriptor(*usbItr, logger);
        }
    }
    /// 输出USB设备信息
    static void _PrintUsbDescriptor(const UsbDescriptor& usb, LoggerAdapter& logger)
    {
        logger << "Device Descriptor:" << endl;
        logger << "Usb.Name: " << usb.Name << endl;
        logger << "Usb.Path: " << usb.Path << endl;
        logger << "ConnectionIndex:     " << usb.ConnectionIndex << endl;
        logger << "bcdUSB:              " << _hex_num(usb.bcdUSB) << endl;
        logger << "bDeviceClass:        " << _hex_num(usb.bDeviceClass) << endl;
        logger << "bDeviceSubClass:     " << _hex_num(usb.bDeviceSubClass) << endl;
        logger << "bDeviceProtocol:     " << _hex_num(usb.bDeviceProtocol) << endl;
        logger << "bMaxPacketSize0:     " << _hex_num(usb.bMaxPacketSize0) 
            << " (" << static_cast<uint>(usb.bMaxPacketSize0) << ')' << endl;
        logger << "idVendor:            " << _hex_num(usb.Vid) << endl;
        logger << "idProduct:           " << _hex_num(usb.Pid) << endl;
        logger << "bcdDevice:           " << _hex_num(usb.bcdDevice) << endl;
        logger << "iManufacturer:       " << _hex_num(usb.iManufacturer) << endl;
        logger << "iProduct:            " << _hex_num(usb.iProduct) << endl;
        logger << "iSerialNumber:       " << _hex_num(usb.iSerialNumber) << endl;
        logger << "bNumConfigurations:  " << _hex_num(usb.bNumConfigurations) << endl;
        logger << "ConnectionStatus:    " << usb.ConnectionStatus << endl;
        logger << "Current Config Value:" << _hex_num(usb.CurrentConfigurationValue) << endl;
        logger << "Device Bus Speed:    " << usb.Speed << endl;
        logger << "Device Address:      " << _hex_num(usb.DeviceAddress) << endl;
        logger << "Number of Pipes:     " << usb.Pipes.size() << endl;
        logger << "Open Pipes:          " << usb.NumberOfOpenPipes << endl;

        list<ConfigurationDescriptor>::const_iterator cfgItr;
        for(cfgItr = usb.Configurations.begin();cfgItr != usb.Configurations.end(); ++cfgItr)
        {
            logger.WriteLine();
            _PrintConfigurationDescriptor(*cfgItr, logger);
        }

        list<InterfaceDescriptor>::const_iterator interfaceItr;
        for(interfaceItr = usb.Interfaces.begin();interfaceItr != usb.Interfaces.end(); ++interfaceItr)
        {
            logger.WriteLine();
            _PrintInterfaceDescriptor(*interfaceItr, logger);
        }

        list<EndpointDescriptor>::const_iterator epItr;
        for(epItr = usb.Pipes.begin();epItr != usb.Pipes.end(); ++epItr)
        {
            logger.WriteLine();
            _PrintEndpointDescriptor(*epItr, logger);
        }
        list<UnknownDescriptor>::const_iterator unknownItr;
        for(unknownItr = usb.UnknownDescriptions.begin();unknownItr != usb.UnknownDescriptions.end(); ++unknownItr)
        {
            logger.WriteLine();
            logger << "Unknown Descriptor:" << endl;
            logger << "bDescriptorType:   " << _hex_num(unknownItr->bDescriptorType) << endl;
            logger << "bLength:           " << unknownItr->Description.GetLength() << endl;
            logger.WriteStream(unknownItr->Description) << endl;
        }
    }
    /// 输出USB配置信息
    static void _PrintConfigurationDescriptor(const ConfigurationDescriptor& cfg, LoggerAdapter& logger)
    {
        logger << "Configuration Descriptor:" << endl;
        logger << "wTotalLength:        " << _hex_num(cfg.wTotalLength) << endl;
        logger << "bNumInterfaces:      " << _hex_num(cfg.bNumInterfaces) << endl;
        logger << "bConfigurationValue: " << _hex_num(cfg.bConfigurationValue) << endl;
        logger << "iConfiguration:      " << _hex_num(cfg.iConfiguration) << " (" << cfg.Configuration << ')' << endl;
        logger << "bmAttributes:        " << _hex_num(cfg.bmAttributes) << " (" << cfg.PowerMode << ')' << endl;
        logger << "MaxPower:            " << cfg.MaxPower_mA << "mA" << endl;
    }
    /// 输出USB接口信息
    static void _PrintInterfaceDescriptor(const InterfaceDescriptor& interfaceDesc, LoggerAdapter& logger)
    {
        logger << "Interface Descriptor:" << endl;
        logger << "bInterfaceNumber:   " << _hex_num(interfaceDesc.bInterfaceNumber) << endl;
        logger << "bAlternateSetting:  " << _hex_num(interfaceDesc.bAlternateSetting) << endl;
        logger << "bNumEndpoints:      " << _hex_num(interfaceDesc.bNumEndpoints) << endl;
        logger << "bInterfaceClass:    " << _hex_num(interfaceDesc.bInterfaceClass) << endl;
        logger << "bInterfaceSubClass: " << _hex_num(interfaceDesc.bInterfaceSubClass) << endl;
        logger << "bInterfaceProtocol: " << _hex_num(interfaceDesc.bInterfaceProtocol) << endl;
        logger << "iInterface:         " << _hex_num(interfaceDesc.iInterface) << endl;
        logger << "wNumClasses:        " << _hex_num(interfaceDesc.wNumClasses) << endl;
    }
    /// 输出USB端点信息
    static void _PrintEndpointDescriptor(const EndpointDescriptor& ep, LoggerAdapter& logger)
    {
        logger << "Endpoint Descriptor: " << endl;
        logger << "bEndpointAddress: " << _hex_num(ep.bEndpointAddress) << (ep.IsInputEndpoint ? "   IN" : "   OUT") << endl;
        logger << "Transfer Type:    ";
        switch(ep.TransferType)
        {
        case ControlTransferType:
            logger << "Control" << endl;
            break;
        case IsochronousTransferType:
            logger << "Isochronous" << endl;
            break;
        case BulkTransferType:
            logger << "Bulk" << endl;
            break;
        case InterruptTransferType:
            logger << "Interrupt" << endl;
            break;
        }
        logger << "wMaxPacketSize:   " << _hex_num(ep.wMaxPacketSize) << " (" << static_cast<uint>(ep.wMaxPacketSize) << ')' << endl;
        if(ep.IsAsyncEndpoint)
        {
            logger << "wInterval:        " << _hex_num(ep.wInterval) << endl;
            logger << "bSyncAddress:     " << _hex_num(ep.bSyncAddress) << endl;
        }
        else
        {
            logger << "bInterval:        " << _hex_num(ep.bInterval) << endl;
        }
    }
    //----------------------------------------------------- 
    bool _DriverNameToDeviceDesc(const char* driverName, string& deviceDesc, BOOL isDeviceId)
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
            if(cr == CR_SUCCESS && strcmp_t(cvt.to_char_t(driverName), buf) == 0)
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
    bool _GetHCDDriverKeyName(HANDLE HCD, string& driverName)
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
    bool _GetDriverKeyName(HANDLE Hub, ULONG ConnectionIndex, string& drvName)
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
    bool _GetRootHubName(HANDLE HostController, string& hubName)
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
    bool _GetExternalHubName(HANDLE Hub, ULONG ConnectionIndex, string& hubName)
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
    void _DisplayStringDescriptor(UCHAR Index, PSTRING_DESCRIPTOR_NODE StringDescs, string& desc)
    {
        CharConverter cvt;
        int descChars;
        while(StringDescs)
        {
            if(StringDescs->DescriptorIndex == Index)
            {
                //
                // bString from USB_STRING_DESCRIPTOR isn't NULL-terminated, so 
                // calculate the number of characters.  
                // 
                // bLength is the length of the whole structure, not just the string.  
                // 
                // bLength is bytes, bString is WCHARs
                // 
                descChars = ((int)StringDescs->StringDescriptor->bLength -
                    offsetof(USB_STRING_DESCRIPTOR, bString)) /
                    sizeof(WCHAR);
                //
                // Use the * precision and pass the number of characters just caculated.
                // bString is always WCHAR so specify widestring regardless of what TCHAR resolves to
                // 
                const char* str = cvt.to_char(StringDescs->StringDescriptor->bString, descChars);
                desc = str;
                break;
            }

            StringDescs = StringDescs->Next;
        }
    }
    PUSB_DESCRIPTOR_REQUEST _GetConfigDescriptor(
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

        configDescReq = (PUSB_DESCRIPTOR_REQUEST)malloc_alloc::allocate(nBytes);
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

        if(!success || nBytes != nBytesReturned)
        {
            malloc_alloc::deallocate(configDescReq);
            return NULL;
        }

        if(configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
        {
            malloc_alloc::deallocate(configDescReq);
            return NULL;
        }

        return configDescReq;
    }
    PSTRING_DESCRIPTOR_NODE _GetStringDescriptor(
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

        stringDescNode = (PSTRING_DESCRIPTOR_NODE)malloc_alloc::allocate(sizeof(STRING_DESCRIPTOR_NODE) +
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
    BOOL _AreThereStringDescriptors(PUSB_DEVICE_DESCRIPTOR DeviceDesc,
        PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc)
    {
        PUCHAR                  descEnd;
        PUSB_COMMON_DESCRIPTOR  commonDesc;

        //
        // Check Device Descriptor strings
        //

        if(DeviceDesc->iManufacturer ||
            DeviceDesc->iProduct ||
            DeviceDesc->iSerialNumber)
        {
            return TRUE;
        }

        //
        // Check the Configuration and Interface Descriptor strings
        //
        descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

        commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;
        
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
                commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
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
                commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
                continue;

            default:
                commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
                continue;
            }
            break;
        }

        return FALSE;
    }
    PSTRING_DESCRIPTOR_NODE _GetStringDescriptors(
        HANDLE  hHubDevice,
        ULONG   ConnectionIndex,
        UCHAR   DescriptorIndex,
        ULONG   NumLanguageIDs,
        USHORT  *LanguageIDs,
        PSTRING_DESCRIPTOR_NODE StringDescNodeTail)
    {
        ULONG i;

        for(i = 0; i < NumLanguageIDs; i++)
        {
            StringDescNodeTail->Next = _GetStringDescriptor(hHubDevice,
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
    PSTRING_DESCRIPTOR_NODE _GetAllStringDescriptors(
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
        supportedLanguagesString = _GetStringDescriptor(hHubDevice,
            ConnectionIndex,
            0,
            0);

        if(supportedLanguagesString == NULL)
        {
            return NULL;
        }

        numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;
        languageIDs = (USHORT*)(&supportedLanguagesString->StringDescriptor->bString[0]);

        stringDescNodeTail = supportedLanguagesString;

        //
        // Get the Device Descriptor strings
        //

        if(DeviceDesc->iManufacturer)
        {
            stringDescNodeTail = _GetStringDescriptors(hHubDevice,
                ConnectionIndex,
                DeviceDesc->iManufacturer,
                numLanguageIDs,
                languageIDs,
                stringDescNodeTail);
        }

        if(DeviceDesc->iProduct)
        {
            stringDescNodeTail = _GetStringDescriptors(hHubDevice,
                ConnectionIndex,
                DeviceDesc->iProduct,
                numLanguageIDs,
                languageIDs,
                stringDescNodeTail);
        }

        if(DeviceDesc->iSerialNumber)
        {
            stringDescNodeTail = _GetStringDescriptors(hHubDevice,
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
                    stringDescNodeTail = _GetStringDescriptors(
                        hHubDevice,
                        ConnectionIndex,
                        ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
                        numLanguageIDs,
                        languageIDs,
                        stringDescNodeTail);
                }
                commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)(commonDesc)+commonDesc->bLength);
                continue;
            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if(commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    break;
                }
                if(((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    stringDescNodeTail = _GetStringDescriptors(
                        hHubDevice,
                        ConnectionIndex,
                        ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface,
                        numLanguageIDs,
                        languageIDs,
                        stringDescNodeTail);
                }
                commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)(commonDesc)+commonDesc->bLength);
                continue;

            default:
                commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)(commonDesc)+commonDesc->bLength);
                continue;
            }
            break;
        }

        return supportedLanguagesString;
    }
    void _DisplayConfigurationDescriptor(ConfigurationDescriptor& cfgDesc,
        PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
        PSTRING_DESCRIPTOR_NODE         StringDescs)
    {
        cfgDesc.wTotalLength = ConfigDesc->wTotalLength;
        cfgDesc.bNumInterfaces = ConfigDesc->bNumInterfaces;
        cfgDesc.bConfigurationValue = ConfigDesc->bConfigurationValue;
        cfgDesc.iConfiguration = ConfigDesc->iConfiguration;
        if(cfgDesc.iConfiguration) _DisplayStringDescriptor(cfgDesc.iConfiguration, StringDescs, cfgDesc.Configuration);
        cfgDesc.bmAttributes = ConfigDesc->bmAttributes;
        if(ConfigDesc->bmAttributes & 0x80)
        {
            cfgDesc.PowerMode = "Bus Powered";
        }

        if(ConfigDesc->bmAttributes & 0x40)
        {
            cfgDesc.PowerMode = "Self Powered";
        }

        if(ConfigDesc->bmAttributes & 0x20)
        {
            cfgDesc.PowerMode = "Remote Wakeup";
        }
        cfgDesc.MaxPower_mA = ConfigDesc->MaxPower * 2;
    }
    void _DisplayInterfaceDescriptor(InterfaceDescriptor& interfaceDesc,
        PUSB_INTERFACE_DESCRIPTOR   InterfaceDesc,
        PSTRING_DESCRIPTOR_NODE     StringDescs)
    {
        interfaceDesc.bInterfaceNumber = InterfaceDesc->bInterfaceNumber;
        interfaceDesc.bAlternateSetting = InterfaceDesc->bAlternateSetting;
        interfaceDesc.bNumEndpoints = InterfaceDesc->bNumEndpoints;
        interfaceDesc.bInterfaceClass = InterfaceDesc->bInterfaceClass;
        interfaceDesc.bInterfaceSubClass = InterfaceDesc->bInterfaceSubClass;
        interfaceDesc.bInterfaceProtocol = InterfaceDesc->bInterfaceProtocol;
        interfaceDesc.iInterface = InterfaceDesc->iInterface;
        if(interfaceDesc.iInterface) 
            _DisplayStringDescriptor(interfaceDesc.iInterface, StringDescs, interfaceDesc.Interface);
        interfaceDesc.wNumClasses = 0;
        if(InterfaceDesc->bLength == sizeof(USB_INTERFACE_DESCRIPTOR2))
        {
            PUSB_INTERFACE_DESCRIPTOR2 interfaceDesc2;
            interfaceDesc2 = (PUSB_INTERFACE_DESCRIPTOR2)InterfaceDesc;
            interfaceDesc.wNumClasses = interfaceDesc2->wNumClasses;
        }
    }
    void _DisplayEndpointDescriptor(EndpointDescriptor& ep, PUSB_ENDPOINT_DESCRIPTOR EndpointDesc)
    {
        ep.bEndpointAddress = EndpointDesc->bEndpointAddress;
        ep.IsInputEndpoint = Tobool(USB_ENDPOINT_DIRECTION_IN(EndpointDesc->bEndpointAddress));
        switch(EndpointDesc->bmAttributes & 0x03)
        {
        case 0x00:
            ep.TransferType = ControlTransferType;
            break;
        case 0x01:
            ep.TransferType = IsochronousTransferType;
            break;
        case 0x02:
            ep.TransferType = BulkTransferType;
            break;
        case 0x03:
            ep.TransferType = InterruptTransferType;
            break;
        }
        ep.wMaxPacketSize = EndpointDesc->wMaxPacketSize;
        if(EndpointDesc->bLength == sizeof(USB_ENDPOINT_DESCRIPTOR))
        {
            ep.IsAsyncEndpoint = false;
            ep.bInterval = EndpointDesc->bInterval;
            ep.wInterval = 0;
            ep.bSyncAddress = 0;
        }
        else
        {
            PUSB_ENDPOINT_DESCRIPTOR2 endpointDesc2 = (PUSB_ENDPOINT_DESCRIPTOR2)EndpointDesc;
            ep.IsAsyncEndpoint = true;
            ep.bInterval = 0;
            ep.wInterval = endpointDesc2->wInterval;
            ep.bSyncAddress = endpointDesc2->bSyncAddress;
        }
    }
    void _DisplayHidDescriptor(UsbHidDescriptor& hid, PUSB_HID_DESCRIPTOR HidDesc)
    {
        hid.bcdHID = HidDesc->bcdHID;
        hid.bCountryCode = HidDesc->bCountryCode;
        hid.bNumDescriptors = HidDesc->bNumDescriptors;
        for(byte i = 0;i < hid.bNumDescriptors; ++i)
        {
            hid.Descriptors.push_back(UsbHidOptionalDescriptor());
            hid.Descriptors.back().bDescriptorType = HidDesc->OptionalDescriptors[i].bDescriptorType;
            hid.Descriptors.back().wDescriptorLength = HidDesc->OptionalDescriptors[i].wDescriptorLength;
        }
    }
    void _DisplayUnknownDescriptor(UnknownDescriptor& unknown, PUSB_COMMON_DESCRIPTOR CommonDesc)
    {
        unknown.bDescriptorType = CommonDesc->bDescriptorType;
        unknown.Description.Append(ByteArray((PUCHAR)CommonDesc, CommonDesc->bLength));
    }
    void _DisplayUsbDescriptor(UsbDescriptor& usb, 
        PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx,
        PUSB_CONFIGURATION_DESCRIPTOR configDesc, 
        PSTRING_DESCRIPTOR_NODE       stringDescs)
    {
        usb.ConnectionIndex = connectionInfoEx->ConnectionIndex;
        usb.bcdUSB = connectionInfoEx->DeviceDescriptor.bcdUSB;
        usb.bDeviceClass = connectionInfoEx->DeviceDescriptor.bDeviceClass;
        usb.bDeviceSubClass = connectionInfoEx->DeviceDescriptor.bDeviceSubClass;
        usb.bDeviceProtocol = connectionInfoEx->DeviceDescriptor.bDeviceProtocol;
        usb.bMaxPacketSize0 = connectionInfoEx->DeviceDescriptor.bMaxPacketSize0;
        usb.Vid = connectionInfoEx->DeviceDescriptor.idVendor;
        usb.Pid = connectionInfoEx->DeviceDescriptor.idProduct;
        usb.bcdDevice = connectionInfoEx->DeviceDescriptor.bcdDevice;
        usb.iManufacturer = connectionInfoEx->DeviceDescriptor.iManufacturer;
        if(usb.iManufacturer) _DisplayStringDescriptor(usb.iManufacturer, stringDescs, usb.Manufacturer);
        usb.iProduct = connectionInfoEx->DeviceDescriptor.iProduct;
        if(usb.iProduct) _DisplayStringDescriptor(usb.iProduct, stringDescs, usb.Name);
        usb.iSerialNumber = connectionInfoEx->DeviceDescriptor.iSerialNumber;
        if(usb.iSerialNumber) _DisplayStringDescriptor(usb.iSerialNumber, stringDescs, usb.SerialNumber);
        usb.bNumConfigurations = connectionInfoEx->DeviceDescriptor.bNumConfigurations;
        switch(connectionInfoEx->ConnectionStatus)
        {
        case 0:
            usb.ConnectionStatus = "NoDeviceConnected";
            break;
        case 1:
            usb.ConnectionStatus = "DeviceConnected";
            break;
        case 2:
            usb.ConnectionStatus = "DeviceFailedEnumeration";
            break;
        case 3:
            usb.ConnectionStatus = "DeviceGeneralFailure";
            break;
        case 4:
            usb.ConnectionStatus = "DeviceCausedOvercurrent";
            break;
        case 5:
            usb.ConnectionStatus = "DeviceNotEnoughPower";
            break;
        }
        switch(connectionInfoEx->Speed)
        {
        case UsbLowSpeed:
            usb.Speed = "Low";
            break;
        case UsbFullSpeed:
            usb.Speed = "Full";
            break;
        case UsbHighSpeed:
            usb.Speed = "High";
            break;
        default:
            usb.Speed = "Unknown";
            break;
        }
        usb.CurrentConfigurationValue = connectionInfoEx->CurrentConfigurationValue;
        usb.DeviceAddress = connectionInfoEx->DeviceAddress;
        usb.NumberOfOpenPipes = connectionInfoEx->NumberOfOpenPipes;

        /* 设置配置描述信息 */
        PUCHAR                  descEnd;
        PUSB_COMMON_DESCRIPTOR  commonDesc;
        UCHAR                   bInterfaceClass;
        UCHAR                   bInterfaceSubClass;
        BOOL                    displayUnknown;

        bInterfaceClass = 0;
        descEnd = (PUCHAR)configDesc + configDesc->wTotalLength;

        commonDesc = (PUSB_COMMON_DESCRIPTOR)configDesc;

        while((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
            (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
        {
            displayUnknown = FALSE;

            switch(commonDesc->bDescriptorType)
            {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if(commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    displayUnknown = TRUE;
                    break;
                }
                usb.Configurations.push_back(ConfigurationDescriptor());
                _DisplayConfigurationDescriptor(usb.Configurations.back(), 
                    (PUSB_CONFIGURATION_DESCRIPTOR)commonDesc, stringDescs);
                break;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if((commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
                    (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2)))
                {
                    displayUnknown = TRUE;
                    break;
                }
                bInterfaceClass = ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->bInterfaceClass;
                bInterfaceSubClass = ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->bInterfaceSubClass;

                usb.Interfaces.push_back(InterfaceDescriptor());
                _DisplayInterfaceDescriptor(usb.Interfaces.back(),
                    (PUSB_INTERFACE_DESCRIPTOR)commonDesc,
                    stringDescs);
                break;

            case USB_ENDPOINT_DESCRIPTOR_TYPE:
                if((commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
                    (commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR2)))
                {
                    displayUnknown = TRUE;
                    break;
                }
                usb.Pipes.push_back(EndpointDescriptor());
                _DisplayEndpointDescriptor(usb.Pipes.back(), (PUSB_ENDPOINT_DESCRIPTOR)commonDesc);
                break;
            default:
                break;
            }

            if(displayUnknown)
            {
                usb.UnknownDescriptions.push_back(UnknownDescriptor());
                _DisplayUnknownDescriptor(usb.UnknownDescriptions.back(), commonDesc);
            }

            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)(commonDesc) + commonDesc->bLength);
        }
    }
    bool _EnumerateHub(const char* HubName,
        HubDescriptor& hub,
        PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo,
        PUSB_DESCRIPTOR_REQUEST ConfigDesc,
        PSTRING_DESCRIPTOR_NODE StringDescs,
        PCSTR DeviceDesc)
    {
        USB_NODE_INFORMATION   hubInfo;
        USB_HUB_CAPABILITIES   hubCaps;
        USB_HUB_CAPABILITIES_EX hubCapsEx;
        HANDLE                  hHubDevice;
        BOOL                    success;
        ULONG                   nBytes;
        CharConverter hubNameCvt;
        CharConverter cvt;
        if(DeviceDesc)
        {
            hub.Name = DeviceDesc;
        }
        else
        {
            hub.Name = HubName;
        }
        hHubDevice = INVALID_HANDLE_VALUE;
        // Create the full hub device name
        //
        hub.Path = "\\\\.\\";
        hub.Path += HubName;
        // Try to hub the open device
        //
        LOGGER(_log.WriteLine("CreateFile..."));
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

        if(ConnectionInfo != NULL && ConfigDesc != NULL && StringDescs != NULL)
        {
            LOGGER(_log.WriteLine("DisplayUsbDescriptor..."));
            if(hub.Description.size() < 1)
                hub.Description.push_back(UsbDescriptor());
            _DisplayUsbDescriptor(hub.Description.back(), 
                ConnectionInfo, 
                (PUSB_CONFIGURATION_DESCRIPTOR)(ConfigDesc + 1), 
                StringDescs);
        }

        // USB_HUB_CAPABILITIES_EX is only available in Vista and later headers
#if (_WIN32_WINNT >= 0x0600) 

        //
        // Now query USBHUB for the USB_HUB_CAPABILTIES_EX structure for this hub.
        //
        LOGGER(_log.WriteLine("IOCTL_USB_GET_HUB_CAPABILITIES_EX..."));
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
            hub.CapabilityFlags = HubCapFlags->ul;
            hub.IsHighSpeedCapable = HubCapFlags->HubIsHighSpeedCapable;
            hub.IsHighSpeed = HubCapFlags->HubIsHighSpeed;
            hub.IsMulitTransCapable = HubCapFlags->HubIsMultiTtCapable;
            hub.IsMulitTransON = HubCapFlags->HubIsMultiTt;
            hub.IsRootHub = Tobool(HubCapFlags->HubIsRoot);
            hub.IsArmedWakeOnConnect = HubCapFlags->HubIsArmedWakeOnConnect;
            hub.ReservedMBZ = HubCapFlags->ReservedMBZ;
        }
        else
        {
            // 未知的属性
            hub.CapabilityFlags = 0;
            hub.IsHighSpeedCapable = false;
            hub.IsHighSpeed = false;
            hub.IsMulitTransCapable = false;
            hub.IsMulitTransON = false;
            hub.IsRootHub = (ConnectionInfo == NULL);
            hub.IsArmedWakeOnConnect = false;
            hub.ReservedMBZ = 0;
        }
#endif
        //
        // Now query USBHUB for the USB_HUB_CAPABILTIES structure for this hub.
        //
        LOGGER(_log.WriteLine("IOCTL_USB_GET_HUB_CAPABILITIES..."));
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
            hub.Is2xCapable = hubCaps.HubIs2xCapable;
        }
        //
        // Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
        // This will tell us the number of downstream ports to enumerate, among
        // other things.
        //
        LOGGER(_log.WriteLine("IOCTL_USB_GET_NODE_INFORMATION..."));
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
        switch(wHubChar & 0x0004)
        {
        case 0x0000:
            hub.IsCompoundDevice = false;
            break;
        case 0x0004:
            hub.IsCompoundDevice = true;
            break;
        }
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

        // Now recursively enumrate the ports of this hub.
        //
        LOGGER(_log.WriteLine("EnumerateHubPorts..."));
        _EnumerateHubPorts(hHubDevice, hub, hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts);
        CloseHandle(hHubDevice);
        return true;
    }
    void _EnumerateHubPorts(HANDLE hHubDevice, HubDescriptor& hub, ULONG NumPorts)
    {
        ULONG       index;
        BOOL        success;

        PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;
        PUSB_DESCRIPTOR_REQUEST             configDesc = NULL;
        PSTRING_DESCRIPTOR_NODE             stringDescs = NULL;

        string driverKeyName;
        string deviceDesc;

        // Loop over all ports of the hub.
        //
        // Port indices are 1 based, not 0 based.
        //
        LOGGER(_log.WriteLine("Look For Ports..."));
        for(index = 1; index <= NumPorts; index++)
        {
            LOGGER(_log << "Port:<" << index << ">\n");
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
            nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + sizeof(USB_PIPE_INFO) * 30;
            connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)malloc_alloc::allocate(nBytesEx);

            if(connectionInfoEx == NULL)
                break;
            //
            // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION_EX structure
            // for this port.  This will tell us if a device is attached to this
            // port, among other things.
            //
            connectionInfoEx->ConnectionIndex = index;
            LOGGER(_log.WriteLine("IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX..."));
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
                nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
                connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)malloc_alloc::allocate(nBytes);
                connectionInfo->ConnectionIndex = index;
                LOGGER(_log.WriteLine("IOCTL_USB_GET_NODE_CONNECTION_INFORMATION..."));
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
                    malloc_alloc::deallocate(connectionInfo);
                    malloc_alloc::deallocate(connectionInfoEx);
                    continue;
                }

                // Copy IOCTL_USB_GET_NODE_CONNECTION_INFORMATION into
                // IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX structure.
                //
                connectionInfoEx->ConnectionIndex = connectionInfo->ConnectionIndex;
                connectionInfoEx->DeviceDescriptor = connectionInfo->DeviceDescriptor;
                connectionInfoEx->CurrentConfigurationValue = connectionInfo->CurrentConfigurationValue;
                connectionInfoEx->Speed = UCHAR((connectionInfo->LowSpeed) ? UsbLowSpeed : UsbFullSpeed);
                connectionInfoEx->DeviceIsHub = connectionInfo->DeviceIsHub;
                connectionInfoEx->DeviceAddress = connectionInfo->DeviceAddress;
                connectionInfoEx->NumberOfOpenPipes = connectionInfo->NumberOfOpenPipes;
                connectionInfoEx->ConnectionStatus = connectionInfo->ConnectionStatus;

                memcpy(&connectionInfoEx->PipeList[0],
                    &connectionInfo->PipeList[0],
                    sizeof(USB_PIPE_INFO) * 30);

                malloc_alloc::deallocate(connectionInfo);
            }

            // If there is a device connected, get the Device Description
            //
            deviceDesc = "";
            if(connectionInfoEx->ConnectionStatus != NoDeviceConnected)
            {
                LOGGER(_log.WriteLine("GetDriverKeyName..."));
                if(_GetDriverKeyName(hHubDevice, index, driverKeyName))
                {
                    LOGGER(_log.WriteLine("DriverNameToDeviceDesc..."));
                    _DriverNameToDeviceDesc(driverKeyName.c_str(), deviceDesc, FALSE);
                }
            }

            // If there is a device connected to the port, try to retrieve the
            // Configuration Descriptor from the device.
            //
            if(connectionInfoEx->ConnectionStatus == DeviceConnected)
            {
                LOGGER(_log.WriteLine("GetConfigDescriptor..."));
                configDesc = _GetConfigDescriptor(hHubDevice, index, 0);
            }
            else
            {
                configDesc = NULL;
            }

            LOGGER(_log.WriteLine("AreThereStringDescriptors..."));
            if(configDesc != NULL && _AreThereStringDescriptors(&connectionInfoEx->DeviceDescriptor,
                (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc + 1)))
            {
                LOGGER(_log.WriteLine("GetAllStringDescriptors..."));
                stringDescs = _GetAllStringDescriptors(
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
                LOGGER(_log.WriteLine("GetExternalHubName..."));
                if(_GetExternalHubName(hHubDevice, index, extHubName))
                {
                    hub.ExternalHub.push_back(HubDescriptor());
                    hub.ExternalHub.back().Name = extHubName;
                    hub.ExternalHub.back().ConnectionIndex = connectionInfoEx->ConnectionIndex;

                    LOGGER(_log.WriteLine("EnumerateHub..."));
                    _EnumerateHub(extHubName.c_str(),
                        hub.ExternalHub.back(),
                        connectionInfoEx,
                        configDesc,
                        stringDescs,
                        deviceDesc.c_str());
                }
            }
            else
            {
                // Allocate some space for a USBDEVICEINFO structure to hold the
                // Config Descriptors, Strings Descriptors, and connection info
                // pointers.  GPTR zero initializes the structure for us.
                //
                if(configDesc != NULL)
                {
                    /* 设置USB设备属性 */
                    hub.ExteranlUsb.push_back(UsbDescriptor());
                    hub.ExteranlUsb.back().ConnectionIndex = connectionInfoEx->ConnectionIndex;

                    LOGGER(_log.WriteLine("DisplayUsbDescriptor..."));
                    _DisplayUsbDescriptor(hub.ExteranlUsb.back(), connectionInfoEx,
                        (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc + 1), stringDescs);
                }
            }

            malloc_alloc::deallocate(connectionInfoEx);
            connectionInfoEx = NULL;

            if(configDesc)
            {
                malloc_alloc::deallocate(configDesc);
                configDesc = NULL;
            }

            if(stringDescs != NULL)
            {
                PSTRING_DESCRIPTOR_NODE Next;

                do
                {
                    Next = stringDescs->Next;
                    malloc_alloc::deallocate(stringDescs);
                    stringDescs = Next;

                } while(stringDescs != NULL);

                stringDescs = NULL;
            }
        }
    }
    bool _EnumerateHostController(HANDLE hHCDev, BusDescriptor& bus)
    {
        LOGGER(_log.WriteLine("GetHCDDriverKeyName..."));
        if(!_GetHCDDriverKeyName(hHCDev, bus.DriverKey))
            return false;

        string deviceId;
        LOGGER(_log.WriteLine("DriverNameToDeviceDesc..."));
        if(!_DriverNameToDeviceDesc(bus.DriverKey.c_str(), deviceId, true))
            return false;

        LOGGER(_log.WriteLine("UnFormat Hub ID..."));
        ULONG ven, dev, subsys, rev;
        if(sscanf(deviceId.c_str(), "PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x",
            &ven, &dev, &subsys, &rev) != 4)
        {
            return false;
        }
        bus.VendorID = static_cast<ushort>(ven);
        bus.DeviceID = static_cast<ushort>(dev);
        bus.SubSysID = subsys;
        bus.Revision = rev;

        LOGGER(_log.WriteLine("DriverNameToDeviceDesc..."));
        if(!_DriverNameToDeviceDesc(bus.DriverKey.c_str(), bus.Name, FALSE))
            return false;

        // 挂载ID
        bus.RootHub.ConnectionIndex = 0;
        LOGGER(_log.WriteLine("GetRootHubName..."));
        if(!_GetRootHubName(hHCDev, bus.RootHub.DeviceKey))
            return true;
        LOGGER(_log.WriteLine("EnumerateHub..."));
        _EnumerateHub(bus.RootHub.DeviceKey.c_str(), bus.RootHub, NULL, NULL, NULL, "RootHub");
        return true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 枚举USB总线上的所有信息
    size_t EnumUsbBus(list<BusDescriptor>& buslist)
    {
        LOG_FUNC_NAME();

        const size_t NUM_HCS_TO_CHECK = 10;

        CharConverter cvt;
        char_t HCName[16];
        HANDLE hHCDev;
        size_t count = 0;
        ByteBuilder tmp(64);

        LOGGER(_log.WriteLine("Look For HCS..."));
        for(size_t HCNum = 0;HCNum < NUM_HCS_TO_CHECK; ++HCNum)
        {
            LOGGER(_log << "HCNum:<" << HCNum << ">\n");
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

                LOGGER(_log.WriteLine("EnumerateHostController..."));
                if(_EnumerateHostController(hHCDev, buslist.back()))
                {
                    ++count;
                }
                else
                {
                    buslist.pop_back();
                }
                CloseHandle(hHCDev);
            }
        }
        LOGGER(_log.WriteLine("SetupDiGetClassDevs..."));
        HDEVINFO deviceInfo = SetupDiGetClassDevs((LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
            NULL, NULL,
            (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
        if(deviceInfo == INVALID_HANDLE_VALUE)
            return _logRetValue(count);

        SP_DEVICE_INTERFACE_DATA         deviceInfoData;
        PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData;
        ULONG                            index;
        ULONG                            requiredLength;

        deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        LOGGER(_log.WriteLine("Look For SetupDiEnumDeviceInterfaces..."));
        for(index = 0;
            SetupDiEnumDeviceInterfaces(deviceInfo,
            0,
            (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
            index,
            &deviceInfoData);
            index++)
        {
            LOGGER(_log.WriteLine("SetupDiGetDeviceInterfaceDetail..."));
            SetupDiGetDeviceInterfaceDetail(deviceInfo,
                &deviceInfoData,
                NULL,
                0,
                &requiredLength,
                NULL);

            deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, requiredLength);
            deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            SetupDiGetDeviceInterfaceDetail(deviceInfo,
                &deviceInfoData,
                deviceDetailData,
                requiredLength,
                &requiredLength,
                NULL);

            // 检测是否已经获取过信息
            tmp.Clear();
            tmp = cvt.to_char(deviceDetailData->DevicePath);
            StringConvert::ToUpper(tmp);
            LOGGER(_log << "Path:<" << tmp.GetString() << ">\n");
            ULONG venId, devId, subsysId, revId;
            if(sscanf(tmp.GetString(), "\\\\?\\PCI#VEN_%x&DEV_%x&SUBSYS_%x&REV_%x",
                &venId, &devId, &subsysId, &revId) != 4)
            {
                continue;
            }
            list<BusDescriptor>::iterator itr = buslist.begin();
            for(itr = buslist.begin();itr != buslist.end(); ++itr)
            {
                if((*itr).VendorID == venId &&
                    (*itr).DeviceID == devId &&
                    (*itr).SubSysID == subsysId &&
                    (*itr).Revision == revId)
                {
                    break;
                }
            }
            if(itr != buslist.end())
                break;

            LOGGER(_log.WriteLine("CreateFile..."));
            hHCDev = CreateFile(deviceDetailData->DevicePath,
                GENERIC_WRITE,
                FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            // If the handle is valid, then we've successfully opened a Host
            // Controller.  Display some info about the Host Controller itself,
            // then enumerate the Root Hub attached to the Host Controller.
            //
            if(hHCDev != INVALID_HANDLE_VALUE)
            {
                buslist.push_back(BusDescriptor());
                LOGGER(_log.WriteLine("EnumerateHostController..."));
                if(_EnumerateHostController(hHCDev, buslist.back()))
                {
                    ++count;
                }
                else
                {
                    buslist.pop_back();
                }
                CloseHandle(hHCDev);
            }

            GlobalFree(deviceDetailData);
        }
        LOGGER(_log.WriteLine("SetupDiDestroyDeviceInfoList..."));
        SetupDiDestroyDeviceInfoList(deviceInfo);

        return _logRetValue(count);
    }
    /// 枚举当前系统上连接的USB设备显示名称
    size_t EnumUsbName(list<string>& devlist)
    {
        GUID USB_GUID = { 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED };
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&USB_GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
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
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
#pragma warning(default:4200)
//--------------------------------------------------------- 
} // namesapce env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 
