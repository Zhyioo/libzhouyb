//========================================================= 
/**@file ComICCardDevAdapter.cpp 
 * @brief device目录下ComICCardDevAdapter库中全局变量、函数的实现
 * 
 * @date 2012-06-23   11:16:58 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../device/iccard/device/ComICCardDevAdapter.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace device {
//---------------------------------------------------------
/// IC卡PowerOn上电配置项参数主键 卡槽 Slot
const char ComContactICCardBaseDevAdapter::SlotKey[] = "Slot";
/// IC卡PowerOn上电配置项参数主键 等待相应时间 DelayTime 
const char ComContactICCardBaseDevAdapter::DelayTimeKey[] = "DelayTime";
//---------------------------------------------------------
/// 最小卡序号
const byte ComContactlessICCardDevAdapter::CardNumber = 0xFF;
//---------------------------------------------------------
} // namespace device 
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//=========================================================