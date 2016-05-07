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
        return itr;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    SplitArgParser(char splitChar = SPLIT_CHAR) : IStringArgParser(), SplitChar(splitChar) {}
    /// 解析数据
    virtual size_t Parse(const ByteArray& str)
    {
        list<string> strlist;
        StringHelper::Split(str.GetString(), strlist, SplitChar);
        list<string>::iterator itr;
        for(itr = strlist.begin();itr != strlist.end(); ++itr)
        {
            _args.obj().push_back(ArgValue<string, string>());
            _args.obj().back().Value = (*itr);
        }
        _itr = _args.obj().begin();
        return strlist.size();
    }
    /// 字符间分隔符
    char SplitChar;
    /// 转换为字符串
    virtual size_t ToString(ByteBuilder& argMsg)
    {
        size_t len = 0;
        list<ArgValue<string, string> >::iterator itr;
        for(itr = _args.obj().begin();itr != _args.obj().end(); ++itr)
        {
            len += itr->Value.length();
            len += 1;
            argMsg.Append(ByteArray(itr->Value.c_str(), itr->Value.length()));
            argMsg.Append(SplitChar);
        }
        if(len > 0)
        {
            --len;
            argMsg.RemoveTail();
        }
        return len;
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