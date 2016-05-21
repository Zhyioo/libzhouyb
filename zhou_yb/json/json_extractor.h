//========================================================= 
/**@file json_extractor.h
 * @brief 使用JsonCpp库做json数据转换
 * 
 * @date 2016-05-21   11:13:58
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_JSON_EXTRACTOR_H_
#define _LIBZHOUYB_JSON_EXTRACTOR_H_
//--------------------------------------------------------- 
#include "./jsoncpp/autolink.h"
#include "./jsoncpp/config.h"
#include "./jsoncpp/features.h"
#include "./jsoncpp/forwards.h"
#include "./jsoncpp/value.h"
#include "./jsoncpp/reader.h"
#include "./jsoncpp/writer.h"

#include "../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace json_extractor {
//--------------------------------------------------------- 
/// Json参数解析器
class JsonArgParser : public IStringArgParser
{
protected:
    /// 键值比较函数
    virtual bool _Compare(const KeyType& k1, const KeyType& k2)
    {
        ByteArray k1Array(k1.c_str(), k1.length());
        ByteArray k2Array(k2.c_str(), k2.length());

        return StringConvert::Compare(k1Array, k2Array, IsIgnoreCase);
    }
public:
    JsonArgParser() : IStringArgParser() { IsIgnoreCase = true; }
    /// 解析json字符串
    static size_t Parse(IArgParser<string, string>& arg, const ByteArray& str)
    {
        Json::Reader reader;
        Json::Value root;
        size_t count = 0;
        const char* endDoc = str.GetString() + str.GetLength();
        if(!reader.parse(str.GetString(), endDoc, root))
            return count;

        Json::Value::iterator itr;
        for(itr = root.begin();itr != root.end(); ++itr)
        {
            arg.PushValue(itr.key().asString(), (*itr).asString());
            ++count;
        }

        return count;
    }
    /// 将数据转为json字符串
    static size_t ToString(IArgParser<string, string>& arg, ByteBuilder& argMsg)
    {
        Json::FastWriter writer;
        Json::Value root;
        KeyType key;
        ValueType val;
        
        while(arg.EnumValue(&val, &key))
        {
            root[key] = val;
        }
        string jsonstr = writer.write(root);
        size_t len = jsonstr.length();
        argMsg.Append(ByteArray(jsonstr.c_str(), len));
        return len;
    }
    /// 在查找键值的时候是否忽略大小写
    bool IsIgnoreCase;
    /// 从字符串中解析json
    virtual size_t Parse(const ByteArray& str)
    {
        return Parse(*this, str);
    }
    /// 将json数据输出到字符串中
    virtual size_t ToString(ByteBuilder& argMsg)
    {
        return ToString(*this, argMsg);
    }
};
//--------------------------------------------------------- 
} // namespace json_extractor
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_JSON_EXTRACTOR_H_
//========================================================= 