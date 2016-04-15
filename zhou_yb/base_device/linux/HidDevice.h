//========================================================= 
/**@file HidDevice.h 
 * @brief Linux下的HID设备 
 * 
 * @warning 需要预装libusb库
 * 
 * @date 2013-05-05   15:33:41 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "UsbDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// HID设备句柄 
struct HidHandler : public UsbHandler
{
    /// 输入包长度 
    ushort InputLength;
    /// 输出包长度 
    ushort OutputLength;
    /// 输入端点 
    byte EndpointIn;
    /// 输出端点 
    byte EndpointOut;
    /// 端点类型 
    byte Attributes;
    /// 报告ID
    byte ReportId;
};
/// HID设备句柄工厂类 
class HidHandlerFactory : public UsbHandlerFactory
{
public:
    virtual bool Create(HidHandler& obj, const char* sArg = NULL)
    {
        if(!UsbHandlerFactory::Create(obj))
            return false;

        struct usb_config_descriptor* pConfig = &(obj.usb_dev->config[obj.configId]);
        struct usb_interface* pInterface = &(pConfig->interface[obj.interfaceId]);
        int interfaceClass = pInterface->altsetting[obj.altsettingId].bInterfaceClass;
        
        // 不是HID接口设备
        if(interfaceClass != USB_CLASS_HID)
        {
            LOGGER(_log.WriteLine("InterfaceClass is not USB_CLASS_HID."));
            return false;
        }

        /* 默认值 */
        obj.EndpointIn = USB_ENDPOINT_IN;
        obj.EndpointOut = USB_ENDPOINT_OUT;

        obj.InputLength = obj.usb_dev->descriptor.bMaxPacketSize0;
        obj.OutputLength = obj.usb_dev->descriptor.bMaxPacketSize0;

        // 查找端点
        for(int i = 0;i < pInterface->altsetting[obj.altsettingId].bNumEndpoints; ++i)
        {
            // HID端点
            struct usb_endpoint_descriptor* endpoint = &pInterface->altsetting[obj.altsettingId].endpoint[i];
            obj.Attributes = endpoint->bmAttributes;
            if(obj.Attributes == USB_ENDPOINT_TYPE_INTERRUPT)
            {
                LOGGER(_log.WriteLine("Endpoint is USB_ENDPOINT_TYPE_INTERRUPT."));
                // 输出端点
                if(BitConvert::IsMask(endpoint->bEndpointAddress, USB_ENDPOINT_IN))
                {
                    obj.EndpointIn = endpoint->bEndpointAddress;
                    obj.InputLength = endpoint->wMaxPacketSize;
                    LOGGER(_log.WriteLine("set epIn & wMaxPacketSize..."));
                }
                else
                {
                    obj.EndpointOut = endpoint->bEndpointAddress;
                    obj.OutputLength = endpoint->wMaxPacketSize;
                    LOGGER(_log.WriteLine("set epOut & wMaxPacketSize..."));
                }
            }
        }
        LOGGER(_log << "EndpointIn :<" << _hex_num(obj.EndpointIn) << ">,PackerSize:<" << obj.InputLength << ">\n");
        LOGGER(_log << "EndpointOut:<" << _hex_num(obj.EndpointOut) << ">,PackerSize:<" << obj.OutputLength << ">\n");

        return true;
    }
};
//---------------------------------------------------------
/// HID控制传输通信句柄读取器 
class HidControlHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<HidHandler> _handle;
    /// 超时时间
    uint _timeoutMs;
public:
    HidControlHandlerReader(const HidHandler& handle)
    {
        _handle = handle;
    }
    /// 开始读数据 
    virtual bool Async(uint timeoutMs) { _timeoutMs = timeoutMs; return true; }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        const size_t HID_REPORT_GET = 0x01;
        const int requestType = USB_ENDPOINT_IN + USB_TYPE_CLASS + USB_RECIP_INTERFACE;
        const int requestValue = _handle->ReportId + (USB_REQ_SET_FEATURE << 8);
        int recvLen = 0;

        // 直接使用data作为输出缓冲区 
        size_t lastlen = data.GetLength();
        data.Append(static_cast<byte>(0x00), _handle->InputLength);
        char* pBuf = reinterpret_cast<char*>(const_cast<byte*>(data.GetBuffer()));
        recvLen = usb_control_msg(_handle->Handle, requestType, HID_REPORT_GET, requestValue, 
            _handle->interfaceVal, pBuf, _handle->InputLength, static_cast<int>(_timeoutMs));
        bool isRead = (recvLen > 0);
        if(isRead && ByteConvert::OrVal(ByteArray(pBuf, _handle->InputLength)) != static_cast<byte>(0x00))
            return SIZE_EOF;

        // 读取失败或者读到的数据是0包 
        data.RemoveTail(_handle->InputLength);
        return 0;
    }
};
/// HID控制传输通信句柄写入器 
class HidControlHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<HidHandler> _handle;
    /// 待写入的数据 
    ByteArray _data;
    /// 超时时间
    uint _timeoutMs;
public:
    HidControlHandlerWriter(const HidHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint timeoutMs)
    {
        _data = data;
        _timeoutMs = timeoutMs;
        return true;
    }
    /// 写数据 
    virtual size_t Wait()
    {
        const size_t HID_REPORT_SET = 0x09;
        const int requestType = USB_ENDPOINT_OUT + USB_TYPE_CLASS + USB_RECIP_INTERFACE;
        const int requestValue = _handle->ReportId + (USB_REQ_SET_FEATURE << 8);
        char* pbuff = reinterpret_cast<char*>(const_cast<byte*>(_data.GetBuffer()));
        int sendLen = usb_control_msg(_handle->Handle, requestType, HID_REPORT_SET, requestValue, 
            _handle->interfaceVal, pbuff, _handle->OutputLength, static_cast<int>(_timeoutMs));
        bool bRet = (sendLen >= 0 && sendLen == (_handle->OutputLength));
        return bRet ? SIZE_EOF : 0;
    }
};
/// HID中断传输通信句柄读取器 
class HidInterruptHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<HidHandler> _handle;
    /// 超时时间
    uint _timeoutMs;
public:
    HidInterruptHandlerReader(const HidHandler& handle) { _handle = handle; }
    /// 开始读数据 
    virtual bool Async(uint timeoutMs) { _timeoutMs = timeoutMs; return true; }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        // 直接使用data作为输出缓冲区 
        size_t lastlen = data.GetLength();
        data.Append(static_cast<byte>(0x00), _handle->InputLength);
        char* pBuf = reinterpret_cast<char*>(const_cast<byte*>(data.GetBuffer()));
        int recvLen = usb_interrupt_read(_handle->Handle, _handle->EndpointIn, pBuf, _handle->InputLength, 
            static_cast<int>(_timeoutMs));
        bool bRet = static_cast<bool>(recvLen > 0 && recvLen == _handle->InputLength);
        if(bRet) return SIZE_EOF;
        
        data.RemoveTail(_handle->InputLength);
        return 0;
    }
};
/// HID中断传输通信句柄写入器 
class HidInterruptHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄
    Ref<HidHandler> _handle;
    /// 待写入的数据
    ByteArray _data;
    /// 超时时间
    uint _timeoutMs;
public:
    HidInterruptHandlerWriter(const HidHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint timeoutMs)
    {
        _data = data;
        _timeoutMs = timeoutMs;
        return true;
    }
    /// 写数据 
    virtual size_t Wait()
    {
        char* pbuf = reinterpret_cast<char*>(const_cast<byte*>(_data.GetBuffer()));
        int sendLen = usb_interrupt_write(_handle->Handle, _handle->EndpointOut, pbuf, _handle->OutputLength, static_cast<int>(_timeoutMs));
        bool bRet = bool(sendLen >= 0 && sendLen == _handle->OutputLength);
        return bRet ? SIZE_EOF : 0;
    }
};
//--------------------------------------------------------- 
/// linux下HID设备操作支持(需要预装libusb库)
class HidHandlerDevice : 
    public HandlerBaseDevice<HidHandler, HidHandlerFactory>,
    public IInteractiveTrans
{
protected:
    //----------------------------------------------------- 
    /// 数据的临时缓冲区 
    ByteBuilder _tmpBuff;
    //----------------------------------------------------- 
public:
    //-----------------------------------------------------
    HidHandlerDevice() : HandlerBaseDevice() { ReportId = 0x00; }
    //----------------------------------------------------- 
    /* Open IsOpen Close Read Write接口采用UsbHandlerDevice提供的默认实现 */
    //-----------------------------------------------------
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        _hDev.ReportId = ReportId;
        _tmpBuff.Clear();
        bool bRead = false;
        
        if(_hDev.Attributes == USB_ENDPOINT_TYPE_INTERRUPT)
        {
            HidInterruptHandlerReader interruptReader(_hDev);
            bRead = HandlerBaseDevice::Read(interruptReader, _tmpBuff);
        }
        else
        {
            HidControlHandlerReader controlReader(_hDev);
            bRead = HandlerBaseDevice::Read(controlReader, _tmpBuff);
        }
        if(bRead)
        {
            // 过滤第一个字节的 ReportId
            data.Append(_tmpBuff.SubArray(1));
        }
        return bRead;
    }
    /// 写数据 
    virtual bool Write(const ByteArray& data)
    {
        _hDev.ReportId = ReportId;
        _tmpBuff.Clear();
        _tmpBuff += ReportId;
        _tmpBuff += data;
        ByteConvert::Fill(_tmpBuff, _hDev.OutputLength, true, 0x00);
        bool bWrite = false;
        if(_hDev.Attributes == USB_ENDPOINT_TYPE_INTERRUPT)
        {
            HidInterruptHandlerWriter interruptWriter(_hDev);
            bWrite = HandlerBaseDevice::Write(interruptWriter, _tmpBuff);
        }
        else
        {
            HidControlHandlerWriter controlWriter(_hDev);
            bWrite = HandlerBaseDevice::Write(controlWriter, _tmpBuff);
        }

        return bWrite;
    }
    //----------------------------------------------------- 
    /// 获取HID发送数据包大小
    inline size_t GetSendLength() const
    {
        return _hDev.OutputLength - 1;
    }
    /// 获取HID接收数据包大小
    inline size_t GetRecvLength() const
    {
        return _hDev.InputLength - 1;
    }
    /// 设备包的ReportId
    byte ReportId;
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
/// HID设备
typedef UsbHandlerAppender<HidHandlerDevice> HidDevice;
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device
} // namespace zhou_yb
//========================================================= 
