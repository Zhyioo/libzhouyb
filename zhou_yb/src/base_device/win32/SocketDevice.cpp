//========================================================= 
/**@file SocketDevice.cpp
 * @brief Windows下socket资源文件 
 * 
 * @date 2015-07-17   22:37:35
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SOCKETDEVICE_CPP_
#define _LIBZHOUYB_SOCKETDEVICE_CPP_
//--------------------------------------------------------- 
#include "../../../base_device/win32/SocketDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// SocketDevice生成参数配置项时的主键 SocketAfKey
const char SocketHandlerFactory::SocketAfKey[] = "SocketAf";
/// SocketDevice生成参数配置项时的主键 SocketTypeKey
const char SocketHandlerFactory::SocketTypeKey[] = "SocketType";
/// SocketDevice生成参数配置项时的主键 SocketProtocolKey
const char SocketHandlerFactory::SocketProtocolKey[] = "SocketProtocol";
/// WSA启动标志 
bool SocketHandlerFactory::_isWsaStarted = false;
/// 成员变量引用计数 
size_t SocketHandlerFactory::_useCount = 0;
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SOCKETDEVICE_CPP_
//=========================================================