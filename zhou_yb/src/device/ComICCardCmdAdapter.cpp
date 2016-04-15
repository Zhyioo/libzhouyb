//========================================================= 
/**@file ComICCardCmdAdapter.cpp 
 * @brief device目录下ComICCardCmdAdapter库中全局变量、函数的实现
 * 
 * @date 2012-06-23   11:17:39 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMICCARDCMDADAPTER_CPP_
#define _LIBZHOUYB_COMICCARDCMDADAPTER_CPP_
//--------------------------------------------------------- 
#include "../../device/cmd_adapter/ComICCardCmdAdapter.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//---------------------------------------------------------
/// 数据头标识 
const byte ComICCardCmdAdapter::STX = 0x02;
/// 报文尾标识 
const byte ComICCardCmdAdapter::ETX = 0x03;
/// 扩展填充的字节 
const byte ComICCardCmdAdapter::ExBYTE = 0x30;
//---------------------------------------------------------
} // namespace cmd_adapter
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMICCARDCMDADAPTER_CPP_
//=========================================================