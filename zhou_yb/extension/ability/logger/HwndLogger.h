//========================================================= 
/**@file HwndLogger.h 
 * @brief 扩展的日志类(支持将日志打印到指定句柄的窗口中)
 * 
 * @date 2011-10-23   11:07:37 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HWNDLOGGER_H_
#define _LIBZHOUYB_HWNDLOGGER_H_
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
 * @brief 将日志重定向到窗口中 
 *
 * 该类以WM_CHAR的消息实现，故窗口句柄需要支持WM_CHAR消息，建议
 * 采用Textbox并且设置格式为：非只读、多行、支持回车、自动滚动
 */
class HwndLogger : public StringLogger
{
protected:
    /// 设定的句柄 
    HWND _hwnd;
    // 缓冲区事件 
    virtual void _buf_after_changed()
    {
        string strVal = StringLogger::String();
        StringLogger::Clear();
        size_t len = strVal.length();
        
        /* 如果窗口句柄为空则输出到当前能够输出的地方 */
        HWND hFocusWnd = _hwnd;
        if(_hwnd == NULL)
        {
            HWND hWnd = GetForegroundWindow();
            DWORD processID = GetWindowThreadProcessId(hWnd, NULL);
            DWORD threadID = GetCurrentThreadId();
            AttachThreadInput(threadID, processID, TRUE);
            hFocusWnd = GetFocus();
        }
        // 设置输入光标到最后 
        PostMessage(hFocusWnd, EM_SETSEL, static_cast<WPARAM>(-1), static_cast<LPARAM>(-1));
        WPARAM tmp = 0;
        // 通知窗体数据更新 
        for(size_t i = 0;i < len; ++i)
        {
            // 取后8位(自动转换会在前面补0造成字符不正确) 
            tmp = 0xFF & strVal[i];
            PostMessage(hFocusWnd, WM_CHAR, tmp, 0);
        }
    }
public:
    HwndLogger(HWND hwnd = NULL, bool openlog = false)
        : _hwnd(hwnd), StringLogger()
    {
        if(openlog)
            Open();
    }
    //-----------------------------------------------------
    /**
     * @brief 开启日志记录功能
     * @param [in] sArg [default:NULL] 
     * - 参数格式:
     *  - NULL 输出到当前光标的位置  
     *  - pointer OstreamLogger::PointerKey 配置项参数(HWND句柄) 
     * .
     */ 
    virtual bool Open(const char* sArg = NULL)
    {
        HWND hWnd = _hwnd;
        if(sArg != NULL)
        {
            ArgParser cfg;
            cfg.Parse(sArg);
            pointer ptr = NULL;
            ArgConvert::FromConfig(cfg, OstreamLogger::PointerKey, ptr);
            hWnd = (HWND)ptr;
        }
        return Open(hWnd);
    }
    /// 打开窗口句柄输出日志
    bool Open(HWND hwnd)
    {
        _hwnd = hwnd;
        return StringLogger::Open();
    }
    //-----------------------------------------------------
};
//---------------------------------------------------------
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HWNDLOGGER_H_
//=========================================================
