//========================================================= 
/**@file IConverter.h 
 * @brief 数据转换接口DLL函数导出声明(使用自动的工具宏导出为C方式) 
 * 
 * @date 2013-07-30   22:53:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
/*
 * 每个转换器DLL中可以包含多个转换接口,每个转换接口由3个函数构成
 * 其中,Tips接口可以不支持.转换时,ConvertFrom将数据转换,并在DLL中
 * 缓存,上层通过ConvertTo来分段获取数据.
 *
 * 转换的输入不一定仅限为数据,可以带一些配置信息,或者为文件路径,
 * 转换时输入路径,转换后直接以文件的形式将数据输出也行 
 *
 * 在配置插件时,由3部分组成:
 * "插件的显示名称" + "=" + "插件路径" + ["," + "插件入口"]
 * 上述格式中,中括号的数据可选,如果不指明入口点,则认为插件的显示名
 * 称就是入口点,比如:
 * BcdToAscii=".\converters\BaseConvert.dll,BcdToAscii"
 * AsciiToBcd=".\converters\BaseConvert.dll"
 */
//--------------------------------------------------------- 
#include "../../base/Convert.h"
//--------------------------------------------------------- 
/* 转换器DLL需要支持的接口 */
/// 将input按照format格式转换,返回需要的缓冲区大小,转换后的结果由ConvertTo保存 
typedef size_t (__cdecl *fpConvertFrom)(const char* input, const char* arg);
/// 获取转换后的结果,返回当前已经拷贝的数据长度 
typedef size_t (__cdecl *fpConvertTo)(char* output, size_t outsize);
/// 获取转换器的一些说明[128长度内]
typedef void   (__cdecl *fpTips)(char tips[128]);
//--------------------------------------------------------- 
/* 如果使用const char* 类型作为输入,ByteBuilder 类型作为输出则可以使用以下示例简化操作 */
//--------------------------------------------------------- 
/* 宏声明 */
/// 函数调用方式 
#ifndef CONVERTER_CALL
#   define CONVERTER_CALL __cdecl 
#endif
/// 函数导出方式 
#ifndef CONVERTER_API
#   define CONVERTER_API extern "C" __declspec(dllexport)
#endif
/// 转换后剩余数据的长度 
#define CONVERTER_LEN(spacename) _##spacename##_global_length
/// 转换的临时缓冲区 
#define CONVERTER_BUFF(spacename) _##spacename##_global_buffer

/// 转换的导出函数 
#define CONVERTER_EXPORT(spacename) \
ByteBuilder CONVERTER_BUFF(spacename)(128); \
size_t CONVERTER_LEN(spacename); \
CONVERTER_API size_t CONVERTER_CALL spacename##_ConvertFrom(const char* input, const char* arg) \
{ \
    CONVERTER_BUFF(spacename).Clear(); \
    spacename(input, CONVERTER_BUFF(spacename), arg); \
    CONVERTER_LEN(spacename) = CONVERTER_BUFF(spacename).GetLength(); \
    return CONVERTER_LEN(spacename); \
} \
CONVERTER_API size_t CONVERTER_CALL spacename##_ConvertTo(char* output, size_t outsize) \
{ \
    if(CONVERTER_LEN(spacename) < 1) \
        return 0; \
    size_t copylen = _min(outsize - 1, CONVERTER_LEN(spacename)); \
    size_t offset = CONVERTER_BUFF(spacename).GetLength() - CONVERTER_LEN(spacename); \
    memcpy(output, CONVERTER_BUFF(spacename).GetBuffer(offset), copylen); \
    output[copylen] = 0; \
    CONVERTER_LEN(spacename) -= copylen; \
    return copylen; \
}
#define CONVERTER_TIPS_EXPORT(spacename,tipsstr) \
CONVERTER_API void CONVERTER_CALL spacename##_Tips(char tips[128]) \
{ \
    int len = strlen(tipsstr); \
    memcpy(tips, tipsstr, len); \
    tips[len] = 0; \
} 
/*
--- 函数实现 ---
bool BcdToAscii(const char* bcd, ByteBuilder& ascii, const char* arg)
{
    DevCommand::FromAscii(bcd, ascii);

    return true;
}
--- 转换接口定义(实际导出的转换函数) ---
CONVERTER_EXPORT(BcdToAscii);
CONVERTER_TIPS_EXPORT(BcdToAscii, "将16进制表示的字符串剔除空格后转换为ASCII码串(D6D0416231322121=>中Ab12!!)");
*/
//========================================================= 