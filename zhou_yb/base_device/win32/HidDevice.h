//========================================================= 
/**@file HidDevice.h 
 * @brief Windows下 Hid设备通信实现 
 * 
 * @date 2012-03-05   21:33:35 
 * @author Zhyioo 
 * @version 1.0
 */
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDDEVICE_H_
#define _LIBZHOUYB_HIDDEVICE_H_
//--------------------------------------------------------- 
#include "FileDevice.h"

extern "C"
{
#   include <hidsdi.h>
#   include <setupapi.h>
#   pragma comment(lib, "setupapi.lib")
#   pragma comment(lib, "hid.lib")
}
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// HID设备句柄 
struct HidHandler : public WinAsyncHandler
{
    /// 接收包长度 
    size_t InputLength;
    /// 发送包长度 
    size_t OutputLength;
    /// 输入端点 
    byte EndpointIn;
    /// 输出端点 
    byte EndpointOut;
    /// 端点类型 
    byte Attributes;
};
//--------------------------------------------------------- 
/// HID句柄工厂类 
class HidHandlerFactory : public FileHandlerFactory
{
protected:
    /// 获取HID属性信息 
    static bool _SetHidHandler(HidHandler& handle)
    {
        HIDD_ATTRIBUTES hidAttributes;
        if(!HidD_GetAttributes(handle.Handle, &hidAttributes))
            return false;

        PHIDP_PREPARSED_DATA hidPreparsedData;
        HIDP_CAPS capab;
        if(!HidD_GetPreparsedData(handle.Handle, &hidPreparsedData))
            return false;

        bool bRet = false;
        if(HidP_GetCaps(hidPreparsedData, &capab) == HIDP_STATUS_SUCCESS)
        {
            handle.InputLength = capab.InputReportByteLength;
            handle.OutputLength = capab.OutputReportByteLength;
            bRet = true;
        }

        HidD_FreePreparsedData(hidPreparsedData);
        return bRet;
    }
public:
    /// 创建HID设备句柄
    virtual bool Create(HidHandler& obj, const char* sArg = NULL)
    {
        if(!FileHandlerFactory::Create(obj, sArg))
            return false;
        /// 获取HID相关属性 
        LOGGER(_log.WriteLine("SetHidHandler..."));
        if(!_SetHidHandler(obj))
        {
            FileHandlerFactory::Dispose(obj);
            LOGGER(_log.WriteLine("获取HID属性失败"));
            return false;
        }
        LOGGER(_log << "InputLength:<" << obj.InputLength << ">\n"
            << "OutputLength:<" << obj.OutputLength << ">\n"
            << "EndpointIn:<" << _hex_num(obj.EndpointIn) << ">\n"
            << "EndpointOut:<" << _hex_num(obj.EndpointOut) << ">\n");
        return true;
    }
};
//--------------------------------------------------------- 
/// HID设备的一些信息 
struct HidDescriptor
{
    /// 设备的VID
    ushort Vid;
    /// 设备的PID
    ushort Pid;
    /// 引用页Id
    ushort Usage;
    /// 引用页面值 
    ushort UsagePage;
    /// 产品版本号 
    ushort VersionNumber;
    /// 设备的输入报告长度 
    ushort InputLength;
    /// 设备的输出报告长度 
    ushort OutputLength;
    /// 设备描述路径
    string Path;
    /// 设备产品名称 
    string Name;
    /// 设备制造商 
    string Manufacturer;
    /// 标识序列号 
    string SerialNumber;
};
//--------------------------------------------------------- 
/// HID Interrupt通信句柄读取器 
class HidInterruptHandlerReader : public AsyncFileHandlerReader
{
protected:
    /// HID数据包长度 
    size_t _inputLen;
public:
    HidInterruptHandlerReader(const HidHandler& handle) : AsyncFileHandlerReader(handle)
    {
        _inputLen = handle.InputLength;
    }
    virtual bool Async(uint timeoutMs)
    {
        return AsyncFileHandlerReader::_AsyncRead(_inputLen, timeoutMs);
    }
};
/// HID Interrupt通信句柄写入器 
typedef AsyncFileHandlerWriter HidInterruptHandlerWriter;
//--------------------------------------------------------- 
/// HID Report通信句柄读取器 
class HidControlFileHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<HidHandler> _handle;
public:
    HidControlFileHandlerReader(const HidHandler& handle)
    {
        _handle = handle;
    }
    /// 开始读数据 
    virtual bool Async(uint) { return true; }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        // 直接使用data作为输出缓冲区 
        size_t lastlen = data.GetLength();
        data.Append(static_cast<byte>(0x00), _handle->InputLength);
        byte* pBuf = const_cast<byte*>(data.GetBuffer(lastlen));
        bool bRead = Tobool(HidD_GetInputReport(_handle->Handle, reinterpret_cast<PVOID>(pBuf), _handle->InputLength));
        // 截掉一字节的ReportID
        if(bRead && ByteConvert::OrVal(ByteArray(pBuf + 1, _handle->InputLength - 1)) != static_cast<byte>(0x00))
            return _handle->InputLength;
        
        // 读取失败 
        data.RemoveTail(_handle->InputLength);
        return 0;
    }
};
/// HID Report通信句柄写入器 
class HidControlFileHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<HidHandler> _handle;
    /// 待写入的数据 
    ByteArray _data;
public:
    HidControlFileHandlerWriter(const HidHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint)
    {
        _data = data;
        return true;
    }
    /// 写数据 
    virtual size_t Wait()
    {
        bool isWrite = Tobool(HidD_SetOutputReport(_handle->Handle, const_cast<byte*>(_data.GetBuffer()), _data.GetLength()));
        return isWrite ? _data.GetLength() : 0;
    }
};
//--------------------------------------------------------- 
/// HID Feature通信句柄读取器 
class HidFeatureFileHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<HidHandler> _handle;
public:
    HidFeatureFileHandlerReader(const HidHandler& handle)
    {
        _handle = handle;
    }
    /// 开始读数据 
    virtual bool Async(uint) { return true; }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        // 直接使用data作为输出缓冲区 
        size_t lastlen = data.GetLength();
        data.Append(static_cast<byte>(0x00), _handle->InputLength);
        byte* pBuf = const_cast<byte*>(data.GetBuffer(lastlen));
        bool bRead = Tobool(HidD_GetFeature(_handle->Handle, reinterpret_cast<PVOID>(pBuf), _handle->InputLength));
        // 截掉一字节的ReportID
        if(bRead && ByteConvert::OrVal(ByteArray(pBuf + 1, _handle->InputLength - 1)) != static_cast<byte>(0x00))
            return _handle->InputLength;

        // 读取失败 
        data.RemoveTail(_handle->InputLength);
        return 0;
    }
};
/// HID Feature通信句柄写入器 
class HidFeatureFileHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<HidHandler> _handle;
    /// 待写入的数据 
    ByteArray _data;
public:
    HidFeatureFileHandlerWriter(const HidHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint)
    {
        _data = data;
        return true;
    }
    /// 写数据 
    virtual size_t Wait()
    {
        bool isWrite = Tobool(HidD_SetFeature(_handle->Handle, const_cast<byte*>(_data.GetBuffer()), _data.GetLength()));
        return isWrite ? _data.GetLength() : 0;
    }
};
//--------------------------------------------------------- 
/// HID相关操作功能扩展器 
template<class THidHandlerDevice>
class HidHandlerAppender :
    public THidHandlerDevice,
    public IInteractiveTrans
{
public:
    //----------------------------------------------------- 
    /// 设备类型定义
    typedef HidDescriptor device_info;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 临时缓冲区
    ByteBuilder _tmpBuff;
    /// 获取设备描述信息 
    BOOL _GetHidInformation(HANDLE hDev, device_info& devInfo, bool isStrDesc)
    {
        /* 获取VID PID */
        HIDD_ATTRIBUTES hidAttributes;
        if(!HidD_GetAttributes(hDev, &hidAttributes))
        {
            _logErr(DeviceError::DevNotExistErr, "HidD_GetAttributes失败");
            return FALSE;
        }

        LOGGER(_log << "VID:<" << _hex(hidAttributes.VendorID)
            << ">,PID:<" << _hex(hidAttributes.ProductID)
            << ">\nVersionNumber:<" << _hex(hidAttributes.VersionNumber) << ">\n");

        devInfo.Vid = hidAttributes.VendorID;
        devInfo.Pid = hidAttributes.ProductID;
        devInfo.VersionNumber = hidAttributes.VersionNumber;

        /* 获取输入输出报告包长度 */
        PHIDP_PREPARSED_DATA hidPreparsedData;
        HIDP_CAPS capab;
        if(!HidD_GetPreparsedData(hDev, &hidPreparsedData))
        {
            _logErr(DeviceError::DevInitErr, "HidD_GetPreparsedData失败");
            return FALSE;
        }

        BOOL bResult = FALSE;
        if(HidP_GetCaps(hidPreparsedData, &capab) == HIDP_STATUS_SUCCESS)
        {
            bResult = TRUE;

            devInfo.InputLength = capab.InputReportByteLength;
            devInfo.OutputLength = capab.OutputReportByteLength;
            devInfo.Usage = capab.Usage;
            devInfo.UsagePage = capab.UsagePage;

            LOGGER(_log << "--设备信息(" << static_cast<void*>(hDev) << ")--\n"
                << "UsagePage:<" << _hex(capab.UsagePage) << ">,Usage:<" << _hex(capab.Usage) << ">\n"
                << "InputReportByteLength:<" << capab.InputReportByteLength << ">\n"
                << "OutputReportByteLength:<" << capab.OutputReportByteLength << ">\n"
                << "FeatureReportByteLength:<" << capab.FeatureReportByteLength << ">\n"
                << "NumberLinkCollectionNodes:<" << capab.NumberLinkCollectionNodes << ">\n"
                << "NumberInputButtonCaps:<" << capab.NumberInputButtonCaps << ">\n"
                << "NumberInputValueCaps:<" << capab.NumberInputValueCaps << ">\n"
                << "NumberOutputButtonCaps:<" << capab.NumberOutputButtonCaps << ">\n"
                << "NumberOutputValueCaps:<" << capab.NumberOutputValueCaps << ">\n"
                << "NumberFeatureButtonCaps:<" << capab.NumberFeatureButtonCaps << ">\n"
                << "NumberFeatureValueCaps:<" << capab.NumberFeatureValueCaps << ">\n"
                << "NumberFeatureValueCaps:<" << capab.NumberFeatureValueCaps << ">\n\n");
        }

        HidD_FreePreparsedData(hidPreparsedData);

        // 不需要获取字符串描述信息
        if(!isStrDesc)
            return bResult;

        /* 获取产品相关信息 */
        wchar_t tmp[256] = { 0 };
        CharConverter cvt;
        // 设备名称 
        if(HidD_GetProductString(hDev, tmp, sizeof(tmp)))
        {
            // 转换失败 
            devInfo.Name = cvt.to_char(tmp);
            LOGGER(_log << "产品名称:<" << devInfo.Name << ">\n");
        }
        else
        {
            _logErr(DeviceError::DevInitErr, "HidD_GetProductString失败");
        }
        // 厂商名称 
        if(HidD_GetManufacturerString(hDev, tmp, sizeof(tmp)))
        {
            devInfo.Manufacturer = cvt.to_char(tmp);
            LOGGER(_log << "厂商名称:<" << devInfo.Manufacturer << ">\n");
        }
        else
        {
            _logErr(DeviceError::DevInitErr, "HidD_GetProductString失败");
        }
        // 设备序列号标识  
        if(HidD_GetSerialNumberString(hDev, tmp, sizeof(tmp)))
        {
            devInfo.SerialNumber = cvt.to_char(tmp);
            LOGGER(_log << "序列号标识名称:<" << devInfo.SerialNumber << ">\n\n");
        }
        else
        {
            _logErr(DeviceError::DevInitErr, "HidD_GetProductString失败");
        }

        return bResult;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    HidHandlerAppender() : THidHandlerDevice(){ ReportId = 0x00; }
    //----------------------------------------------------- 
    /**
     * @brief 获取当前所有的HID设备名
     * @param [out] _list 获取到的设备名列表
     * @param [in] isStrDesc 是否获取
     * @return 获取到的HID设备数目
     */
    size_t EnumDevice(list<device_info>& _list, bool isStrDesc = true)
    {
        /* Log Header */
        LOG_FUNC_NAME();

        size_t devCount = 0;
        HANDLE hHid = INVALID_HANDLE_VALUE;
        GUID hidGuid;

        HidD_GetHidGuid(&hidGuid);

        LOGGER(_log << "Guid:<" << _hex(hidGuid.Data1) << '-' << _hex(hidGuid.Data2) << '-'
            << _hex(hidGuid.Data3) << '-';
        ByteBuilder tmpBuff(18);
        ByteConvert::ToAscii(ByteArray(hidGuid.Data4, 2), tmpBuff);
        tmpBuff += "-";
        ByteConvert::ToAscii(ByteArray(hidGuid.Data4 + 2, 6), tmpBuff);
        _log << tmpBuff.GetString() << ">\n");

        // SetupDiGetClassDevs 
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

        LOGGER(_log << "SetupDiGetClassDevs返回句柄:<" << _hex_num(hDevInfo) << ">\n");

        if(hDevInfo == INVALID_HANDLE_VALUE)
        {
            _logErr(DeviceError::DevInitErr, "SetupDiGetClassDevs返回句柄无效");
            return _logRetValue(devCount);
        }

        SP_DEVICE_INTERFACE_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        int deviceNo = 0;
        SetLastError(NO_ERROR);

        LOGGER(_log.WriteLine("开始查找HID设备:"));
        while(GetLastError() != ERROR_NO_MORE_ITEMS)
        {
            LOGGER(_log << "查找过程中的设备序号deviceNo:<" << deviceNo << ">\n");
            /* SetupDiEnumInterfaceDevice */
            if(SetupDiEnumInterfaceDevice(hDevInfo, 0, &hidGuid, deviceNo++, &devInfoData))
            {
                ULONG requiredLength = 0;
                SetupDiGetInterfaceDeviceDetail(hDevInfo, &devInfoData, NULL, 0, &requiredLength, NULL);
                PSP_INTERFACE_DEVICE_DETAIL_DATA pDevDetail = reinterpret_cast<SP_INTERFACE_DEVICE_DETAIL_DATA*>(malloc(requiredLength));
                pDevDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

                BOOL bRet = SetupDiGetInterfaceDeviceDetail(hDevInfo, &devInfoData,
                    pDevDetail, requiredLength, NULL, NULL);
                if(!bRet)
                {
                    _logErr(DeviceError::DevNotExistErr, "SetupDiGetInterfaceDeviceDetail失败");
                    free(pDevDetail);
                    break;
                }
                LOGGER(_log << "CreateFile:<" << pDevDetail->DevicePath << ">\n");

                hHid = CreateFile(pDevDetail->DevicePath,
                    0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

                if(!WinHandler::IsValidHandle(hHid))
                {
                    _logErr(DeviceError::DevNotExistErr, "CreateFile失败");

                    free(pDevDetail);
                    continue;
                }
                /* 获取设备相关属性信息 */
                _list.push_back();
                CharConverter cvt;
                _list.back().Path = cvt.to_char(pDevDetail->DevicePath);
                ++devCount;

                free(pDevDetail);
                // 输入输出报告必须获取到，否则无法通信 
                if(_GetHidInformation(hHid, _list.back(), isStrDesc) == FALSE)
                {
                    --devCount;
                    _list.pop_back();
                }
                WinHandler::Close(hHid);
            }
            LOGGER(
            else
            {
                _log.WriteLine("SetupDiEnumInterfaceDevice 枚举设备结束");
            });
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return _logRetValue(devCount);
    }
    //----------------------------------------------------- 
    /// 发送一个包长度的数据 
    virtual bool Write(const ByteArray& data)
    {
        _tmpBuff.Clear();
        _tmpBuff += ReportId;
        _tmpBuff += data;
        ByteConvert::Fill(_tmpBuff, _hDev.OutputLength, true, 0x00);
        return THidHandlerDevice::Write(_tmpBuff);
    }
    /// 接收一个包长度的数据 
    virtual bool Read(ByteBuilder& data)
    {
        _tmpBuff.Clear();
        bool bRead = THidHandlerDevice::Read(_tmpBuff);
        if(bRead)
        {
            // 过滤第一个字节的 ReportId
            data.Append(_tmpBuff.SubArray(1));
        }
        return bRead;
    }
    //----------------------------------------------------- 
    /// 获取HID发送的有效数据包大小
    inline size_t GetSendLength() const
    {
        return _hDev.OutputLength - 1;
    }
    /// 获取HID接收的有效数据包大小
    inline size_t GetRecvLength() const
    {
        return _hDev.InputLength - 1;
    }
    //----------------------------------------------------- 
    /// 报告ID 
    byte ReportId;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 自动识别协议类型的HID设备 
class HidProtocolHandlerDevice : public WinHandlerBaseDevice<HidHandler, HidHandlerFactory>
{
public:
    //----------------------------------------------------- 
    /// 数据传输方式 
    enum TransmitMode
    {
        /// 中断传输 
        InterruptTransmit = 0,
        /// 控制传输 
        ControlTransmit,
        /// 特征传输 
        FeatureTransmit
    };
    /// 将字符形式的传输方式转换为枚举
    static TransmitMode StringToMode(const char* str)
    {
        ByteArray modeArray(str);
        if(StringConvert::Compare("Interrupt", modeArray, true))
            return InterruptTransmit;
        if(StringConvert::Compare("Control", modeArray, true))
            return ControlTransmit;
        if(StringConvert::Compare("Feature", modeArray, true))
            return FeatureTransmit;
        return ControlTransmit;
    }
    /// 将传输方式转换为字符串描述
    static const char* ModeToString(TransmitMode mode)
    {
        switch(mode)
        {
        case InterruptTransmit:
            return "Interrupt";
        case ControlTransmit:
            return "Control";
        case FeatureTransmit:
            return "Feature";
        }
        return "Unknown";
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 设备数据传输方式 
    TransmitMode _transmitMode;
    /// 初始化 
    inline void _init()
    {
        _transmitMode = ControlTransmit;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    HidProtocolHandlerDevice() : WinHandlerBaseDevice() { _init(); }
    //----------------------------------------------------- 
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        bool bRead = false;
        if(_transmitMode == InterruptTransmit)
        {
            HidInterruptHandlerReader interruptHandlerReader(_hDev);
            HandlerEasyReader interruptEasyReader(interruptHandlerReader);
            bRead = WinHandlerBaseDevice::Read(interruptEasyReader, data);
        }
        else if(_transmitMode == ControlTransmit)
        {
            HidControlFileHandlerReader reportHandlerReader(_hDev);
            HandlerEasyReader reportEasyReader(reportHandlerReader);
            bRead = WinHandlerBaseDevice::Read(reportEasyReader, data);
        }
        return bRead;
    }
    /// 写数据 
    virtual bool Write(const ByteArray& data)
    {
        bool bWrite = false;
        if(_transmitMode == InterruptTransmit)
        {
            HidInterruptHandlerWriter interruptHandlerWriter(_hDev);
            bWrite = WinHandlerBaseDevice::Write(interruptHandlerWriter, data);
        }
        else if(_transmitMode == ControlTransmit)
        {
            HidControlFileHandlerWriter reportHandlerWriter(_hDev);
            bWrite = WinHandlerBaseDevice::Write(reportHandlerWriter, data);
        }
        return bWrite;
    }
    /**
     * @brief 设置设备数据传输方式
     *
     * 如果设备采用控制端点，则读写数据需要用DDK中的API来操作，
     * 设备为中断端点时需要用ReadFile,WriteFile来操作
     */
    TransmitMode SetTransmitMode(TransmitMode mode)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "设置端点传输方式:<" << static_cast<int>(mode) << ">\n");
        TransmitMode last = _transmitMode;
        _transmitMode = mode;

        return _logRetValue(last);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// HID通信方式的设备(只支持控制端点的方式传输数据) 
typedef HidHandlerAppender<
    HandlerDevice<
        WinHandlerBaseDevice<HidHandler, HidHandlerFactory>,
            HidInterruptHandlerReader, 
            HidInterruptHandlerWriter>
    > HidInterruptDevice;
/// HID通信方式的设备(只支持中断端点的方式传输数据)
typedef HidHandlerAppender<
    HandlerDevice<
        WinHandlerBaseDevice<HidHandler, HidHandlerFactory>,
        HidControlFileHandlerReader,
        HidControlFileHandlerWriter>
    > HidReportDevice;
/// HID通信方式的设备(只支持Feature方式传输数据)
typedef HidHandlerAppender<
    HandlerDevice<
        WinHandlerBaseDevice<HidHandler, HidHandlerFactory>,
        HidFeatureFileHandlerReader,
        HidFeatureFileHandlerWriter>
    > HidFeatureDevice;
/// HID通信方式的设备(自动识别端点类型)
typedef HidHandlerAppender<HidProtocolHandlerDevice> HidDevice;
//--------------------------------------------------------- 
} // namesapce env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#pragma warning(default:4996)
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDDEVICE_H_
//========================================================= 
