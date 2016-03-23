//========================================================= 
/**@file ComDevice.h 
 * @brief Windows串口设备
 * 
 * @date 2011-12-17   15:31:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "FileDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// 串口句柄工厂类 
class ComHandlerFactory : public FileHandlerFactory
{
public:
    /// 将串口号转为Open.sArg参数 
    static string ToArg(uint port)
    {
        char portName[32] = { 0 };
        // 串口号只占一位
        if(port < 10)
            sprintf(portName, "COM%d", port);
        else
            sprintf(portName, "\\\\.\\COM%d", port);
        return string(portName);
    }
};
/// 串口属性相关操作功能的扩展类 
template<class THandlerDevice>
class ComHandlerAppender : public THandlerDevice
{
public:
    //----------------------------------------------------- 
    /** 
     * @brief 获取所有的串口(从注册表中获取) 
     * @param [out] _list 获取到的串口号 
     * @retval -1 获取过程出现错误 
     * @retval 其他 获取到的HID设备数目 
     */ 
    int EnumDevice(list<uint>& _list)
    {
        LOG_FUNC_NAME();

        int comCount = -1;
        
        long  lReg; 
        HKEY  hKey;
        DWORD  MaxValueLength;
        DWORD  dwValueNumber;

        lReg = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_QUERY_VALUE, &hKey);
        if(lReg != ERROR_SUCCESS)
        {
            _logErr(DeviceError::OperatorErr, "RegOpenKeyEx 打开注册表失败");
            return _logRetValue(comCount);
        }

        lReg = RegQueryInfoKey(hKey, NULL, NULL ,NULL ,NULL, NULL, NULL, 
            &dwValueNumber, &MaxValueLength, NULL, NULL, NULL);
        if(lReg != ERROR_SUCCESS)
        {
            _logErr(DeviceError::OperatorErr, "RegQueryInfoKey 获取注册表信息失败");
            return _logRetValue(comCount);
        }

        char_t valBuff[128];
        char_t comBuff[8];
        comCount = 0;

        DWORD cchValueName = 0;
        DWORD dwValueSize = sizeof(comBuff) * sizeof(char_t);
        for(DWORD i = 0;i < dwValueNumber; ++i)
        {
            cchValueName = MaxValueLength + 1;
            dwValueSize = sizeof(comBuff) * sizeof(char_t);

            memset(valBuff, 0, sizeof(valBuff));

            lReg = RegEnumValue(hKey, i, reinterpret_cast<LPTSTR>(valBuff), &cchValueName, NULL, NULL, NULL, NULL);
            if(lReg != ERROR_SUCCESS && lReg != ERROR_NO_MORE_ITEMS)
                continue;

            memset(comBuff, 0, sizeof(comBuff));

            lReg = RegQueryValueEx(hKey, reinterpret_cast<LPTSTR>(valBuff), NULL, NULL, reinterpret_cast<LPBYTE>(comBuff), &dwValueSize);
            if(lReg != ERROR_SUCCESS)
                continue;

            lReg = strlen_t(comBuff);
            CharConverter cvt;
            const char* pComBuff = cvt.to_char(comBuff);
            if(lReg > 3 && StringConvert::StartWith(ByteArray(pComBuff, lReg), "COM"))
            {
                ++comCount;
                _list.push_back(ArgConvert::FromString<uint>(pComBuff + 3));
            }
        }

        LOGGER(_log<<"注册表中枚举到的串口,数目<"<<comCount<<">:\n";
        list<uint>::iterator itr;
        for(itr = _list.begin();itr != _list.end(); ++itr)
        {
            _log<<"COM:<"<<*itr<<">\n";
        });

        return _logRetValue(comCount);
    }
    /**
     * @brief 打开串口 
     * @param [in] port 需要打开的串口号 
     */ 
    bool Open(uint port)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        LOGGER(_log<<"串口号:<"<<port<<">\n");
        
        string portName = ComHandlerFactory::ToArg(port);
        return _logRetValue(THandlerDevice::Open(portName.c_str()));
    }
    /** 
     * @brief 设置串口属性 
     * @param [in] baud 波特率 
     * @param [in] byteSize [default:8] 字节大小(默认为8位)
     * @param [in] parity [default:NOPARITY] 校验(默认不校验)
     * @param [in] stopBits [default:ONESTOPBIT] 停止位(默认为1位停止位)
     */
    bool SetProperty(uint baud, byte byteSize = 8, byte parity = NOPARITY, byte stopBits = ONESTOPBIT)
    {
        /* Log Header */
        LOG_FUNC_NAME();

        LOGGER(_log.WriteLine("设置串口属性:");
        _log<<"波特率:<"<<baud<<">\n"
            <<"字节数:<"<<static_cast<uint>(byteSize)<<">\n"
            <<"校验:<"<<static_cast<uint>(parity)<<">\n"
            <<"停止位:<"<<static_cast<uint>(stopBits)<<">\n");

        ASSERT_DeviceValid(IsOpen());

        DCB dcb;
        if(GetCommState(_hDev.Handle, &dcb) != TRUE)
        {
            _logErr(DeviceError::OperatorErr, "GetCommState失败");
            return _logRetValue(false);
        }
    
        dcb.BaudRate = baud;
        dcb.ByteSize = byteSize;
        dcb.Parity = parity;
        dcb.StopBits = stopBits;
        if(SetCommState(_hDev.Handle, &dcb) != TRUE)
        {
            _logErr(DeviceError::OperatorErr, "SetCommState失败");
            return _logRetValue(false);
        }

        return _logRetValue(true);
    }
    /**
     * @brief 设置串口内部超时
     * 
     * @param [in] timeoutMs 读写的超时时间
     * @param [in] multiplier [default:0] 读写时间系数
     * @param [in] readInterval [default:DEV_OPERATOR_INTERVAL] 读两个连续字节之间的最大轮询间隔
     * 
     * @return uint 上一次的超时时间
     */
    uint SetComTimeout(uint timeoutMs, uint multiplier = 0, uint readInterval = DEV_OPERATOR_INTERVAL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(IsOpen());

        COMMTIMEOUTS commTimeOuts;
        GetCommTimeouts(_hDev.Handle, &commTimeOuts);
        uint lastTimeouts = commTimeOuts.WriteTotalTimeoutConstant;
        /* Log */
        LOGGER(_log<<"更改写超时:<"<<timeoutMs<<">,旧值为:<"<<lastTimeouts<<">\n");

        commTimeOuts.ReadIntervalTimeout = readInterval;
        commTimeOuts.ReadTotalTimeoutMultiplier = multiplier;
        commTimeOuts.ReadTotalTimeoutConstant = timeoutMs;
        commTimeOuts.WriteTotalTimeoutMultiplier = multiplier;
        commTimeOuts.WriteTotalTimeoutConstant = timeoutMs;
        SetCommTimeouts(_hDev.Handle, &commTimeOuts);

        return lastTimeouts;
    }
    /** 
     * @brief 设置串口FIFO缓冲区大小 
     * @param [in] inSize 输入缓冲区大小 
     * @param [in] outSize 输出缓冲区大小 
     */ 
    bool SetBufferSize(uint inSize, uint outSize)
    {
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(IsOpen());

        LOGGER(_log<<"输入缓冲区大小:<"<<inSize<<">,输出缓冲区大小:<"<<outSize<<">\n");

        bool bRet = (SetupComm(_hDev.Handle, static_cast<DWORD>(inSize), static_cast<DWORD>(outSize)) == TRUE);

        return _logRetValue(bRet);
    }
    /// 清除串口缓冲区 
    void Clean()
    {
        /* Log Header */
        LOG_FUNC_NAME();

        if(IsOpen())
        {
            LOGGER(_log.WriteLine("清空缓冲区"));
            // 清空输入输出缓冲区
            PurgeComm(_hDev.Handle, static_cast<DWORD>(PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT));
        }
        
        _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 默认的串口设备 
typedef ComHandlerAppender<
    HandlerDevice<
        WinHandlerBaseDevice<WinHandler, FileHandlerFactory>,
        FileHandlerReader,
        FileHandlerWriter>
    > ComDevice;
//--------------------------------------------------------- 
/// 异步串口数据读取器 
class AsyncComFileHandlerReader : public AsyncFileHandlerReader
{
public:
    AsyncComFileHandlerReader(const WinAsyncHandler& handle) : AsyncFileHandlerReader(handle) {}
    virtual bool Async(uint timeoutMs)
    {
        COMSTAT comStat;
        DWORD dwErrFlags = 0;

        SetObjZero(comStat);
        ClearCommError(_handle->Handle, &dwErrFlags, &comStat);

        return AsyncFileHandlerReader::_AsyncRead(comStat.cbInQue, timeoutMs);
    }
};
/// 异步串口设备
typedef ComHandlerAppender<
    HandlerDevice<
        WinHandlerBaseDevice<WinAsyncHandler, FileHandlerFactory>,
        AsyncComFileHandlerReader, 
        AsyncFileHandlerWriter>
    > AsyncComDevice;
//--------------------------------------------------------- 
/// 基于时间的串口句柄工厂类 
class EventComHandlerFactory : public ComHandlerFactory
{
public:
    bool Create(WinAsyncHandler& obj, const char* sArg = NULL)
    {
        if(!ComHandlerFactory::Create(obj, sArg))
            return false;
        bool bOpen = Tobool(SetCommMask(obj.Handle, EV_RXCHAR));
        if(!bOpen) ComHandlerFactory::Dispose(obj);
        return bOpen;
    }
};
/// 基于事件的串口数据读取器 
class ComEventAsyncFileHandlerReader : public IHandlerReader
{
protected:
    /// 写入句柄 
    Ref<WinAsyncHandler> _handle;
public:
    ComEventAsyncFileHandlerReader(const WinAsyncHandler& handle) { _handle = handle; }
    /// 开始读数据 
    virtual bool Async(uint) { return true; }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        DWORD evtMask;
        WinAsyncHandler::AsyncReset(_handle);
        // 读取失败 
        if(Tobool(WaitCommEvent(_handle->Handle, &evtMask, &(_handle->WaitOverlapped))))
        {
            // 成功接收到字符 
            if(evtMask == EV_RXCHAR)
            {
                data += static_cast<byte>(0x00);
                return 1;
            }
        }
        return 0;
    }
};
/// EventComDevice底层设备定义 
typedef ComHandlerAppender<
    HandlerDevice<
        WinHandlerBaseDevice<WinAsyncHandler, EventComHandlerFactory>, 
        ComEventAsyncFileHandlerReader, 
        AsyncFileHandlerWriter> 
    > EventComDevice;
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device
} // namespace zhou_yb
//=========================================================
