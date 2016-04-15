//========================================================= 
/**@file StringCallBackLogger.h 
 * @brief 扩展的日志类(支持将日志打印操作回调)
 * 
 * @date 2012-9-8   02:44:02 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_STRINGCALLBACKLOGGER_H_
#define _LIBZHOUYB_STRINGCALLBACKLOGGER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include <sstream>
using std::ostringstream;
//---------------------------------------------------------
namespace zhou_yb {
namespace extension {
namespace ability {
//---------------------------------------------------------
/**
 * @brief 将日志的输出操作通过一个回调函数进行回调  
 */
class StringCallBackLogger : public StringLogger
{
public:
    typedef void (__stdcall *logOutputCallBack)(const char* str);
protected:
    /// 回调函数 
    logOutputCallBack _callback;
    // 日志缓冲区更新事件 
    virtual void _buf_after_changed()
    {
        string strVal = StringLogger::String();
        StringLogger::Clear();
        size_t len = strVal.length();

        // 利用回调通知修改  
        if(len > 0)
        {
            _callback(strVal.c_str());
        }
    }
public:
    StringCallBackLogger(logOutputCallBack callback = NULL, bool openlog = false)
        : StringLogger()
    {
        _os = &_ostr;
        
        if(openlog)
            Open(callback);
    }
    //-----------------------------------------------------
    /**
     * @brief 开启日志记录功能
     * @param [in] sArg [default:NULL]
     * - 参数格式:
     *  - NULL 根据上一次的配置打开 
     *  - pointer OstreamLogger::PointerKey 回调函数指针参数配置项 
     * .
     */ 
    virtual bool Open(const char* sArg = NULL)
    {
        if(!_is_empty_or_null(sArg))
        {
            ArgParser cfg;
            cfg.Parse(sArg);
            pointer ptr = NULL;
            ArgConvert::FromConfig(cfg, OstreamLogger::PointerKey, ptr);
            _callback = reinterpret_cast<logOutputCallBack>(ptr);
        }
        return Open(_callback);
    }
    /// 打开回调函数
    bool Open(logOutputCallBack callback)
    {
        if(StringLogger::Open())
        {
            _callback = callback;
            if(!IsOpen())
                StringLogger::Close();
        }

        return IsOpen();
    }
    /// 是否打开 
    virtual bool IsOpen()
    {
        return _callback != NULL && StringLogger::IsOpen();
    }
    /// 刷新缓冲区
    virtual void Flush()
    {
        if(IsOpen())
        {
            StringLogger::Flush();
            _buf_after_changed();
        }
    }
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_STRINGCALLBACKLOGGER_H_
//========================================================= 
