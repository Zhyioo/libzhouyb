//========================================================= 
/**@file TextPrinter.h 
 * @brief 文本输出工具 
 * 
 * @date 2015-01-03   18:16:04 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TEXTPRINTER_H_
#define _LIBZHOUYB_TEXTPRINTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../container/object.h"
using zhou_yb::container::shared_obj;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 文本输出器 
class TextPrinter 
{
public:
    //----------------------------------------------------- 
    /// 输出文本的模式 
    enum TextMode
    {
        /* 重要级别依次递增 */

        /// 日志 [灰色]
        TextLogger = 0,
        /// 普通文本 [暗黑色]
        TextNote,
        /// 消息 [深绿色]
        TextMessage, 
        /// 提示 [粉红色]
        TextTips,
        /// 信息 [淡蓝色]
        TextInfo,
        /// 成功 [蓝色]
        TextResult,
        /// 警告 [橙黄色]
        TextWarning,
        /// 重要警告 [洋红色]
        TextHighWarning,
        /// 错误信息 [深红色]
        TextError
    };
    //----------------------------------------------------- 
    /// 文本输出回调函数 
    typedef void (__stdcall *TextPrintCallback)(int, const char*);
    //----------------------------------------------------- 
    static void LoggerTextPrint(LoggerAdapter& log, TextMode mode, const char* text)
    {
        switch(mode)
        {
        case TextLogger:
            log << "LOGGER:";
            break;
        case TextMessage:
            log << "MESSAGE:";
            break;
        case TextTips:
            log << "TIPS:";
            break;
        case TextInfo:
            log << "INFO:";
            break;
        case TextResult:
            log << "RESULT:";
            break;
        case TextError:
            log << "ERROR:";
            break;
        case TextWarning:
            log << "WARNING:";
            break;
        case TextHighWarning:
            log << "HIGH WARNING:";
            break;
        case TextNote:
            log << "NOTE:";
            break;
        default:
            log << "TEXT:";
            break;
        }
        log << text << endl;
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 输出回调函数 
    shared_obj<TextPrintCallback> _printer;
    /// 日志输出 
    LoggerAdapter _logPrinter;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    TextPrinter()
    {
        _printer.obj() = NULL;
    }
    TextPrinter(const TextPrinter& obj)
    {
        Select(obj);
    }
    virtual ~TextPrinter()
    {
        Release();
    }
    //----------------------------------------------------- 
    /// 输出信息 
    void TextPrint(TextMode mode, const char* text)
    {
        if(_printer.obj() != NULL)
        {
            _printer.obj()(static_cast<int>(mode), text);
        }
        _logPrinter << current_systime << ' ';
        LoggerTextPrint(_logPrinter, mode, text);
    }
    //----------------------------------------------------- 
    inline void Select(TextPrintCallback printer)
    {
        _printer.obj() = printer;
    }
    inline void Select(LoggerAdapter& log)
    {
        _logPrinter.Select(log);
    }
    inline void Select(const TextPrinter& obj)
    {
        if(&obj == this)
            return;

        _printer = obj._printer;
        _logPrinter.Select(obj._logPrinter);
    }
    inline void Release()
    {
        _printer.obj() = NULL;
        _logPrinter.Release();
    }
    /// 返回适配的日志对象,这样能够更直接方便的输出TextLogger
    inline const LoggerAdapter& GetLogger() const
    {
        return _logPrinter;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TEXTPRINTER_H_
//========================================================= 