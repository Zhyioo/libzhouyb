//========================================================= 
/**@file StringLogger.h 
 * @brief 扩展的日志类,将日志输出到一个字符串中 
 * 
 * @date 2014-06-02   11:26:26 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_STRINGLOGGER_H_
#define _LIBZHOUYB_STRINGLOGGER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include <sstream>
using std::ostringstream;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 将日志输出到一个字符串中 
class StringLogger : public OstreamLogger
{
protected:
    /// 输出流 
    ostringstream _ostr;
public:
    StringLogger() : OstreamLogger()
    {
        OstreamLogger::Open(_ostr);
    }
    virtual ~StringLogger() { Close(); }
    //----------------------------------------------------- 
    /// 开启日志 
    virtual bool Open(const char* = NULL)
    {
        return OstreamLogger::Open(_ostr);
    }
    /// 关闭日志 
    virtual void Close()
    {
        OstreamLogger::Close();
        Clear();
    }
    //----------------------------------------------------- 
    /// 返回当前记录的日志 
    inline string String() const
    {
        return _ostr.str();
    }
    /// 清空日志  
    void Clear()
    {
        _ostr.clear();
        _ostr.str("");
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_STRINGLOGGER_H_
//========================================================= 
