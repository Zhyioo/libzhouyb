//========================================================= 
/**@file ini.cpp 
 * @brief base目录下ini库中全局函数、变量的实现 
 * 
 * @date 2012-06-23   11:17:14 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_INI_CPP_
#define _LIBZHOUYB_INI_CPP_
//--------------------------------------------------------- 
#include "../../container/ini.h"
using namespace zhou_yb;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
string IniItem::EmptyValue = "";
IniItem IniGroup::EmptyItem;
IniGroup IniFile::EmptyGroup;
//--------------------------------------------------------- 
} // namespace container 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_INI_CPP_
//=========================================================