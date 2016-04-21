//========================================================= 
/**@file FileDevice.h 
 * @brief 基于文件形式的设备
 * 
 * @date 2011-12-16 18:45:06 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_FILEDEVICE_H_
#define _LIBZHOUYB_FILEDEVICE_H_
//--------------------------------------------------------- 
#include "WinHandler.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// 文件句柄工厂类
class FileHandlerFactory : 
    public IFactory<WinHandler>, 
    public IFactory<WinAsyncHandler>,
    public LoggerBehavior
{
protected:
    //----------------------------------------------------- 
    /// 解析设备路径 
    bool _parse_dev_path(const char* sArg, string& devPath)
    {
        if(NULL != sArg)
        {
            ArgParser cfg;
            cfg.Parse(sArg);
            string path;
            if(!ArgConvert::FromConfig<string>(cfg, FileHandlerFactory::PathKey, devPath))
                devPath = sArg;
        }
        return devPath.length() > 0;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 文件设备路径键值 
    static const char PathKey[10];
    //----------------------------------------------------- 
    /// 创建句柄(同步) 
    virtual bool Create(WinHandler& obj, const char* sArg = NULL)
    {
        string devPath;
        LOGGER(_log.WriteLine("ParseArg..."));
        if(!_parse_dev_path(sArg, devPath))
            return false;
        CharConverter cvt;
        LOGGER(_log.WriteLine("CreateFile..."));
        obj.Handle = CreateFile(cvt.to_char_t(devPath.c_str()),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        return IsValid(obj);
    }
    /// 返回对象是否有效(同步) 
    virtual bool IsValid(const WinHandler& obj)
    {
        return WinHandler::IsValidHandle(obj.Handle);
    }
    /// 销毁句柄(同步) 
    virtual void Dispose(WinHandler& obj)
    {
        WinHandler::Close(obj.Handle);
    }
    //----------------------------------------------------- 
    /// 创建句柄(异步)
    virtual bool Create(WinAsyncHandler& obj, const char* sArg = NULL)
    {
        string devPath;
        LOGGER(_log.WriteLine("ParseArg..."));
        if(!_parse_dev_path(sArg, devPath))
            return false;
        CharConverter cvt;
        LOGGER(_log.WriteLine("CreateFile..."));
        obj.Handle = CreateFile(cvt.to_char_t(devPath.c_str()),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL,
            NULL);
        WinHandler& hCom = obj;
        if(!IsValid(hCom))
            return false;
        
        LOGGER(_log.WriteLine("CreateEvent..."));
        HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(!WinHandler::IsValidHandle(hEvent))
        {
            WinHandler::Close(obj.Handle);
            LOGGER(_log.WriteLine("创建异步事件读句柄失败"));
            return false;
        }
        obj.WaitOverlapped.hEvent = hEvent;
        LOGGER(_log << "Event Handler:<" << _hex_num(obj.WaitOverlapped.hEvent) << ">\n");

        return true;
    }
    /// 返回对象是否有效(异步) 
    virtual bool IsValid(const WinAsyncHandler& obj)
    {
        return WinHandler::IsValidHandle(obj.Handle) &&
            WinHandler::IsValidHandle(obj.WaitOverlapped.hEvent);
    }
    /// 销毁句柄(异步) 
    virtual void Dispose(WinAsyncHandler& obj)
    {
        CancelIo(obj.Handle);
        WinHandler::Close(obj.Handle);
        WinHandler::Close(obj.WaitOverlapped.hEvent);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 文件句柄读取器(同步)
class FileHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<WinHandler> _handle;
    /// 读取指定大小的数据,返回实际读取到的数据长度 
    size_t _AsyncRead(ByteBuilder& data, size_t rlen)
    {
        DWORD readCount = 0;
        /// 读取数据缓冲区 
        byte buff[DEV_BUFFER_SIZE] = { 0 };
        if(Tobool(ReadFile(_handle->Handle, buff, _min(rlen, DEV_BUFFER_SIZE), &readCount, NULL)))
        {
            size_t recvLen = static_cast<size_t>(readCount);
            data.Append(ByteArray(buff, recvLen));
            return recvLen;
        }
        return 0;
    }
public:
    FileHandlerReader(const WinHandler& handle)
    {
        _handle = handle;
    }
    /// 开始读取数据 
    virtual bool Async(uint) { return true; }
    /// 读取数据
    virtual size_t Wait(ByteBuilder& data)
    {
        return _AsyncRead(data, DEV_BUFFER_SIZE);
    }
};
/// 文件句柄写入器(同步) 
class FileHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<WinHandler> _handle;
    /// 待写入的数据 
    ByteArray _data;
public:
    FileHandlerWriter(const WinHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint) 
    {
        _data = data;
        return true;
    }
    /// 写数据,返回成功写入的数据长度 
    virtual size_t Wait()
    {
        if(_data.IsEmpty())
            return SIZE_EOF;
        DWORD writeCount = 0;
        if(Tobool(WriteFile(_handle->Handle, _data.GetBuffer(), _data.GetLength(), &writeCount, NULL)))
        {
            // 移除已经发送的数据 
            _data = _data.SubArray(writeCount);
            return writeCount;
        }
        return 0;
    }
};
//--------------------------------------------------------- 
/// 文件句柄异步读接口 
class AsyncFileHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<WinAsyncHandler> _handle;
    /// 接收缓冲区 
    byte _buff[DEV_BUFFER_SIZE];
    /// 是否需要再次触发读取数据 
    bool _isAsyncWait;
    /// 超时时间
    uint _timeoutMs;
    /// 读取数据 
    bool _AsyncRead(size_t rlen, uint timeoutMs)
    {
        _timeoutMs = timeoutMs;
        
        DWORD readCount = 0;
        ResetEvent(_handle->WaitOverlapped.hEvent);
        SetArrayZero(_buff);
        bool bRead = Tobool(ReadFile(_handle->Handle, _buff, _min(rlen, DEV_BUFFER_SIZE), &readCount, &(_handle->WaitOverlapped)));
        if(!bRead && GetLastError() != ERROR_IO_PENDING)
            return false;
        _isAsyncWait = false;
        return true;
    }
public:
    AsyncFileHandlerReader(const WinAsyncHandler& handle) { _handle = handle; }
    /// 开始读数据 
    virtual bool Async(uint timeoutMs)
    {
        return _AsyncRead(DEV_BUFFER_SIZE, timeoutMs);
    }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        if(_isAsyncWait && !Async(_timeoutMs))
            return 0;
        DWORD readCount = 0;
        // 立即返回 
        DWORD dwRet = WaitForSingleObject(_handle->WaitOverlapped.hEvent, _timeoutMs);
        switch(dwRet)
        {
        case WAIT_OBJECT_0:
            if(Tobool(GetOverlappedResult(_handle->Handle, &(_handle->WaitOverlapped), &readCount, FALSE)))
            {
                if(readCount > 0)
                    data.Append(ByteArray(_buff, readCount));
                _isAsyncWait = true;
            }
            break;
        case WAIT_TIMEOUT:
            break;
        // 失败时取消IO 
        case WAIT_FAILED:
        default:
            CancelIo(_handle->Handle);
            _isAsyncWait = true;
            break;
        }
        return readCount;
    }
};
/// 文件句柄异步写接口 
class AsyncFileHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<WinAsyncHandler> _handle;
    /// 待发送的数据 
    ByteArray _data;
    /// 是否需要再次触发读取数据
    bool _isAsyncWait;
    /// 超时时间
    uint _timeoutMs;
public:
    AsyncFileHandlerWriter(const WinAsyncHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint timeoutMs)
    {
        _timeoutMs = timeoutMs;

        DWORD writeCount = 0;
        ResetEvent(_handle->WaitOverlapped.hEvent);
        bool bWrite = Tobool(WriteFile(_handle->Handle, data.GetBuffer(), data.GetLength(), &writeCount, &(_handle->WaitOverlapped)));
        if(!bWrite && GetLastError() != ERROR_IO_PENDING)
            return false;
        _data = data;
        _isAsyncWait = false;
        return true;
    }
    /// 写数据 
    virtual size_t Wait()
    {
        if(_isAsyncWait && !Async(_data, _timeoutMs))
            return 0;

        DWORD writeCount = 0;
        // 立即返回 
        DWORD dwRet = WaitForSingleObject(_handle->WaitOverlapped.hEvent, _timeoutMs);
        switch(dwRet)
        {
        case WAIT_OBJECT_0:
            if(Tobool(GetOverlappedResult(_handle->Handle, &(_handle->WaitOverlapped), &writeCount, FALSE)))
            {
                // 数据是否全部发送完毕 
                if(writeCount >= _data.GetLength())
                    return SIZE_EOF;
                // 继续发送剩余的数据
                _data = _data.SubArray(writeCount);
                _isAsyncWait = true;
            }
            break;
        case WAIT_TIMEOUT:
            break;
        // 失败时取消IO 
        case WAIT_FAILED:
        default:
            CancelIo(_handle->Handle);
            _isAsyncWait = true;
            break;
        }
        return writeCount;
    }
};
//--------------------------------------------------------- 
/// 同步文件设备 
typedef HandlerDevice<
    WinHandlerBaseDevice<WinHandler, FileHandlerFactory>,
    FileHandlerReader, 
    FileHandlerWriter> FileDevice;
/// 异步文件设备 
typedef HandlerDevice<
    WinHandlerBaseDevice<WinAsyncHandler, FileHandlerFactory>,
    AsyncFileHandlerReader, 
    AsyncFileHandlerWriter> AsyncFileDevice;
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_FILEDEVICE_H_
//========================================================= 
