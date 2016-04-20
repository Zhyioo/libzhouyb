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
using zhou_yb::base::IArgParser;

#include "StringHelper.h"
using zhou_yb::extension::ability::StringHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 通过'|'号分隔参数的解析器
class SplitArgParser : public IArgParser<string, string>
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
    /// 解析数据
    size_t Parse(const ByteArray& str, char splitChar = SPLIT_CHAR)
    {
        list<string> strlist;
        StringHelper::Split(str.GetString(), strlist, splitChar);
        list<string>::iterator itr;
        for(itr = strlist.begin();itr != strlist.end(); ++itr)
        {
            _args.obj().push_back(ArgValue<string, string>());
            _args.obj().back().Value = (*itr);
        }
        _itr = _args.obj().begin();
        return strlist.size();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
}
}
}
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SPLITARGPARSER_H_
//========================================================= 