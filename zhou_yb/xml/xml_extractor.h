//========================================================= 
/**@file xml_extractor.h 
 * @brief XML元素属性萃取器 
 * 
 * @date 2015-01-30   22:45:58 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include <string>
#include <string.h>
using std::string;
using std::wstring;

#include <sstream>
using std::ostringstream;
using std::istringstream;

#include "tinyxml/tinyxml.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace xml {
//--------------------------------------------------------- 
/// XML属性声明+定义宏
#define XML_PROPERTY(type, name) \
    struct name##XmlID { static const char XmlID[32]; }; \
    const char name##XmlID::XmlID[] = #name; \
    typedef XmlProperty<type, name##XmlID> name##Property
//--------------------------------------------------------- 
/// XML中数据转换器 
template<class T>
struct XmlConvert
{
    bool ConvertFrom(const char* sVal, T& Val)
    {
        string tmp = sVal;
        istringstream sstream(tmp);
        sstream >> Val;

        return true;
    }
    bool ConvertTo(const T& Val, string& sVal)
    {
        ostringstream sstream;
        sstream << Val;
        sVal = sstream.str();

        return true;
    }
};
//--------------------------------------------------------- 
//@{
/**@name
* @brief 对于各种转换器的特化版本
*/
/// string转换器 
template<>
struct XmlConvert < string >
{
    bool ConvertFrom(const char* sVal, string& Val)
    {
        Val = sVal;
        return true;
    }
    bool ConvertTo(const string& Val, string& sVal)
    {
        sVal = Val;
        return true;
    }
};
/// bool转换器 
template<>
struct XmlConvert < bool >
{
    bool ConvertFrom(const char* sVal, bool& Val)
    {
        Val = StringConvert::IsEqual(sVal, "true", true);
        return true;
    }
    bool ConvertTo(const bool& Val, string& sVal)
    {
        if(Val)
            sVal = "true";
        else
            sVal = "false";

        return true;
    }
};
//@}
//--------------------------------------------------------- 
/**
 * @brief XML中属性转换,可将对应的属性值从XML中读取出来或者序列化到XML文件中
 * @param [in] T 数据类型
 * @param [in] TXmlID XML文件中的ID,需要有静态 const char* XmlID 属性
 * @param [in] TConvert XML数据格式转换器
 */
template<class T, class TXmlID, class TConvert = XmlConvert<T> >
class XmlProperty
{
public:
    typedef T PropertyType;
    typedef TXmlID PropertyID;
    typedef TConvert PropertyConvert;
    /// 从XML文件里面解析 
    static bool FromXml(TiXmlElement* pXml, T& Val)
    {
        const char* pVal = pXml->Attribute(TXmlID::XmlID);
        if(pVal == NULL || strlen(pVal) < 1)
            return false;
        return TConvert().ConvertFrom(pVal, Val);
    }
    /// 写入到XML文件 
    static bool ToXml(TiXmlElement* pXml, const T& Val)
    {
        string sVal;
        if(!TConvert().ConvertTo(Val, sVal))
            return false;
        pXml->SetAttribute(TXmlID::XmlID, sVal.c_str());

        return true;
    }
};
//--------------------------------------------------------- 
/// XML资源转换器 
class IXmlParser
{
protected:
    TiXmlElement* _pXml;
    virtual string _XmlValue(const char* xmlID) = 0;
public:
    IXmlParser(TiXmlElement* pXml = NULL)
    {
        _pXml = pXml;
    }

    inline void SetXml(TiXmlElement* pXml)
    {
        _pXml = pXml;
    }
    inline TiXmlElement* GetXml() const
    {
        return _pXml;
    }

    template<class T>
    bool Get(const char* xmlID, T& val)
    {
        string sVal = _XmlValue(xmlID);
        if(sVal == "")
            return false;
        return XmlConvert<T>().ConvertFrom(sVal.c_str(), val);
    }

    template<class T>
    T GetDefault(const char* xmlID, const T& defVal = T())
    {
        T Val;
        if(GetValue(xmlID, Val))
            return Val;
        return defVal;
    }
    
    template<class TProperty>
    bool Get(typename TProperty::PropertyType& Val)
    {
        typedef typename TProperty::PropertyID _XmlID;
        typedef typename TProperty::PropertyConvert _XmlConvert;

        string sVal = _XmlValue(_XmlID::XmlID);
        if(sVal == "")
            return false;

        return _XmlConvert().ConvertFrom(sVal.c_str(), Val);
    }

    template<class TProperty>
    typename TProperty::PropertyType GetDefault(const typename TProperty::PropertyType& defVal = typename TProperty::PropertyType())
    {
        typename TProperty::PropertyType Val;
        if(Get<TProperty>(Val))
            return Val;
        return defVal;
    }
};
//--------------------------------------------------------- 
/// 默认的XML解析器 
class XmlParser : public IXmlParser
{
protected:
    virtual string _XmlValue(const char* xmlID)
    {
        return _strput(_pXml->Attribute(xmlID));
    }
public:
    XmlParser(TiXmlElement* pXml = NULL) : IXmlParser(pXml) {}
};
//--------------------------------------------------------- 
} // namespace xml
} // namespace zhou_yb
//========================================================= 
