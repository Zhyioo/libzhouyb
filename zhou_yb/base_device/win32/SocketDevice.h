//========================================================= 
/**@file SocketDevice.h
 * @brief Windows下套接字通信设备 
 * 
 * @date 2015-07-17   21:31:37
 * @author Zhyioo 
 * @version 1.0
 */
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SOCKETDEVICE_H_
#define _LIBZHOUYB_SOCKETDEVICE_H_
//--------------------------------------------------------- 
// 在某些项目中可能需要单独定义该宏  
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Winsock2.h>
#include <strsafe.h>
#include <intsafe.h>

#pragma comment(lib, "Ws2_32.lib")

#include "../../include/Base.h"
#include "WinHandler.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// socket句柄 
typedef Handler<SOCKET> SocketHandler;
/// socket句柄(异步)
struct AsyncSocketHandler : public SocketHandler
{
    /// 信号 
    WSAEVENT SocketEvent;

    AsyncSocketHandler() : SocketHandler() { SocketEvent = INVALID_HANDLE_VALUE; }
    /// 等待信号 
    bool WaitForEvent(DWORD dwEvent, DWORD dwIndex, uint timeoutMs)
    {
        // 立即返回 
        DWORD dwRet = WSAWaitForMultipleEvents(1, &SocketEvent, TRUE, timeoutMs, FALSE);
        if(dwRet == WSA_WAIT_FAILED)
            return false;

        WSANETWORKEVENTS tNetEvents;
        if(WSAEnumNetworkEvents(Handle, SocketEvent, &tNetEvents) == SOCKET_ERROR)
            return false;

        // 等到事件 
        if((tNetEvents.lNetworkEvents & dwEvent) && (tNetEvents.iErrorCode[dwIndex] == ERROR_SUCCESS))
            return true;

        return false;
    }
};
//--------------------------------------------------------- 
/// socket工厂类 
class SocketHandlerFactory : 
    public IFactory<SocketHandler>, 
    public IFactory<AsyncSocketHandler>,
    public LoggerBehavior
{
protected:
    //----------------------------------------------------- 
    /// WSA启动标志 
    static bool _isWsaStarted;
    /// 成员变量引用计数 
    static size_t _useCount;
    /// 解析Socket参数
    bool _parse_socket(const char* sArg, int& af, int& type, int& protocol)
    {
        if(_is_empty_or_null(sArg))
            return false;
        ArgParser cfg;
        if(cfg.Parse(sArg) < 1)
            return false;

        ArgConvert::FromConfig<int>(cfg, SocketAfKey, af);
        ArgConvert::FromConfig<int>(cfg, SocketTypeKey, type);
        ArgConvert::FromConfig<int>(cfg, SocketProtocolKey, protocol);

        return true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// SocketHandlerFactory生成参数配置项时的主键 SocketAfKey
    static const char SocketAfKey[16];
    /// SocketHandlerFactory生成参数配置项时的主键 SocketTypeKey
    static const char SocketTypeKey[16];
    /// SocketHandlerFactory生成参数配置项时的主键 SocketProtocolKey
    static const char SocketProtocolKey[16];
    //----------------------------------------------------- 
    /// 启动WSA 
    static bool WsaStart()
    {
        if(_isWsaStarted)
            return true;

        WSADATA WSAData = { 0 };
        _isWsaStarted = (WSAStartup(MAKEWORD(2, 2), &WSAData) == ERROR_SUCCESS);
        return _isWsaStarted;
    }
    /// 关闭WSA 
    static void WsaEnd()
    {
        if(_isWsaStarted)
        {
            WSACleanup();
            _isWsaStarted = false;
        }
    }
    /// 生成参数 
    static string ToArg(int af, int type, int protocol)
    {
        string sArg;
        sArg += ArgConvert::ToConfig<int>(SocketAfKey, af);
        sArg += ArgConvert::ToConfig<int>(SocketTypeKey, type);
        sArg += ArgConvert::ToConfig<int>(SocketProtocolKey, protocol);

        return sArg;
    }
    //----------------------------------------------------- 
    /// 创建socket 
    virtual bool Create(SocketHandler& obj, const char* sArg = NULL)
    {
        int af = 0;
        int type = 0;
        int protocol = 0;
        LOGGER(_log.WriteLine("ParseArg..."));
        if(!_parse_socket(sArg, af, type, protocol))
            return false;

        return Create(obj, af, type, protocol);
    }
    /// 创建socket
    bool Create(SocketHandler& obj, int af, int type, int protocol)
    {
        LOGGER(_log << "Af:<" << af << ">,Type:<" << type << ">,Protocol:<" << protocol << ">\n");

        if(!SocketHandlerFactory::WsaStart())
        {
            LOGGER(_log.WriteLine("WSAStartup失败"));
            return false;
        }

        ++SocketHandlerFactory::_useCount;

        obj.Handle = socket(af, type, protocol);
        return IsValid(obj);
    }
    /// 返回句柄是否有效 
    virtual bool IsValid(const SocketHandler& obj)
    {
        return obj.Handle != INVALID_SOCKET;
    }
    /// 关闭socket
    virtual void Dispose(SocketHandler& obj)
    {
        closesocket(obj.Handle);
        obj.Handle = INVALID_SOCKET;

        // 最后一个对象负责释放WSA 
        --(SocketHandlerFactory::_useCount);
        if(SocketHandlerFactory::_useCount < 1)
        {
            SocketHandlerFactory::WsaEnd();
        }
    }
    //----------------------------------------------------- 
    /// 创建socket(异步)
    virtual bool Create(AsyncSocketHandler& obj, const char* sArg = NULL)
    {
        int af = 0;
        int type = 0;
        int protocol = 0;
        LOGGER(_log.WriteLine("ParseArg..."));
        if(!_parse_socket(sArg, af, type, protocol))
            return false;

        return Create(obj, af, type, protocol);
    }
    /// 创建socket(异步)
    bool Create(AsyncSocketHandler& obj, int af, int type, int protocol, 
        DWORD events = FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE)
    {
        SocketHandler& handle = obj;
        bool bCreate = Create(handle, af, type, protocol);
        if(bCreate)
        {
            LOGGER(_log.WriteLine("WSACreateEvent..."));
            bCreate = false;
            obj.SocketEvent = WSACreateEvent();
            if(WinHandler::IsValidHandle(obj.SocketEvent))
            {
                LOGGER(_log.WriteLine("WSAEventSelect..."));
                if(WSAEventSelect(obj.Handle, obj.SocketEvent, events) != SOCKET_ERROR)
                {
                    bCreate = true;
                }
            }
            if(!bCreate)
            {
                LOGGER(_log.WriteLine("创建Socket事件失败"));
                Dispose(obj);
            }
        }
        return bCreate;
    }
    /// 返回句柄是否有效(异步)
    virtual bool IsValid(const AsyncSocketHandler& obj)
    {
        const SocketHandler& handle = obj;
        return obj.SocketEvent != INVALID_HANDLE_VALUE && IsValid(handle);
    }
    /// 关闭socket(异步)
    virtual void Dispose(AsyncSocketHandler& obj)
    {
        LOGGER(_log.WriteLine("WSAEventSelect..."));
        WSAEventSelect(obj.Handle, obj.SocketEvent, 0);
        LOGGER(_log.WriteLine("WSACloseEvent..."));
        WSACloseEvent(obj.SocketEvent);
        obj.SocketEvent = INVALID_HANDLE_VALUE;

        SocketHandler& handle = obj;
        Dispose(handle);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 同步socket读取器 
class SocketHandlerReader : public IHandlerReader
{
protected:
    /// 句柄 
    Ref<SocketHandler> _handle;
    /// 读取数据 
    size_t _AsyncRead(ByteBuilder& data, size_t rlen)
    {
        /// 接收缓冲区 
        char _buff[DEV_BUFFER_SIZE];
        int iRet = recv(_handle->Handle, _buff, _min(rlen, DEV_BUFFER_SIZE), 0);
        if(iRet == SOCKET_ERROR || iRet <= 0)
            return 0;
        size_t len = static_cast<size_t>(iRet);
        data.Append(ByteArray(_buff, len));
        return len;
    }
public:
    SocketHandlerReader(const SocketHandler& handle) { _handle = handle; }
    /// 开始读 
    virtual bool Async(uint) { return true; }
    /// 等待读取数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        return _AsyncRead(data, DEV_BUFFER_SIZE);
    }
};
/// 同步socket写入器 
class SocketHandlerWriter : public IHandlerWriter
{
protected:
    /// 句柄
    Ref<SocketHandler> _handle;
    /// 待发送的数据 
    ByteArray _data;
public:
    SocketHandlerWriter(const SocketHandler& handle) { _handle = handle; }
    virtual bool Async(const ByteArray& data, uint)
    {
        _data = data;
        return true;
    }
    virtual size_t Wait()
    {
        int iRet = send(_handle->Handle, _data.GetString(), _data.GetLength(), 0);
        if(iRet == SOCKET_ERROR)
            return 0;
        return _data.GetLength();
    }
};
//--------------------------------------------------------- 
/// 异步socket读取器 
class AsyncSocketHanderReader : public SocketHandlerReader
{
protected:
    Ref<AsyncSocketHandler> _handle;
    uint _timeoutMs;
public:
    AsyncSocketHanderReader(const AsyncSocketHandler& handle) : SocketHandlerReader(handle) { _handle = handle; }
    virtual bool Async(uint timeoutMs)
    {
        _timeoutMs = timeoutMs;
        return SocketHandlerReader::Async(timeoutMs);
    }
    virtual size_t Wait(ByteBuilder& data)
    {
        if(!_handle->WaitForEvent(FD_READ, FD_READ_BIT, _timeoutMs))
            return 0;
        return SocketHandlerReader::Wait(data);
    }
};
//--------------------------------------------------------- 
/// 同步socket设备
typedef HandlerDevice<HandlerBaseDevice<SocketHandler, SocketHandlerFactory>, SocketHandlerReader, SocketHandlerWriter> SocketHandlerDevice;
/// 同步socket设备
class SocketDevice : public SocketHandlerDevice
{
protected:
    /// 记录错误信息
    virtual void _logErr(int errCode, const char* errinfo = NULL)
    {
        DWORD errId = WSAGetLastError();
        string msg = _strput(errinfo);
        if(msg.length() > 0)
            msg += " ";
        msg += ArgConvert::ToString(errId);
        msg += ",";
        msg += WinLastErrBehavior::TransErrToString(errId);
        DeviceBehavior::_logErr(errCode, msg.c_str());
    }
public:
    SocketDevice() : SocketHandlerDevice() {}

    /// 连接 
    virtual bool Connect(const sockaddr* name, int namelen)
    {
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(IsOpen());

        if(connect(_hDev.Handle, name, namelen) == SOCKET_ERROR)
        {
            _logErr(DeviceError::DevConnectErr, "connect失败");
            return _logRetValue(false);
        }

        return _logRetValue(true);
    }
    /// 绑定 
    virtual bool Bind(const sockaddr* name, int namelen)
    {
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(IsOpen());

        if(bind(_hDev.Handle, name, namelen) == SOCKET_ERROR)
        {
            _logErr(DeviceError::DevConnectErr, "bind失败");
            return _logRetValue(false);
        }

        return _logRetValue(true);
    }
    /// 监听 
    virtual bool Listen(int backlog)
    {
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(IsOpen());

        if(listen(_hDev.Handle, backlog) == SOCKET_ERROR)
        {
            _logErr(DeviceError::DevConnectErr, "listen失败");
            return _logRetValue(false);
        }

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
/// 异步socket设备
typedef HandlerDevice<HandlerBaseDevice<AsyncSocketHandler, SocketHandlerFactory>, AsyncSocketHanderReader, SocketHandlerWriter> AsyncSocketHandlerDevice;
/// 异步socket设备 
class AsyncSocketDevice : public AsyncSocketHandlerDevice
{
protected:
    /// 等待事件 
    bool _WaitForEvent(DWORD dwEvent, DWORD dwIndex)
    {
        LOG_FUNC_NAME();
        Timer timer;
        while(timer.Elapsed() < _waitTimeout)
        {
            if(_hDev.WaitForEvent(dwEvent, dwIndex, _waitInterval))
                return true;

            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                _logErr(DeviceError::OperatorInterruptErr, "等待事件被中断");
                return false;
            }
            Timer::Wait(_waitInterval);
        }
        _logErr(DeviceError::WaitTimeOutErr, "等待事件超时");
        return false;
    }
    /// 记录错误信息 
    virtual void _logErr(int errCode, const char* errinfo = NULL)
    {
        DWORD errId = WSAGetLastError();
        string msg = _strput(errinfo);
        if(msg.length() > 0)
            msg += " ";
        msg += ArgConvert::ToString(errId);
        msg += ",";
        msg += WinLastErrBehavior::TransErrToString(errId);
        DeviceBehavior::_logErr(errCode, msg.c_str());
    }
public:
    AsyncSocketDevice() : AsyncSocketHandlerDevice() {}
    /// 连接 
    virtual bool Connect(const sockaddr* name, int namelen)
    {
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(IsOpen());

        connect(_hDev.Handle, name, namelen);
        return _logRetValue(_WaitForEvent(FD_CONNECT, FD_CONNECT_BIT));
    }
};
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SOCKETDEVICE_H_
//========================================================= 