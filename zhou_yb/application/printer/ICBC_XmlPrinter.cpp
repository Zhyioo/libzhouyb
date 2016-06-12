//========================================================= 
/**@file ICBC_XmlPrinter.cpp
 * @brief 
 * 
 * @date 2016-06-12   13:42:03
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICBC_XMLPRINTER_CPP_
#define _LIBZHOUYB_ICBC_XMLPRINTER_CPP_
//--------------------------------------------------------- 
#include "ICBC_XmlPrinter.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace printer {
//--------------------------------------------------------- 
XML_PROPERTY_SRC(int, ColumnSpan);
XML_PROPERTY_SRC(int, RowSpan);
XML_PROPERTY_SRC(int, TabSpan);
XML_PROPERTY_SRC(int, XSize);
XML_PROPERTY_SRC(int, YSize);
XML_PROPERTY_SRC(string, Text);
XML_PROPERTY_SRC(string, Source);
XML_PROPERTY_SRC(int, Height);
XML_PROPERTY_SRC(int, Width);
XML_PROPERTY_SRC(int, Left);
XML_PROPERTY_SRC(bool, IsShowTop);
XML_PROPERTY_SRC(bool, IsShowBottom);
XML_PROPERTY_SRC(string, Code);
XML_PROPERTY_SRC(string, CodeMode);
XML_PROPERTY_SRC(int, Count);
XML_PROPERTY_SRC(string, Alignment);
XML_PROPERTY_SRC(int, Angle);
//--------------------------------------------------------- 
} // namespace printer
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICBC_XMLPRINTER_CPP_
//========================================================= 