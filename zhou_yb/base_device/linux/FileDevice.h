//========================================================= 
/**@file FileDevice.h 
 * @brief Linux下基于文件形式的设备(包括串口)
 * 
 * @date 2012-08-23   16:43:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_FILEDEVICE_H_
#define _LIBZHOUYB_FILEDEVICE_H_
//--------------------------------------------------------- 
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>

#include "../Handler.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// 无效文件句柄
#define INVALID_FD_VALUE (-1)
//--------------------------------------------------------- 
/// Linux下的文件句柄 
typedef Handler<int> FileHandler;
/// Linux下的文件句柄工厂类 
class FileHandlerFactory : public IFactory<FileHandler>, public LoggerBehavior
{
protected:
    //----------------------------------------------------- 
    /// 解析设备路径
    bool _parse_dev_path(const char* sArg, string& devPath, int& accessMode)
    {
        if(_is_empty_or_null(sArg))
            return false;
        
        ArgParser cfg;
        if(cfg.Parse(sArg))
        {
            if(!ArgConvert::FromConfig<string>(cfg, DevPathKey, devPath))
                devPath = sArg;

            ArgConvert::FromConfig<int>(cfg, AccessKey, accessMode);
        }
        else
        {
            devPath = sArg;
        }
        return devPath.length() > 0;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 打开设备的路径键值
    static const char DevPathKey[10];
    /// 打开设备权限的键值(int值)
    static const char AccessKey[10];
    //----------------------------------------------------- 
    /// 生成参数 
    static string ToArg(const char* fileName, int accessMode = O_RDWR)
    {
        string sArg;
        sArg += ArgConvert::ToConfig<string>(DevPathKey, fileName);
        sArg += ArgConvert::ToConfig<int>(AccessKey, accessMode);
        return sArg;
    }
    //----------------------------------------------------- 
    /// 创建句柄 
    virtual bool Create(FileHandler& obj, const char* sArg = NULL)
    {
        string devPath;
        int accessMode = O_RDWR;
        LOGGER(_log.WriteLine("ParseArg..."));
        if(!_parse_dev_path(sArg, devPath, accessMode))
            return false;
        LOGGER(_log << "FileName:<" << devPath << ">,AccessMode:<" << accessMode << ">\n");
        obj.Handle = open(devPath.c_str(), accessMode);
        return IsValid(obj);
    }
    /// 返回句柄是否有效 
    virtual bool IsValid(const FileHandler& obj)
    {
        return obj.Handle != INVALID_FD_VALUE;
    }
    /// 释放句柄 
    virtual void Dispose(FileHandler& obj)
    {
        close(obj.Handle);
        obj.Handle = INVALID_FD_VALUE;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 文件句柄读取器(同步)
class FileHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<FileHandler> _handle;
    /// 接收缓冲区,有长度进行标识,不需要初始化 
    byte _buff[DEV_BUFFER_SIZE];
    /// 读取数据 
    size_t _AsyncRead(ByteBuilder& data, size_t rlen)
    {
        ssize_t readCount = read(_handle->Handle, _buff, _min(rlen, DEV_BUFFER_SIZE));
        if(readCount < 1) return 0;

        size_t count = static_cast<size_t>(readCount);
        data.Append(ByteArray(_buff, count));
        return count;
    }
public:
    FileHandlerReader(const FileHandler& handle) { _handle = handle; }
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
    Ref<FileHandler> _handle;
    /// 待写入的数据 
    ByteArray _data;
public:
    FileHandlerWriter(const FileHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint)
    {
        _data = data;
        return true;
    }
    /// 写数据,返回成功写入的数据长度 
    virtual size_t Wait()
    {
        ssize_t len = write(_handle->Handle, _data.GetBuffer(), _data.GetLength());
        // 发送失败 
        if(len < 1) return 0;

        size_t slen = static_cast<size_t>(len);
        if(slen >= _data.GetLength())
            return SIZE_EOF;

        _data = _data.SubArray(slen);
        return slen;
    }
};
//--------------------------------------------------------- 
/// 文件句柄读取器(异步)
class AsyncFileHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<FileHandler> _handle;
    /// 接收缓冲区,有长度进行标识,不需要初始化 
    byte _buff[DEV_BUFFER_SIZE];
    /// 读句柄
    fd_set _fds;
    /// 超时时间
    uint _timeoutMs;
    /// 读取数据 
    size_t _AsyncRead(ByteBuilder& data, size_t rlen)
    {
    	// 超时处理
		struct timeval timeoutVal;
		timeoutVal.tv_sec = static_cast<int>(_timeoutMs / 1000);
		timeoutVal.tv_usec = static_cast<int>((_timeoutMs % 1000)*1000);

        int iRet = select(_handle->Handle + 1, &_fds, NULL, NULL, &timeoutVal);
        if(iRet > 0 && FD_ISSET(_handle->Handle, &_fds))
        {
            ssize_t readCount = read(_handle->Handle, _buff, _min(rlen, DEV_BUFFER_SIZE));
            // 数据已经读取完毕 
            if(readCount == 0)
                return SIZE_EOF;

            if(readCount > 0)
            {
                size_t rlen = static_cast<size_t>(iRet);
                data.Append(ByteArray(_buff, rlen));
                return rlen;
            }
        }
        return 0;
    }
public:
    AsyncFileHandlerReader(const FileHandler& handle) { _handle = handle; }
    /// 开始异步读取数据
    virtual bool Async(uint timeoutMs)
    {
    	_timeoutMs = timeoutMs;

        FD_ZERO(&_fds);
        FD_SET(_handle->Handle, &_fds);

        return true;
    }
    /// 等待读取数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        return _AsyncRead(data, DEV_BUFFER_SIZE);
    }
};
/// 文件句柄写入器(异步) 
class AsyncFileHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<FileHandler> _handle;
    /// 待写入的数据 
    ByteArray _data;
    /// 写句柄 
    fd_set _fds;
    /// 超时时间
    uint _timeoutMs;
public:
    AsyncFileHandlerWriter(const FileHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint timeoutMs)
    {
        _data = data;
        _timeoutMs = timeoutMs;

        FD_ZERO(&_fds);
        FD_SET(_handle->Handle, &_fds);

        return true;
    }
    /// 写数据,返回成功写入的数据长度 
    virtual size_t Wait()
    {
    	// 超时处理
		struct timeval timeoutVal;
		timeoutVal.tv_sec = static_cast<int>(_timeoutMs / 1000);
		timeoutVal.tv_usec = static_cast<int>((_timeoutMs % 1000)*1000);

        int iRet = select(_handle->Handle + 1, NULL, &_fds, NULL, &timeoutVal);
        if(iRet > 0 && FD_ISSET(_handle->Handle, &_fds))
        {
            ssize_t len = write(_handle->Handle, _data.GetBuffer(), _data.GetLength());
            // 发送失败 
            if(len < 1) return 0;

            size_t slen = static_cast<size_t>(len);
            if(slen >= _data.GetLength())
                return SIZE_EOF;

            _data = _data.SubArray(slen);
            return slen;
        }
        return 0;
    }
};
//--------------------------------------------------------- 
/// 基于文件的读写设备(同步)
typedef HandlerDevice<HandlerBaseDevice<FileHandler, FileHandlerFactory>, FileHandlerReader, FileHandlerWriter> FileDevice;
/// 基于文件的读写设备(异步)
typedef HandlerDevice<HandlerBaseDevice<FileHandler, FileHandlerFactory>, AsyncFileHandlerReader, AsyncFileHandlerWriter> AsyncFileDevice;
//--------------------------------------------------------- 
#undef INVALID_FD_VALUE
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_FILEDEVICE_H_
//========================================================= 
