//========================================================= 
/**@file SdtApiDevAdapter.cpp 
 * @brief 二代证指令封装资源文件 
 * 
 * @date 2014-12-25   21:19:47 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SDTAPIDEVADAPTER_CPP_
#define _LIBZHOUYB_SDTAPIDEVADAPTER_CPP_
//--------------------------------------------------------- 
#include "../../device/idcard/SdtApiDevAdapter.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace idcard {
//--------------------------------------------------------- 
/// 二代证数据头 
const ByteArray SdtApiDevAdapter::SDT_CMD_PREAMBLE = "\xAA\xAA\xAA\x96\x69";
//--------------------------------------------------------- 
} // namespace idcrad 
} // namespace device 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SDTAPIDEVADAPTER_CPP_
//========================================================= 