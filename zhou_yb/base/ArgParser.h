//========================================================= 
/**@file ArgParser.h 
 * @brief 字符串参数配置信息
 * 
 * 如:[Port:<COM1>][Gate:B][Timeout:30],嵌套:[USB:<[VID:<1DFC>][PID:<8903>]>] 
 * 用于虚拟化不同设备,所有设备统一通过string传入参数操作,做
 * 到接口统一 
 *
 * @date 2011-10-17   11:03:40 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ARGPARSER_H_
#define _LIBZHOUYB_ARGPARSER_H_
//---------------------------------------------------------
#include "Convert.h"
#include "../container/list.h"
#include "../container/object.h"
using zhou_yb::container::shared_obj;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;
using std::istringstream;
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//---------------------------------------------------------
/// 参数转换器(使用命令空间而不使用类方便后续其他代码文件扩展)  
namespace ArgConvert
{
    //----------------------------------------------------- 
    /// 将字符串string转为指定的格式
    template<class T>
    bool FromString(const string& str, T& val)
    {
        istringstream sstream(str);
        sstream >> val;
        return true;
    }
    //----------------------------------------------------- 
    /// 将字符串string转换为指定的格式
    template<class T>
    T FromString(const string& str)
    {
        T tmp;
        FromString<T>(str, tmp);
        return tmp;
    }
    //----------------------------------------------------- 
    /// 将指定格式的数据转换为字符串,需要T支持operator <<或模板特化来实现  
    template<class T>
    string ToString(const T& val)
    {
        ostringstream sstream;
        sstream<<val;
        return sstream.str();
    }
    //----------------------------------------------------- 
    /// 将字符串string转为const char*
    template<>
    const char* FromString<const char*>(const string& str);
    //----------------------------------------------------- 
    /// 解析str中的pointer数据 
    template<> 
    bool FromString<pointer>(const string& str, pointer& val);
    //----------------------------------------------------- 
    /// 解析str中的ByteBuilder数据 
    template<> 
    bool FromString<ByteBuilder>(const string& str, ByteBuilder& val);
    //----------------------------------------------------- 
    /// 解析str中的bool数据 
    template<>
    bool FromString<bool>(const string& str, bool& val);
    //----------------------------------------------------- 
    /// 解析str中的byte数据
    template<>
    bool FromString<byte>(const string& str, byte& val);
    //----------------------------------------------------- 
    /**
     * @brief 将 pointer 转换为字符串
     * @param [in] val 需要转换的pointer
     * @retval string
     * @return 
     *
     * @code
     string str = ArgConvert::ToString((pointer)&log);
     str -> "0x004AF958"
     * @endcode
     *
     */
    template<> 
    string ToString<pointer>(const pointer & val);
    //----------------------------------------------------- 
    /// 将ByteArray转换成字符串 
    template<>
    string ToString<ByteArray>(const ByteArray& val);
    /// 将ByteBuilder转换成字符串 
    template<>
    string ToString<ByteBuilder>(const ByteBuilder& val);
    //----------------------------------------------------- 
    /// 将bool转换成字符串 
    template<>
    string ToString<bool>(const bool& val);
    //----------------------------------------------------- 
    /// 将bool转换成字符串
    template<>
    string ToString<byte>(const byte& val);
    //----------------------------------------------------- 
} // namespace ArgConvert
//--------------------------------------------------------- 
/// 参数值
template<class TKey, class TValue>
struct ArgValue
{
    ArgValue() : Key(), Value() {}

    TKey Key;
    TValue Value;
};
//--------------------------------------------------------- 
/// 参数解析器接口
template<class TKey, class TValue>
class IArgParser
{
public:
    //----------------------------------------------------- 
    /// 键类型
    typedef TKey KeyType;
    /// 值类型
    typedef TValue ValueType;
    /// 参数类型
    typedef ArgValue<TKey, TValue> ArgType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 参数列表,使用shared_obj减少拷贝构造的开销
    shared_obj<list<ArgType> > _args;
    /// 当前键
    typename list<ArgType>::iterator _itr;
    /// 键值比较函数
    virtual bool _Compare(const TKey& k1, const TKey& k2)
    {
        return k1 == k2;
    }
    /**
     * @brief 在所有子项中查找是否有指定键值的配置项 
     * @param [in] key 需要查找的键值 
     */
    virtual typename list<ArgType>::iterator _Find(const TKey& key)
    {
        typename list<ArgType>::iterator itr;
        for(itr = _args.obj().begin();itr != _args.obj().end(); ++itr)
        {
            if(_Compare(key, itr->Key))
                return itr;
        }

        return _args.obj().end();
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 获取数据
    ArgType operator[](const TKey& key)
    {
        typename list<ArgType>::iterator itr = _Find(key);
        if(itr == _args.obj().end())
            return ArgType();
        return (*itr);
    }
    /// 获取子项对应键的值
    bool GetValue(const TKey& key, TValue& val)
    {
        typename list<ArgType>::iterator itr = _Find(key);
        if(itr == _args.obj().end())
            return false;
        val = itr->Value;
        return true;
    }
    /// 获取相同键的所有值
    size_t GetValue(const TKey& key, list<TValue>& vals)
    {
        typename list<ArgType>::iterator itr;
        size_t count = 0;
        for(itr = _args.obj().begin();itr != _args.obj().end(); ++itr)
        {
            if(_Compare(key, itr->Key))
            {
                vals.push_back(itr->Value);
                ++count;
            }
        }
        return count;
    }
    /// 返回是否包含指定项
    bool Contains(const TKey& key)
    {
        return _Find(key) != _args.obj().end();
    }
    /// 枚举键和值
    bool EnumValue(TValue* pVal, TKey* pKey = NULL)
    {
        if(Count() < 1)
            return false;

        if(pVal == NULL)
        {
            _itr = _args.obj().begin();
            return false;
        }

        if(_itr == _args.obj().end())
            return false;
        if(pVal != NULL) (*pVal) = _itr->Value;
        if(pKey != NULL) (*pKey) = _itr->Key;
        ++_itr;

        return true;
    }
    /// 获取项
    size_t Count() const { return _args.obj().size(); }
    /// 清空当前解析到的数据 
    void Clear()
    {
        _args.obj().clear();
        _itr = _args.obj().begin();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 针对string类型的模板特化
template<class TKey>
struct ArgValue<TKey, string>
{
    ArgValue() : Key(), Value("") {}

    TKey Key;
    string Value;

    template<class T>
    T To(const T& defaultVal = T())
    {
        // 空的值
        if(Value.length() < 1)
            return defaultVal;
        return ArgConvert::FromString<T>(Value);
    }
};
//--------------------------------------------------------- 
/// 默认的参数配置解析及转换器 
class ArgParser : public IArgParser<string, string>
{
public:
    //----------------------------------------------------- 
    typedef IArgParser<string, string>::KeyType KeyType;
    typedef IArgParser<string, string>::ValueType ValueType;
    typedef IArgParser<string, string>::ArgType ArgType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 解析单个结点的配置信息str="[  Port :  <COM1>  ]"
    bool _ParseValue(const ByteArray& src, ArgType& node)
    {
        // 至少有2个字符 :数据
        if(src.GetLength() < 2)
            return false;
        ByteArray buf = StringConvert::Trim(src.SubArray(1, src.GetLength() - 2));
        size_t equal = StringConvert::IndexOf(buf, ':');
        if(equal == SIZE_EOF)
            return false;
        // key
        ByteBuilder tmp(8);
        tmp = buf.SubArray(0, equal);
        StringConvert::Trim(tmp);
        node.Key = tmp.GetString();

        // 跳过 ':'
        buf = buf.SubArray(equal + 1, buf.GetLength() - equal - 1);
        ByteArray valArray = StringConvert::Trim(buf);
        if(!valArray.IsEmpty() && valArray[0] == '<')
        {
            ByteArray midArray = StringConvert::Middle(buf, '<', '>');
            if(!midArray.IsEmpty())
            {
                valArray = midArray.SubArray(1, midArray.GetLength() - 2);
            }
        }
        // value
        tmp = valArray;
        node.Value = tmp.GetString();
        return true;
    }
    /// 解析整个配置字符串(找到正确的[]配对) 
    void _Parse(const ByteArray& src)
    {
        ByteArray buf = StringConvert::Trim(src);
        if(buf.GetLength() < 2)
            return;
        if(buf[0] != '[' || buf[buf.GetLength() - 1] != ']')
            return;

        ByteArray subBuf;
        list<ArgType>::iterator itr;
        size_t offset = 0;
        bool isParse = true;
        while(offset < buf.GetLength())
        {
            _args.obj().push_back(ArgType());
            subBuf = StringConvert::Middle(buf.SubArray(offset), '[', ']');
            if(!_ParseValue(subBuf, _args.obj().back()))
            {
                _args.obj().pop_back();
                break;
            }
            
            offset += subBuf.GetLength();
        }
    }
    virtual bool _Compare(const string& k1, const string& k2)
    {
        ByteArray k1Array(k1.c_str(), k1.length());
        ByteArray k2Array(k2.c_str(), k2.length());

        return StringConvert::Compare(k1Array, k2Array, IsIgnoreCase);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ArgParser() : IArgParser() { IsIgnoreCase = true; }
    //-----------------------------------------------------
    /// 解析配置参数字符串  
    size_t Parse(const ByteArray& val)
    {
        _Parse(val);
        _itr = _args.obj().begin();
        return _args.obj().size();
    }
    /// 是否忽略大小写比较
    bool IsIgnoreCase;
    //-----------------------------------------------------
};// class ArgParser
//---------------------------------------------------------
namespace ArgConvert {
//--------------------------------------------------------- 
/**
 * @brief 转换ArgParser中的配置项
 * @param [in] cfg 配置项
 * @param [in] key 主键
 * @param [out] val 找到后的数据
 * @param [in] ignoreCase [default:true] 是否忽略大小写
 * @retval bool
 * @return 是否成功找到
 */
template<class T>
bool FromConfig(IArgParser<string, string>& cfg, const char* key, T& val)
{
    string tmp;
    if(cfg.GetValue(key, tmp))
        return FromString<T>(tmp, val);
    return false;
}
//----------------------------------------------------- 
/**
 * @brief 将指定格式的数据转换为ArgParser能够识别的项
 * @param [in] key 转换的主键
 * @param [in] val 需要转换的值
 * @return string 转换后的字符串
 */
template<class T>
string ToConfig(const char* key, const T& val)
{
    string tmp = "[";
    tmp += key;
    tmp += ":<";
    tmp += ToString<T>(val);
    tmp += ">]";

    return tmp;
}
//--------------------------------------------------------- 
} // namespace ArgConvert
//--------------------------------------------------------- 
} // namespace base
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ARGPARSER_H_
//=========================================================
