//========================================================= 
/**@file MagneticDevAdapter.cpp 
 * @brief device目录下magnetic库中全局变量、函数的实现
 * 
 * @date 2014-10-17   19:30:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../device/magnetic/MagneticDevAdapter.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace magnetic {
//---------------------------------------------------------
/// 一磁道数据最大长度
const size_t MagneticDevAdapter::MaxTrack1Len = 79;
/// 75Bpi二磁道数据最大长度
const size_t MagneticDevAdapter::Max75BpiTrack2Len = 38;
/// 210Bpi二磁道数据最大长度
const size_t MagneticDevAdapter::Max210BpiTrack2Len = 108;
/// 三磁道数据最大长度
const size_t MagneticDevAdapter::MaxTrack3Len = 108;
/// 一磁道数据起始标识符
const byte MagneticDevAdapter::Track1Flag = 0x25;
/// 三磁道数据起始标识符
const byte MagneticDevAdapter::Track3Flag = 0x41;
/// 磁条数据结束符(写磁条时)
const byte MagneticDevAdapter::TrackEndFlag = 0x3F;
/// 磁条数据读取错误标识符
const byte MagneticDevAdapter::TrackErrFlag = 0x7F;
/// 设备操作回送命令错误标识符
const byte MagneticDevAdapter::DevRetErrFlag = 0x71;
//---------------------------------------------------------
} // namespace magnetic 
} // namespace device 
} // namespace zhou_yb
//=========================================================