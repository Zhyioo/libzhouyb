//========================================================= 
/**@file LC_ComStreamCmdAdapter.cpp
 * @brief LC_ComStreamCmdAdapter静态数据成员值定义
 * 
 * @date 2016-01-09   12:11:11
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_COMSTREAMCMDADAPTER_CPP_
#define _LIBZHOUYB_LC_COMSTREAMCMDADAPTER_CPP_
//--------------------------------------------------------- 
#include "../inc/LC_ComStreamCmdAdapter.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace application {
namespace lc {
//--------------------------------------------------------- 
/// ACK应答数据指令
const byte LC_ComStreamCmdAdapter::_ACK_COMMAND[] = { 0xA0, 0x00, 0xA0 };
/// ACK应答数据
const ByteArray LC_ComStreamCmdAdapter::ACK_COMMAND(_ACK_COMMAND, 3);
/// 包头
const byte LC_ComStreamCmdAdapter::ACK_STX = 0x80;
//--------------------------------------------------------- 
} // namespace lc
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_COMSTREAMCMDADAPTER_CPP_
//========================================================= 
