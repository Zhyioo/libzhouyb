//========================================================= 
/**@file ICBC_XmlPrinter.h
 * @brief 
 * 
 * @date 2016-06-12   13:42:08
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICBC_XMLPRINTER_H_
#define _LIBZHOUYB_ICBC_XMLPRINTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../xml/xml_extractor.h"
using zhou_yb::xml_extractor::XmlProperty;
using zhou_yb::xml_extractor::IXmlParser;
using zhou_yb::xml_extractor::XmlParser;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace printer {
//--------------------------------------------------------- 
XML_PROPERTY_ID(int, ColumnSpan);
XML_PROPERTY_ID(int, RowSpan);
XML_PROPERTY_ID(int, TabSpan);
XML_PROPERTY_ID(int, XSize);
XML_PROPERTY_ID(int, YSize);
XML_PROPERTY_ID(string, Text);
XML_PROPERTY_ID(string, Source);
XML_PROPERTY_ID(int, Height);
XML_PROPERTY_ID(int, Width);
XML_PROPERTY_ID(int, Left);
XML_PROPERTY_ID(bool, IsShowTop);
XML_PROPERTY_ID(bool, IsShowBottom);
XML_PROPERTY_ID(string, Code);
XML_PROPERTY_ID(string, CodeMode);
XML_PROPERTY_ID(int, Count);
XML_PROPERTY_ID(string, Alignment);
XML_PROPERTY_ID(int, Angle);
//--------------------------------------------------------- 
struct IXmlObject
{
    virtual bool IsObject(const char* xmlVal) = 0;
    virtual bool FromXML(IXmlParser& xml, Ref<IInteractiveTrans> dev) = 0;
};
class XmlSetter : public IXmlObject
{
public:
    virtual bool IsObject(const char* xmlVal)
    {
        return strcmp(xmlVal, "Setter") == 0;
    }
    virtual bool FromXML(IXmlParser& xml, Ref<IInteractiveTrans> dev)
    {
        int iVal = 0;
        ByteBuilder cmd(8);
        // 设置字间距 
        if(xml.Get<ColumnSpanProperty>(iVal))
        {
            cmd.Clear();
            DevCommand::FromAscii("1B 20", cmd);
            cmd += (byte)iVal;
            dev->Write(cmd);
        }
        // 设置行间距 
        if(xml.Get<RowSpanProperty>(iVal))
        {
            cmd.Clear();
            DevCommand::FromAscii("1B 31", cmd);
            cmd += (byte)iVal;
            dev->Write(cmd);
        }
        // 设置行列制表符 
        if(xml.Get<TabSpanProperty>(iVal))
        {
            cmd.Clear();
            DevCommand::FromAscii("1B 44", cmd);
            cmd += (byte)iVal;
            cmd += (byte)0x00;
            dev->Write(cmd);
        }
        // 字体放大 
        int x = 1;
        int y = 1;
        if(xml.Get<XSizeProperty>(x) || xml.Get<YSizeProperty>(y))
        {
            cmd.Clear();
            DevCommand::FromAscii("1B 58", cmd);
            cmd += (byte)x;
            cmd += (byte)y;
            dev->Write(cmd);
        }
        // 设置对齐方式 
        string align = "";
        if(xml.Get<AlignmentProperty>(align))
        {
            int nMode = -1;
            if(align == "Right")
            {
                nMode = 2;
            }
            else if(align == "Middle")
            {
                nMode = 1;
            }
            else if(align == "Left")
            {
                nMode = 0;
            }
            if(nMode >= 0)
            {
                cmd.Clear();
                DevCommand::FromAscii("1B 61", cmd);
                cmd += nMode;
                dev->Write(cmd);
            }
        }
        // 旋转字体 
        int angle = 0;
        if(xml.Get<AngleProperty>(angle))
        {
            int mode = (int)(angle / 90) % 4;
            cmd.Clear();
            DevCommand::FromAscii("1C 49", cmd);
            cmd += _itobyte(mode);
            dev->Write(cmd);
        }

        return true;
    }
};
class XmlEnter : public IXmlObject
{
public:
    virtual bool IsObject(const char* xmlVal)
    {
        return strcmp(xmlVal, "Enter") == 0;
    }
    virtual bool FromXML(IXmlParser& xml, Ref<IInteractiveTrans> dev)
    {
        int y = 1;
        xml.Get<CountProperty>(y);

        ByteBuilder cmd(8);
        for(int i = 0;i < y; ++i)
            cmd += (byte)0x0D;
        dev->Write(cmd);

        return true;
    }
};
class XmlTab : public IXmlObject
{
public:
    virtual bool IsObject(const char* xmlVal)
    {
        return strcmp(xmlVal, "Tab") == 0;
    }
    virtual bool FromXML(IXmlParser& xml, Ref<IInteractiveTrans> dev)
    {
        int x = 1;
        xml.Get<CountProperty>(x);

        ByteBuilder cmd(8);
        DevCommand::FromAscii("1B 44", cmd);
        cmd += _itobyte(x);
        cmd += (byte)0x00;
        cmd += (byte)0x09;
        dev->Write(cmd);

        return true;
    }
};
class XmlString : public IXmlObject
{
public:
    virtual bool IsObject(const char* xmlVal)
    {
        return strcmp(xmlVal, "String") == 0;
    }
    virtual bool FromXML(IXmlParser& xml, Ref<IInteractiveTrans> dev)
    {
        string text = "";
        if(xml.Get<TextProperty>(text) && text.length() > 0)
        {
            XmlSetter().FromXML(xml, dev);
            Timer::Wait(DEV_OPERATOR_INTERVAL);
            ByteBuilder cmd(8);
            DevCommand::FromAscii("1B 38", cmd);
            cmd += text.c_str();
            dev->Write(cmd);
        }

        return true;
    }
};
class XmlBarcode : public IXmlObject
{
public:
    virtual bool IsObject(const char* xmlVal)
    {
        return strcmp(xmlVal, "BarCode") == 0;
    }
    virtual bool FromXML(IXmlParser& xml, Ref<IInteractiveTrans> dev)
    {
        string barCode = "";
        if(!xml.Get<CodeProperty>(barCode) || barCode.length() < 1)
            return true;

        int iVal = 30;
        ByteBuilder cmd(8);
        xml.Get<HeightProperty>(iVal);
        DevCommand::FromAscii("1D 68", cmd);
        cmd += (byte)iVal;
        dev->Write(cmd);

        cmd.Clear();
        iVal = 3;
        xml.Get<WidthProperty>(iVal);
        DevCommand::FromAscii("1D 77", cmd);
        cmd += (byte)iVal;
        dev->Write(cmd);

        cmd.Clear();
        string barMode = "";
        if(!xml.Get<CodeModeProperty>(barMode))
            barMode = "None";

        int iMode = 0;
        if(barMode == "Top")
            iMode = 1;
        else if(barMode == "Bottom")
            iMode = 2;
        else
            iMode = 0;
        DevCommand::FromAscii("1D 48", cmd);
        cmd += (byte)iMode;
        dev->Write(cmd);

        cmd.Clear();
        iVal = 5;
        xml.Get<LeftProperty>(iVal);
        DevCommand::FromAscii("1D 51", cmd);
        cmd += (byte)iVal;
        cmd += (byte)0x00;
        dev->Write(cmd);

        cmd.Clear();
        DevCommand::FromAscii("1D 6B 45", cmd);
        cmd += (byte)barCode.length();
        cmd += barCode.c_str();
        cmd += (byte)0x0D;
        dev->Write(cmd);

        return true;
    }
};
//--------------------------------------------------------- 
/// XML打印机
class XmlPrinter : DevAdapterBehavior<IInteractiveTrans>
{
protected:
    /// 支持打印的元素列表
    list<IXmlObject*> _xmlObject;
    /// 递归打印元素
    bool _PrintElement(TiXmlElement* pXml)
    {
        const char* pVal = pXml->Value();
        if(pVal == NULL || strlen(pVal) < 1)
            return true;
        list<IXmlObject*>::iterator itr;
        for(itr = _xmlObject.begin(); itr != _xmlObject.end(); ++itr)
        {
            if((*itr)->IsObject(pVal))
            {
                XmlParser xml(pXml);

                // 只要格式符合就打印 
                (*itr)->FromXML(xml, _pDev);
                break;
            }
        }
        TiXmlElement *pXmlSibling = pXml->NextSiblingElement();
        if(pXmlSibling != NULL)
        {
            if(!_PrintElement(pXmlSibling))
                return false;
        }

        return true;
    }
public:
    /// 增加支持的打印元素
    template<class T>
    void Add()
    {
        _xmlObject.push_back(new T());
    }
    /// 增加支持的打印元素
    template<class T, class TArg>
    void Add(const TArg& arg)
    {
        _xmlObject.push_back(new T(arg));
    }
    /**
     * @brief 打印XML数据
     * @date 2016-06-12 14:03
     * 
     * @param [in] xmlstr XML文件名或字符串 
     * @param [in] dev 打印机
     */
    bool Print(const char* xmlstr)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        TiXmlDocument xml;
        ByteArray xmlName(xmlstr);
        if(StringConvert::EndWith(xmlName, ByteArray(".xml"), true))
        {
            ASSERT_FuncErrInfoRet(xml.LoadFile(xmlName.GetString()), DeviceError::DevInitErr,
                "加载XML文件失败");
        }
        else
        {
            // 直接解析XML字符串数据 
            xml.Parse(xmlName.GetString());
        }

        TiXmlElement* pXml = xml.RootElement();
        ASSERT_FuncErrRet(pXml != NULL, DeviceError::ArgFormatErr);

        // 处理子元素 
        TiXmlElement* pXmlChild = pXml->FirstChildElement();
        if(pXmlChild != NULL)
            _PrintElement(pXmlChild);

        return _logRetValue(true);
    }
    /// 清除需要打印的配置数据
    void Clean()
    {
        list<IXmlObject*>::iterator itr;
        for(itr = _xmlObject.begin();itr != _xmlObject.end(); ++itr)
            delete (*itr);
        _xmlObject.clear();
    }
};
//--------------------------------------------------------- 
} // namespace printer
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICBC_XMLPRINTER_H_
//========================================================= 