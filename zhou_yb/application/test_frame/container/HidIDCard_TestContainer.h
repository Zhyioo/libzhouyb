//========================================================= 
/**@file HidIDCard_TestContainer.h
 * @brief 
 * 
 * @date 2016-04-30   15:50:28
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef __HIDIDCARD_TESTCONTAINER_H_
#define __HIDIDCARD_TESTCONTAINER_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/Device.h"

#include "../../idcard/IDCardDevAdapter.h"
using zhou_yb::application::idcard::IDCardDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// HID设备层指令集协议适配器 
typedef TestAdapterContainer<TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> >, IInteractiveTrans, IDCardDevAdapter> HidIDCardCmdTestContainer;
/// HID设备应用层指令集协议适配器
typedef TestAdapterExtractorContainer<HidIDCardCmdTestContainer, IInteractiveTrans, IDCardDevAdapter> HidIDCardTestContainer;
/// HidDevice转Sdtapi读卡器适配器 
typedef TestAdapterExtractorContainer<HidIDCardTestContainer, ISdtApi, SdtApiDevAdapter> HidSdtapiTestContainer;
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // __HIDIDCARD_TESTCONTAINER_H_
//========================================================= 