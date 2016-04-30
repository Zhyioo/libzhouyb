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
    /// 转换Mode为字符串描述信息
    static const char* TextModeToString(TextMode mode)
    {
        switch(mode)
        {
        case TextLogger:
            return "LOGGER:   ";
        case TextMessage:
            return "MESSAGE:  ";
        case TextTips:
            return "TIPS:     ";
        case TextInfo:
            return "INFO:     ";
        case TextResult:
            return "RESULT:   ";
        case TextError:
            return "ERROR:    ";
        case TextWarning:
            return "WARNING:  ";
        case TextHighWarning:
            return "H-WARNING:";
        case TextNote:
            return "NOTE:     ";
        }
        return     "TEXT:     ";
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
        _logPrinter << TextModeToString(mode) << _strput(text) << endl;
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