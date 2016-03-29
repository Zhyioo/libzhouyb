//========================================================= 
/**@file UsbDevice.h 
 * @brief Linux下USB设备类型基础库 
 * 
 * @warning 需要预装libusb库  
 * 
 * @date 2013-05-07   21:33:14 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#ifdef _MSC_VER
#   include <lusb0_usb.h>
#   pragma comment(lib, "libusb")
#else
#   include <usb.h>
#endif

#include "../Handler.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// 获取usb_device*中的VID信息
#define USB_VID(dev)     ((dev)->descriptor.idVendor)
/// 获取usb_device*中的PID信息
#define USB_PID(dev)     ((dev)->descriptor.idProduct)
/// 生成varId值
#define USB_MAKE_ID(configId, interfaceId, altsettingId) static_cast<int>(((configId&0x0FF)<<16)|((interfaceId&0x0FF)<<8)|(altsettingId&0x0FF))
//--------------------------------------------------------- 
/// USB设备句柄定义 
struct UsbHandler : public Handler<usb_dev_handle*>
{
    /// 设备信息 
    struct usb_device* usb_dev;
    /// 设备配置  
    int configId;
    /// 设备接口配置 
    int interfaceId;
    /// 设备接口配置值 
    int interfaceVal;
    /// 设备端点配置 
    int altsettingId;
};
/// USB设备句柄工厂类 
class UsbHandlerFactory : public IFactory<UsbHandler>, public LoggerBehavior
{
public:
    //----------------------------------------------------- 
    /// UsbDevice.Open参数配置项主键 UsbDevicePtr
    static const char UsbDevicePtrKey[16];
    /// UsbDevice.Open参数配置项主键 Config
    static const char ConfigKey[10];
    /// UsbDevice.Open参数配置项主键 Interface
    static const char InterfaceKey[16];
    /// UsbDevice.Open参数配置项主键 AltSetting
    static const char AltSettingKey[16];
    //----------------------------------------------------- 
    static string ToArg(struct usb_device* dev, int varId = 0)
    {
        string sArg;

        int configId = (varId >> 16) & 0x0FF;
        int interfaceId = (varId >> 8) & 0x0FF;
        int altsettingId = (varId & 0x0FF);

        sArg += ArgConvert::ToConfig<pointer>(UsbDevicePtrKey, dev);
        sArg += ArgConvert::ToConfig<int>(ConfigKey, configId);
        sArg += ArgConvert::ToConfig<int>(InterfaceKey, interfaceId);
        sArg += ArgConvert::ToConfig<int>(AltSettingKey, altsettingId);

        return sArg;
    }
    //----------------------------------------------------- 
    /// 设置配置 
    bool SetConfiguration(UsbHandler& obj)
    {
        if(obj.configId == 0) return true;
        LOGGER(_log.WriteLine("usb_set_configuration..."));
        int cfgVal = obj.usb_dev->config[obj.configId].bConfigurationValue;
        int iRet = usb_set_configuration(obj.Handle, cfgVal);
        LOGGER(_log << "usb_set_configuration(" << cfgVal << ") = [" << iRet << "]\n");

        return Tobool(iRet >= 0);
    }
    /// 注册接口配置 
    bool ClaimInterface(UsbHandler& obj)
    {
        /* 从系统重新认领接口 */
        int iRet = 0;
        int val = obj.interfaceVal;
        // linux下不可移植的函数接口
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
        LOGGER(_log.WriteLine("usb_detach_kernel_driver_np..."));
        iRet = usb_detach_kernel_driver_np(obj.Handle, val);
#endif
        LOGGER(_log << "usb_detach_kernel_driver_np(" << val << ") = [" << iRet << "]\n");
        /* 发送数据前的接口初始化 */
        LOGGER(_log.WriteLine("usb_claim_interface..."));
        iRet = usb_claim_interface(obj.Handle, val);
        LOGGER(_log << "usb_claim_interface(" << val << ") = [" << iRet << "]\n");

        return Tobool(iRet >= 0);
    }
    /// 注册接口类型 
    bool SetAltsetting(UsbHandler& obj)
    {
        LOGGER(_log.WriteLine("usb_set_altinterface..."));
        int altsettingVal = obj.usb_dev->config[obj.configId].interface[obj.interfaceId].altsetting[obj.altsettingId].bAlternateSetting;
        int iRet = usb_set_altinterface(obj.Handle, altsettingVal);
        LOGGER(_log << "usb_set_altinterface(" << altsettingVal << ") = [" << iRet << "]\n");
        
        return Tobool(iRet >= 0);
    }
    /// 释放接口类型 
    bool ReleaseInterface(UsbHandler& obj)
    {
        int val = obj.interfaceVal;
        /* 接收数据后的接口恢复 */
        LOGGER(_log.WriteLine("usb_release_interface..."));
        int iRet = usb_release_interface(obj.Handle, val);
        LOGGER(_log << "usb_release_interface(" << val << ") = [" << iRet << "]\n");
        
        return Tobool(iRet >= 0);
    }
    //----------------------------------------------------- 
    /// 创建句柄 
    virtual bool Create(UsbHandler& obj, const char* sArg = NULL)
    {
        int configId = 0;
        int interfaceId = 0;
        int altsettingId = 0;
        struct usb_device* dev = NULL;

        ArgParser cfg;
        LOGGER(_log.WriteLine("ParseArg..."));
        if(!cfg.Parse(sArg))
            return false;
        pointer pDev = NULL;
        LOGGER(_log.WriteLine("Parse UsbDevicePtr..."));
        if(!ArgConvert::FromConfig<pointer>(cfg, UsbDevicePtrKey, pDev))
            return false;
        dev = reinterpret_cast<struct usb_device*>(pDev);
        ArgConvert::FromConfig<int>(cfg, ConfigKey, configId);
        ArgConvert::FromConfig<int>(cfg, InterfaceKey, interfaceId);
        ArgConvert::FromConfig<int>(cfg, AltSettingKey, altsettingId);

        return Create(obj, dev, configId, interfaceId, altsettingId);
    }
    /// 创建句柄 
    bool Create(UsbHandler& obj, struct usb_device* dev, int configId = 0, int interfaceId = 0, int altsettingId = 0)
    {
        LOGGER(_log << "UsbDevice:<" << _hex_num(dev) << ">\n"
            << "ConfigId:<" << configId << ">\n"
            << "InterfaceId:<" << interfaceId << ">\n"
            << "AltsettingId:<" << altsettingId << ">\n");

        LOGGER(_log.WriteLine("usb_open..."));
        obj.Handle = usb_open(dev);
        if(!IsValid(obj))
            return false;

        if(configId >= dev->descriptor.bNumConfigurations)
        {
            LOGGER(_log.WriteLine("Config范围错误,重置为:<0>"));
            configId = 0;
        }
        if(interfaceId >= dev->config[configId].bNumInterfaces)
        {
            LOGGER(_log.WriteLine("Interface范围错误,重置为:<0>"));
            interfaceId = 0;
        }
        if(altsettingId >= dev->config[configId].interface[interfaceId].num_altsetting)
        {
            LOGGER(_log.WriteLine("AltSetting范围错误,重置为:<0>"));
            altsettingId = 0;
        }

        obj.usb_dev = dev;
        obj.configId = configId;
        obj.interfaceId = interfaceId;
        obj.altsettingId = altsettingId;
        obj.interfaceVal = dev->config[configId].interface[interfaceId].altsetting[altsettingId].bInterfaceNumber;

        bool bSetting = SetConfiguration(obj) && ClaimInterface(obj) && SetAltsetting(obj);
        if(!bSetting)
        {
            Dispose(obj);
            LOGGER(_log.WriteLine("设备设置失败"));
            return false;
        }

        return true;
    }
    /// 返回句柄是否有效 
    virtual bool IsValid(const UsbHandler& obj)
    {
        return obj.Handle != NULL;
    }
    /// 关闭句柄 
    virtual void Dispose(UsbHandler& obj)
    {
        LOGGER(_log.WriteLine("usb_close..."));
        int iRet = usb_close(obj.Handle);
        LOGGER(_log << "usb_close() = [" << iRet << "]\n");
        obj.Handle = NULL;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// linux下USB设备操作支持(需要预装libusb-compat库)
template<class THandlerDevice>
class UsbHandlerAppender : public THandlerDevice
{
protected:
    //----------------------------------------------------- 
    /// 输出端点信息
    static void _PrintEndPoint(struct usb_endpoint_descriptor* endpoint, usb_dev_handle*, LoggerAdapter& logger)
    {
        logger<<"     bEndpointAddress: "<<_hex(endpoint->bEndpointAddress)<<"h"<<endl;
        logger<<"     bmAttributes:     "<<_hex(endpoint->bmAttributes)<<"h"<<endl;
        logger<<"     wMaxPacketSize:   "<<static_cast<int>(endpoint->wMaxPacketSize)<<endl;
        logger<<"     bInterval:        "<<static_cast<int>(endpoint->bInterval)<<endl;
        logger<<"     bRefresh:         "<<static_cast<int>(endpoint->bRefresh)<<endl;
        logger<<"     bSynchAddress:    "<<static_cast<int>(endpoint->bSynchAddress)<<endl;
    }
    /// 输出接口配置信息
    static void _PrintAltsetting(struct usb_interface_descriptor* interface_descriptor, usb_dev_handle* udev, LoggerAdapter& logger)
    {
        logger << "    bInterfaceNumber:   " << static_cast<int>(interface_descriptor->bInterfaceNumber) << endl;
        logger << "    bAlternateSetting:  " << static_cast<int>(interface_descriptor->bAlternateSetting) << endl;
        logger << "    bNumEndpoints:      " << static_cast<int>(interface_descriptor->bNumEndpoints) << endl;
        logger << "    bInterfaceClass:    " << static_cast<int>(interface_descriptor->bInterfaceClass) << endl;
        logger << "    bInterfaceSubClass: " << static_cast<int>(interface_descriptor->bInterfaceSubClass) << endl;
        logger << "    bInterfaceProtocol: " << static_cast<int>(interface_descriptor->bInterfaceProtocol) << endl;
        logger << "    iInterface:         " << static_cast<int>(interface_descriptor->iInterface) << endl;
        if(udev)
        {
            char str[256] = {0};
            int ret = usb_get_string_simple(udev, static_cast<int>(interface_descriptor->iInterface), str, sizeof(str));
            if(ret > 0)
            {
                logger << "    sInterface:         " << str <<endl;
            }
        }

        for(int i = 0; i < interface_descriptor->bNumEndpoints; ++i)
        {
            logger<<"    Endpoint:<"<<i<<">\n";
            _PrintEndPoint(&interface_descriptor->endpoint[i], udev, logger);
        }
    }
    /// 输出接口信息
    static void _PrintInterface(struct usb_interface* usbInterface, usb_dev_handle* udev, LoggerAdapter& logger)
    {
        for(int i = 0; i < usbInterface->num_altsetting; i++)
        {
            logger<<"    Altsetting:<"<<i<<">\n";
            _PrintAltsetting(&usbInterface->altsetting[i], udev, logger);
        }
    }
    /// 输出配置信息
    static void _PrintConfiguration(struct usb_config_descriptor *config, usb_dev_handle* udev, LoggerAdapter& logger)
    {
        logger<<"  wTotalLength:         "<<static_cast<int>(config->wTotalLength)<<endl;
        logger<<"  bNumInterfaces:       "<<static_cast<int>(config->bNumInterfaces)<<endl;
        logger<<"  bConfigurationValue:  "<<static_cast<int>(config->bConfigurationValue)<<endl;
        logger<<"  iConfiguration:       "<<static_cast<int>(config->iConfiguration)<<endl;
        logger<<"  bmAttributes:         "<<_hex(config->bmAttributes)<<"h"<<endl;
        logger<<"  MaxPower:             "<<static_cast<int>(config->MaxPower)<<endl;

        for(int i = 0; i < config->bNumInterfaces; ++i)
        {
            logger<<"   Interface:<"<<i<<">\n";
            _PrintInterface(&config->interface[i], udev, logger);
        }
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 设备类型定义 
    typedef struct usb_device* device_info;
    //----------------------------------------------------- 
    /// 输出设备信息
    static void PrintUsbTree(device_info dev, LoggerAdapter& logger)
    {
        usb_dev_handle *udev;
        logger<<"USB Descriptor:#["<<_hex(dev->descriptor.idVendor)
            <<"]-["<<_hex(dev->descriptor.idProduct)<<"]"<<endl;

        char str[256] = {0};
        int ret = 0;

        udev = usb_open(dev);
        if(udev)
        {
            // 厂商信息
            if(dev->descriptor.iManufacturer)
            {
                ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, str, sizeof(str));
                if(ret > 0) logger<<" Manufacturer:   "<<str<<endl;
            }
            if(dev->descriptor.iProduct)
            {
                ret = usb_get_string_simple(udev, dev->descriptor.iProduct, str, sizeof(str));
                if(ret > 0) logger<<" Product:        "<<str<<endl;
            }
            if(dev->descriptor.iSerialNumber)
            {
                ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, str, sizeof(str));
                if(ret > 0) logger<<" SerialNumber:   "<<str<<endl;
            }
        }
        logger<<" bMaxPacketSize0:"<<static_cast<int>(dev->descriptor.bMaxPacketSize0)<<endl;
        // 配置信息
        if(dev->config)
        {
            for (int i = 0; i < dev->descriptor.bNumConfigurations; ++i)
            {
                logger<<" Configuration:<"<<i<<">\n";
                _PrintConfiguration(&dev->config[i], udev, logger);
            }
        }
        usb_close(udev);
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 输出错误信息
    virtual void _logErr(int errCode, const char* errinfo = NULL)
    {
        string msg = _strput(errinfo);
        if(msg.length() > 0)
            msg += " ";
        msg += usb_strerror();
        DeviceBehavior::_logErr(errCode, msg.c_str());
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    /**
     * @brief 枚举系统中所有的USB设备
     * @param [out] _list 获取到的设备名列表
     * @retval -1 枚举过程中出现错误
     * @retval 其他 获取到的HID设备数目
     */
    int EnumDevice(list<device_info>& _list)
    {
        /* Log Header */
        LOGGER(LoggerAdapter _log = THandlerDevice::_log);
        LOG_FUNC_NAME();

        int enumCount = -1;

        int devCount = 0;
        int busCount = 0;
        // 初始化环境
        usb_init();
        LOGGER(_log.WriteLine("usb_init();"));

        // 获取总线数
        busCount = usb_find_busses();
        LOGGER(_log<<"usb_find_busses:["<<busCount<<"]\n");

        if(busCount < 0)
        {
            LOGGER(_log<<"usb_find_busses() err:["<<usb_strerror()<<"]\n");
            // return -1;Error
            return enumCount;
        }
        devCount = usb_find_devices();
        LOGGER(_log<<"usb_find_devices:["<<devCount<<"]\n");

        /* 遍历总线 */
        struct usb_bus* bus = NULL;
        struct usb_device* dev = NULL;
        enumCount = 0;
        for(bus = usb_get_busses();bus;bus = bus->next)
        {
            LOGGER(_log<<"buss : <"<<bus<<">\n");

            for(dev = bus->devices;dev;dev = dev->next)
            {
                ++enumCount;

                /* 按照 序号+数据 的格式输出 */
                LOGGER(_log<<"dev  : <"<<enumCount<<">\n";
                PrintUsbTree(dev, _log));

                _list.push_back(dev);
            }
        }

        return THandlerDevice::_logRetValue(enumCount);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// Usb基础设备 
typedef UsbHandlerAppender<HandlerBaseDevice<UsbHandler, UsbHandlerFactory> > UsbBaseDevice;
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device
} // namespace zhou_yb
//========================================================= 
