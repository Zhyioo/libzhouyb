//========================================================= 
/**@file StringHelper.h 
 * @brief 字符串辅助函数 
 *
 * @date 2014-11-07   21:30:11 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_STRINGHELPER_H_
#define _LIBZHOUYB_STRINGHELPER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 字符串辅助 
class StringHelper
{
protected:
    StringHelper() {}
public:
    /// 分割字符串,返回分隔的子串个数 
    static size_t Split(const char* src, list<string>& _list, char flag = SPLIT_CHAR)
    {
        size_t count = 1;
        size_t srclen = _strlen(src);
        
        _list.push_back("");
        for(size_t i = 0;i < srclen; ++i)
        {
            if(src[i] == flag)
            {
                _list.push_back("");
                ++count;
                continue;
            }
            _list.back() += src[i];
        }

        return count;
    }
    /// 组合字符串,将各个子串用join_str分隔开  
    static string Join(const list<string>& _list, const char* join_str = SPLIT_STRING)
    {
        string dst;
        list<string>::const_iterator itr;
        list<string>::const_iterator lastItr;
        
        lastItr = _list.end();
        --lastItr;
        
        for(itr = _list.begin();itr != lastItr; ++itr)
        {
            dst += *itr;
            dst += _strput(join_str);
        }
        
        return dst;
    }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_STRINGHELPER_H_
//========================================================= 