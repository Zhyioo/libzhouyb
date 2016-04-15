//========================================================= 
/**@file TlvElement.cpp 
 * @brief device目录下ICCardTagElement库中全局变量、函数的实现
 * 
 * @date 2012-06-23   11:41:11 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TLVELEMENT_CPP_
#define _LIBZHOUYB_TLVELEMENT_CPP_
//--------------------------------------------------------- 
#include "../../device/iccard/pboc/base/TlvElement.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace base {
//---------------------------------------------------------
/// 错误的标签头格式 
const ushort TlvConvert::ERROR_TAG_HEADER = 0;
//---------------------------------------------------------
/// 错误的标签头格式 
const ushort TlvHeader::ERROR_TAG_HEADER = TlvConvert::ERROR_TAG_HEADER;
//--------------------------------------------------------- 
} // namespace base 
} // namespace pboc 
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TLVELEMENT_CPP_
//=========================================================