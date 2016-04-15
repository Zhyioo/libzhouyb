//========================================================= 
/**@file HidICC_TestContainer.h 
 * @brief HID IC卡接口转换器 
 *
 * 将HID读卡器根据农行指令集进行封装为IC卡接口的设备 
 * 
 * @date 2015-01-24   19:36:58 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDICC_TESTCONTAINER_H_
#define _LIBZHOUYB_HIDICC_TESTCONTAINER_H_
//--------------------------------------------------------- 
#include "ComICC_TestContainer.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// HID层指令集协议适配器 
typedef TestAdapterContainer<HidDevice, IInteractiveTrans, HidICCardCmdAdapter> HidICCardBaseCmdTestContainer;
/// COM层指令集协议适配器 
typedef TestAdapterExtractorContainer<HidICCardBaseCmdTestContainer, IInteractiveTrans, ComICCardCmdAdapter> HidICCardCmdTestContainer;
/// HidDevice转接触式读卡器适配器 
typedef TestAdapterExtractorContainer<HidICCardCmdTestContainer, IICCardDevice, ComContactICCardDevAdapter> HidContactIC_TestContainer;
/// HidDevice转非接触式读卡器适配器 
typedef TestAdapterExtractorContainer<HidICCardCmdTestContainer, IICCardDevice, ComContactlessICCardDevAdapter> HidContactlessIC_TestContainer;
/// HidDevice转PSAM读卡器适配器 
typedef TestAdapterExtractorContainer<HidICCardCmdTestContainer, IICCardDevice, ComPsamICCardDevAdapter> HidPsamIC_TestContainer;
/// HidDevice转AT88SC102接口适配器 
typedef ComTAT88SC102_TestContainer<HidICCardCmdTestContainer> HidAT88SC102_TestContainer;
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDICC_TESTCONTAINER_H_
//========================================================= 