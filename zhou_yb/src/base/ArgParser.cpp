//========================================================= 
/**@file ArgParser.cpp 
 * @brief base目录下ArgParser库中全局函数、变量的实现 
 * 
 * @date 2012-06-23   11:26:05 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ARGPARSER_CPP_
#define _LIBZHOUYB_ARGPARSER_CPP_
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
template<> const char* ArgConvert::FromString<const char*>(const string& str)
{
    return str.c_str();
}
template<> bool ArgConvert::FromString<string>(const string& str, string& val)
{
    val = str;
    return true;
}
template<> bool ArgConvert::FromString<pointer>(const string& str, pointer& val)
{
    if(str.length() != (2+sizeof(pointer)) 
        && str[0] != '0'
        && _get_lower(str[1]) != 'x')
    {
        // 格式错误
        return false;
    }
    ByteBuilder tmpBuffer(8);
    ByteConvert::FromAscii(ByteArray(str.c_str() + 2, 2*sizeof(pointer)), tmpBuffer);
    int ptr = 0;
    int tmp = 0;
    for(size_t i = 1;i <= tmpBuffer.GetLength(); ++i)
    {
        tmp = tmpBuffer[i - 1];
        tmp <<= (tmpBuffer.GetLength() - i) * BIT_OFFSET;
        ptr += tmp;
    }
    // val为引用传递&,一定存在val的地址,所以&val是安全的 
    int* pInt = reinterpret_cast<int*>(&val);
    *pInt = ptr;

    return true;
}
template<> bool ArgConvert::FromString<ByteBuilder>(const string& str, ByteBuilder& val)
{
    DevCommand::FromAscii(ByteArray(str.c_str(), str.length()), val);
    return true;
}
template<> bool ArgConvert::FromString<bool>(const string& str, bool& val)
{
    int iVal = 0;
    FromString<int>(str, iVal);
    val = StringConvert::Compare(ByteArray(str.c_str(), str.length()), "true", true) || (iVal != 0);
    return true;
}
template<> bool ArgConvert::FromString<byte>(const string& str, byte& val)
{
    byte buff[2];
    if(str.length() > 1)
    {
        buff[0] = str[0];
        buff[1] = str[1];
    }
    else
    {
        buff[0] = 0;
        buff[1] = str[0];
    }
    ByteBuilder tmp(2);
    if(ByteConvert::FromAscii(ByteArray(buff, 2), tmp) < 1)
        return false;
    val = tmp[0];
    return true;
}
template<> string ArgConvert::ToString<string>(const string & val)
{
    return val;
}
template<> string ArgConvert::ToString<pointer>(const pointer & val)
{
    return _hex_num(val);
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
template<> string ArgConvert::ToString<byte>(const byte& val)
{
    string str;
    byte b = val & 0x0F0;
    b >>= HALF_BIT_OFFSET;
    str += (b < 0x0A) ? (b + '0') : (b - 0x0A + 'A');

    b = val & 0x0F;
    str += (b < 0x0A) ? (b + '0') : (b - 0x0A + 'A');

    return str;
}
//--------------------------------------------------------- 
} // namespace base 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ARGPARSER_CPP_
//=========================================================