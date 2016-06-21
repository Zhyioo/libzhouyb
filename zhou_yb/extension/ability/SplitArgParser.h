//========================================================= 
/**@file SplitArgParser.h
 * @brief 通过'|'号分隔参数的解析器
 * 
 * @date 2016-04-19   21:59:23
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SPLITARGPARSER_H_
#define _LIBZHOUYB_SPLITARGPARSER_H_
//--------------------------------------------------------- 
#include "../../base/ArgParser.h"
using zhou_yb::base::ArgValue;
using zhou_yb::base::IStringArgParser;

#include "StringHelper.h"
using zhou_yb::extension::ability::StringHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 通过'|'号分隔参数的解析器
class SplitArgParser : public IStringArgParser
{
protected:
    //----------------------------------------------------- 
    /// 递增参数
    virtual list<ArgValue<string, string> >::iterator _Find(const string& key)
    {
        list<ArgValue<string, string> >::iterator itr = _itr;
        ++_itr;
        // 如果找到结尾则重新定位到开头
        if (_itr == _args.obj().end())
            _itr = _args.obj().begin();
        return itr;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    SplitArgParser(char splitChar = SPLIT_CHAR) : IStringArgParser(), SplitChar(splitChar) {}
    /// 解析数据到IArgParser<string, string>中
    static size_t Parse(IArgParser<string, string>& arg, const ByteArray& str, char splitChar = SPLIT_CHAR)
    {
        list<string> strlist;
        StringHelper::Split(str.GetString(), strlist, splitChar);
        list<string>::iterator itr;
        for(itr = strlist.begin();itr != strlist.end(); ++itr)
        {
            arg.PushValue("", *itr);
        }
        return strlist.size();
    }
    /// 将数据转换为字符串
    static size_t ToString(IArgParser<string, string>& arg, ByteBuilder& argMsg, char splitChar = SPLIT_CHAR)
    {
        size_t len = 0;
        list<ArgValue<string, string> >::iterator itr;
        ValueType val;
        while(arg.EnumValue(&val, NULL))
        {
            len += val.length();
            len += 1;
            argMsg.Append(ByteArray(val.c_str(), val.length()));
            argMsg.Append(splitChar);
        }
        if(len > 0)
        {
            --len;
            argMsg.RemoveTail();
        }
        return len;
    }
    /// 解析数据
    virtual size_t Parse(const ByteArray& str)
    {
        size_t count = Parse(*this, str, SplitChar);
        _itr = _args.obj().begin();
        return count;
    }
    /// 字符间分隔符
    char SplitChar;
    /// 转换为字符串
    virtual size_t ToString(ByteBuilder& argMsg)
    {
        return ToString(*this, argMsg, SplitChar);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SPLITARGPARSER_H_
//========================================================= 