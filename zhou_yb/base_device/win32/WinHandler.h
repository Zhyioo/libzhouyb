//========================================================= 
/**@file WinHandler.h
 * @brief Windows句柄托管 
 * 
 * @date 2015-09-30   16:08:35
 * @author Zhyioo 
 * @version 1.0
 */
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINHANDLER_H_
#define _LIBZHOUYB_WINHANDLER_H_
//--------------------------------------------------------- 
#include "../Handler.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//---------------------------------------------------------
/// 句柄封装,提供基本的操作 
struct WinHandler : public Handler<HANDLE>
{
    WinHandler() { Handle = NULL; }

    /// 返回句柄是否有效 
    inline static bool IsValidHandle(HANDLE handle)
    {
        return handle != NULL && handle != INVALID_HANDLE_VALUE;
    }
    /// 关闭句柄 
    inline static void Close(HANDLE& handle)
    {
        if(IsValidHandle(handle))
        {
            CloseHandle(handle);
            handle = NULL;
        }
    }
};
//--------------------------------------------------------- 
/// 异步句柄 
struct WinAsyncHandler : public WinHandler
{
    /// 异步触发器 
    OVERLAPPED WaitOverlapped;

    WinAsyncHandler() : WinHandler()
    {
        SetObjZero(WaitOverlapped);
        WaitOverlapped.hEvent = NULL;
    }
    /// 重置异步数据 
    static bool AsyncReset(WinAsyncHandler& handle)
    {
        return Tobool(ResetEvent(handle.WaitOverlapped.hEvent));
    }
};
//--------------------------------------------------------- 
/// Windows下获取错误信息的行为 
class WinLastErrBehavior : public LastErrBehavior
{
public:
    /// 获取指定错误码的错误描述信息 
    static const char* TransErrToString(int errCode)
    {
        static string errMsg;

        errMsg = "";
        LPVOID lpErr = NULL;
        if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            ctype_cast(LPTSTR)(&lpErr), 0, NULL))
        {
            CharConverter cvt;
            errMsg = cvt.to_char(reinterpret_cast<char_t*>(lpErr));
            if(errMsg.length() > 0)
            {
                string::iterator itr = errMsg.end();
                --itr;
                while(errMsg.length() > 0 && (*itr == '\n' || *itr == '\r'))
                {
                    string::iterator tmp = itr;
                    --itr;
                    errMsg.erase(tmp);
                }
            }
            LocalFree(lpErr);
        }

        return errMsg.c_str();
    }

    /// 错误信息
    virtual int GetLastErr()
    {
        _lasterr = ::GetLastError();
        return _lasterr;
    }
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage()
    {
        _errinfo = TransErrToString(_lasterr);
        return _errinfo.c_str();
    }
};
//--------------------------------------------------------- 
/// Windows下基于句柄的读写设备 
template<class THandler, class THandlerFactory>
class WinHandlerBaseDevice : public HandlerBaseDevice<THandler, THandlerFactory>
{
protected:
    //----------------------------------------------------- 
    /// 记录错误信息 
    virtual void _logErr(int errCode, const char* errinfo = NULL)
    {
        DWORD errId = GetLastError();
        string msg = _strput(errinfo);
        if(errId != 0)
        {
            if(msg.length() > 0)
                msg += " ";
            msg += ArgConvert::ToString(errId);
            msg += ",";
            msg += WinLastErrBehavior::TransErrToString(errId);
        }
        DeviceBehavior::_logErr(errCode, msg.c_str());
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    WinHandlerBaseDevice() : HandlerBaseDevice() {}
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINHANDLER_H_
//========================================================= 