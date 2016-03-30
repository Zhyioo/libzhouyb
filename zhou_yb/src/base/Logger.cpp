//========================================================= 
/**@file Logger.cpp 
 * @brief base目录下Logger库中全局函数、变量的实现 
 * 
 * @date 2012-06-23   11:22:53 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../base/Logger.h"
using namespace zhou_yb;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base {
//--------------------------------------------------------- 
/// 获取当前系统时间 
size_t get_current_systime(ByteBuilder& timebuff)
{
    // 输出当前系统时间
    time_t lt;
    time(&lt);
    tm t;
    size_t len = 0;

    // VC6下版本
    t = (*localtime(&lt));

    len = timebuff.Format("%04d-%02d-%02d %02d:%02d:%02d",
        (1900 + t.tm_year), (t.tm_mon + 1), t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    return len;
}
//--------------------------------------------------------- 
/// 流日志OstreamLogger.Open参数配置项主键 Ptr
const char OstreamLogger::PointerKey[] = "Ptr";
/// 文件日志FileLogger.Open参数配置项主键 FilePath 
const char FileLogger::FilePathKey[] = "FilePath";
/// 空的日志对象 
const LoggerAdapter LoggerAdapter::Null;
//--------------------------------------------------------- 
} // namespace base 
} // namespace zhou_yb
//=========================================================