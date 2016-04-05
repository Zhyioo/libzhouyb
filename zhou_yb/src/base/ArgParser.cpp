//========================================================= 
/**@file ArgParser.cpp 
 * @brief base目录下ArgParser库中全局函数、变量的实现 
 * 
 * @date 2012-06-23   11:26:05 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../base/DevDefine.h"
#include "../../base/Logger.h"
#include "../../base/ArgParser.h"
using namespace zhou_yb;
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//--------------------------------------------------------- 
// 转换的模板特化 
template<> bool ArgConvert::FromString<pointer>(const char* str, pointer& val)
{
    if(_strlen(str) != 10 
        && str[0] != '0'
        && _get_lower(str[1]) != 'x')
    {
        // 格式错误
        return false;
    }
    ByteBuilder tmpBuffer(8);
    ByteConvert::FromAscii(ByteArray(str + 2, 8), tmpBuffer);
    int ptr = 0;
    for(size_t i = 0;i < tmpBuffer.GetLength() - 1; ++i)
    {
        ptr += tmpBuffer[i];
        ptr <<= BIT_OFFSET;
    }
    ptr += tmpBuffer[tmpBuffer.GetLength() - 1];
    // val为引用传递&,一定存在val的地址,所以&val是安全的 
    int* pInt = reinterpret_cast<int*>(&val);
    *pInt = ptr;

    return true;
}
template<> bool ArgConvert::FromString<ByteBuilder>(const char* str, ByteBuilder& val)
{
    if(_is_empty_or_null(str))
        return false;
    DevCommand::FromAscii(str, val);
    return true;
}
template<> bool ArgConvert::FromString<bool>(const char* str, bool& val)
{
    int iVal = 0;
    FromString<int>(str, iVal);
    val = StringConvert::Compare(str, "true", true) || (iVal != 0);
    return true;
}
template<> string ArgConvert::ToString<pointer>(const pointer & val)
{
    return _hex_num((int*)val);
}
template<> string ArgConvert::ToString<ByteArray>(const ByteArray& val)
{
    if(val.IsEmpty())
        return "";
        
    ByteBuilder tmp(32);
    ByteConvert::ToAscii(val, tmp);
    
    return tmp.GetString();
}
template<> string ArgConvert::ToString<ByteBuilder>(const ByteBuilder& val)
{
    return ArgConvert::ToString<ByteArray>(val);
}
template<> string ArgConvert::ToString<bool>(const bool& val)
{
    return val ? "True" : "False";
}
//--------------------------------------------------------- 
} // namespace base 
} // namespace zhou_yb
//=========================================================