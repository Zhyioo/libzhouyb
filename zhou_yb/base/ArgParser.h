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

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;
using std::istringstream;
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//---------------------------------------------------------
/// 参数配置解析及转换器 
class ArgParser
{
protected:
    //----------------------------------------------------- 
    /// 配置项的结点 
    struct cfg_node
    {
        /// 键值
        string _key;
        /// 值
        string _value;
    };
    /// 配置项列表
    list<cfg_node> _cfg;
    /// 枚举的当前项 
    list<cfg_node>::iterator _itr;
    //----------------------------------------------------- 
    /// 解析单个结点的配置信息str="[  Port :  <COM1>  ]"
    bool _parse_node(const ByteArray& src, cfg_node& node)
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
        // 删除 ':' 号
        tmp.RemoveTail();
        StringConvert::Trim(tmp);
        node._key = tmp.GetString();

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
        node._value = tmp.GetString();
        return true;
    }
    //----------------------------------------------------- 
    /// 解析整个配置字符串(找到正确的[]配对) 
    void _parse(const ByteArray& src)
    {
        ByteArray buf = StringConvert::Trim(src);
        if(buf.GetLength() < 2)
            return;
        if(buf[0] != '[' || buf[buf.GetLength() - 1] != ']')
            return;

        ByteArray subBuf;
        list<cfg_node>::iterator itr;
        size_t offset = 0;
        bool isParse = true;
        while(offset < buf.GetLength())
        {
            cfg_node node;
            subBuf = StringConvert::Middle(buf.SubArray(offset), '[', ']');
            if(!_parse_node(subBuf, node))
                break;
            
            offset += subBuf.GetLength();
            for(itr = _cfg.begin();itr != _cfg.end(); ++itr)
            {
                // 键值相同则重新赋值
                if(itr->_key == node._key)
                {
                    itr->_value = node._value;
                    break;
                }
            }
            // 不是重复项
            if(itr == _cfg.end())
            {
                _cfg.push_back(node);
            }
        }
    }
    //----------------------------------------------------- 
    /**
     * @brief 在所有子项中查找是否有指定键值的配置项 
     * @param [in] key 需要查找的键值 
     * @param [in] ignoreCase [default:true] 是否忽略大小写 
     */
    list<cfg_node>::const_iterator _find(const char* key, bool ignoreCase = true) const
    {
        if(NULL == key)
            return _cfg.end();
        list<cfg_node>::const_iterator itr;
        for(itr = _cfg.begin();itr != _cfg.end(); ++itr)
        {
            // 判断时候需要忽略大小写进行比较 
            if(ignoreCase)
            {
                if(StringConvert::Compare(key, ByteArray(itr->_key.c_str(), itr->_key.length()), true))
                    return itr;
            }
            else
            {
                /* 直接比较 string.opreator ==(string& other)*/
                if(key == itr->_key)
                    return itr;
            }
        }

        return _cfg.end();
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ArgParser() { _itr = _cfg.begin(); }
    //-----------------------------------------------------
    /// 解析配置参数字符串  
    size_t Parse(const char* val)
    {
        _cfg.clear();
        _parse(val);
        _itr = _cfg.begin();
        return _cfg.size();
    }
    //-----------------------------------------------------
    /**
     * @brief 获取键值
     * @param [in] key 键值
     * @param [out] val 获取到的值
     * @param [in] ignoreCase [default:true] 是否对键值比较采用大小写忽略 
     * @retval bool
     * @return 
     */
    bool GetValue(const char* key, string& val, bool ignoreCase = true) const
    {
        list<cfg_node>::const_iterator itr;

        itr = _find(key, ignoreCase);
        if(itr != _cfg.end())
        {
            val = itr->_value.c_str();
            return true;
        }

        return false;
    }
    //----------------------------------------------------- 
    /**
     * @brief 获取键值
     * @param [in] key 键值
     * @param [in] ignoreCase [default:true] 是否对键值比较采用大小写忽略 
     * @retval const char* const
     * @return 获取到的键值字符串 
     */
    const char* const GetValue(const char* key, bool ignoreCase = true) const
    {
        list<cfg_node>::const_iterator itr;

        itr = _find(key, ignoreCase);
        if(itr != _cfg.end())
            return itr->_value.c_str();
        return "";
    }
    //----------------------------------------------------- 
    /**
     * @brief 枚举所有的值
     * @param [out] val 获取到的值(为NULL表示定位到开头) 
     * @param [out] key [default:NULL] 获取到的键值(为空表示不需要获取) 
     * @retval bool
     * @return 是否还有下一个值
     */
    bool EnumValue(string* pVal, string* pKey = NULL)
    {
        if(Count() < 1)
            return false;

        if(pVal == NULL)
            _itr = _cfg.begin();

        if(_itr == _cfg.end())
            return false;
        if(pVal != NULL) (*pVal) = _itr->_value;
        if(pKey != NULL) (*pKey) = _itr->_key;
        ++_itr;

        return true;
    }
    //-----------------------------------------------------
    /// 获取配置项的数目 
    inline size_t Count() const
    {
        return _cfg.size();
    }
    //----------------------------------------------------- 
    /// 清空当前解析到的数据  
    inline void Clear()
    {
        _cfg.clear();
        _itr = _cfg.begin();
    }
    //----------------------------------------------------- 
    /**
     * @brief 返回是否包含指定键值的配置元素
     * @param [in] key 键值
     * @param [in] ignoreCase [default:true] 是否忽略大小写 
     * @retval bool
     * @return 
     */
    bool Contains(const char* key, bool ignoreCase = true) const
    {
        list<cfg_node>::const_iterator itr = _find(key, ignoreCase);
        bool isContains = static_cast<bool>(itr != _cfg.end());
        
        return isContains;
    }
    //----------------------------------------------------- 
    /// 获取指定键的值 
    inline const char* operator[](const char* key) const
    {
        return GetValue(key, true);
    }
    //-----------------------------------------------------
};// class ArgParser
/// 参数转换器(使用命令空间而不使用类方便后续其他代码文件扩展)  
namespace ArgConvert
{
    //----------------------------------------------------- 
    /// 将字符串转换为指定的格式，返回是否成功转换 
    template<class T>
    bool FromString(const char* str, T& val)
    {
        if(_is_empty_or_null(str))
            return false;

        string tmp = str;
        istringstream sstream(tmp);
        sstream>>val;

        return true;
    }
    //----------------------------------------------------- 
    /// 将字符串转换为指定的格式 
    template<class T>
    T FromString(const char* str)
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
    bool FromConfig(const ArgParser& cfg, const char* key, T& val, bool ignoreCase = true)
    {
        string tmp;
        if(cfg.GetValue(key, tmp, ignoreCase))
            return FromString<T>(tmp.c_str(), val);
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
    //----------------------------------------------------- 
    /// 解析str中的pointer数据 
    template<> 
    bool FromString<pointer>(const char* str, pointer& val);
    //----------------------------------------------------- 
    /// 解析str中的ByteBuilder数据 
    template<> 
    bool FromString<ByteBuilder>(const char* str, ByteBuilder& val);
    //----------------------------------------------------- 
    /// 解析str中的bool数据 
    template<>
    bool FromString<bool>(const char* str, bool& val);
    //----------------------------------------------------- 
    /// 解析str中的byte数据
    template<>
    bool FromString<byte>(const char* str, byte& val);
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
} // namespace base
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ARGPARSER_H_
//=========================================================
