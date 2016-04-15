//========================================================= 
/**@file CCID_Device.cpp 
 * @brief base_device目录下CCID_Device库中全局函数、变量的实现
 * 
 * @date 2012-06-23   11:21:34 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_DEVICE_CPP_
#define _LIBZHOUYB_CCID_DEVICE_CPP_
//--------------------------------------------------------- 
#include "../../base_device/CCID_Device.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
//--------------------------------------------------------- 
/// CCID_Device.PowerOn参数配置项主键 DevReader
const char CCID_Device::DevReaderKey[] = "DevReader";

#ifdef SCARD_USING_SHARED_CONTEXT
SCARDCONTEXT CCID_Device::_scardContext = NULL;
size_t CCID_Device::_use_count = 0;
#endif
//--------------------------------------------------------- 
} // base_device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_DEVICE_CPP_
//=========================================================
