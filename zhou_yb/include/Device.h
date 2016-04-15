//========================================================= 
/**@file Device.h 
 * @brief 包含device目录下的所有设备
 * 
 * @date 2011-10-17   19:23:23 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_DEVICE_H_
#define _LIBZHOUYB_DEVICE_H_
//--------------------------------------------------------- 
// 适配器
#include "../device/cmd_adapter/CommandFilter.h"
#include "../device/cmd_adapter/CacheCmdAdapter.h"
#include "../device/cmd_adapter/StreamCmdAdapter.h"
#include "../device/cmd_adapter/PinDevCmdAdapter.h"
#include "../device/cmd_adapter/ComICCardCmdAdapter.h"

#ifndef NO_INCLUDE_USB_SOURCE
#   include "../device/cmd_adapter/HidCmdAdapter.h"
#   include "../device/cmd_adapter/HidICCardCmdAdapter.h"
#endif

using namespace zhou_yb::device::cmd_adapter;
//---------------------------------------------------------
// 磁条机
#include "../device/magnetic/MagneticDevAdapter.h"
using namespace zhou_yb::device::magnetic;
//---------------------------------------------------------
#ifndef NO_INCLUDE_IDCARD_SOURCE
// 身份证
#   include "../device/idcard/ISdtApi.h"
#   include "../device/idcard/SdtApiDevAdapter.h"
#   include "../device/idcard/IDCardCharsetConvert.h"
#   include "../device/idcard/IDCardParser.h"
using namespace zhou_yb::device::idcard;
#endif
//---------------------------------------------------------
#ifndef NO_INCLUDE_ICCARD_SOURCE
// IC卡
#include "../device/iccard/base/ICCardLibrary.h"
#include "../device/iccard/base/ICCardCmd.h"
#include "../device/iccard/base/ICCardAppAdapter.h"
// IC卡指令适配器 
#include "../device/iccard/device/ComICCardDevAdapter.h"
#include "../device/iccard/device/ComStorageCardDevAdapter.h"
#include "../device/iccard/device/CCID_StorageCardDevAdapter.h"

#include "../device/iccard/extension/base/IMemoryCard.h"
#include "../device/iccard/extension/device/CCID_AT88SC102.h"
#include "../device/iccard/extension/device/CCID_AT88SC1608.h"
#include "../device/iccard/extension/device/CCID_Mifare1Card.h"
#include "../device/iccard/extension/device/CCID_SLE4428.h"
#include "../device/iccard/extension/device/CCID_SLE4442.h"
#include "../device/iccard/extension/device/ComAT88SC102.h"
#include "../device/iccard/extension/device/ComMifare1Card.h"
#include "../device/iccard/extension/device/ComSLE4428.h"
#include "../device/iccard/extension/device/ComSLE4442.h"

using namespace zhou_yb::device::iccard::base;
using namespace zhou_yb::device::iccard::device;
using namespace zhou_yb::device::iccard::extension::base;
using namespace zhou_yb::device::iccard::extension::device;

/* PBOC标签部分 */
#include "../device/iccard/pboc/base/TlvElement.h"
#include "../device/iccard/pboc/base/TlvEditor.h"

using namespace zhou_yb::device::iccard::pboc::base;

#ifndef NO_INCLUDE_PBOC_SOURCE
/* PBOC1.0部分 */
#   include "../device/iccard/pboc/v1_0/PBOC_v1_0_CMD.h"
#   include "../device/iccard/pboc/v1_0/PBOC_v1_0_AppAdapter.h"
/* PBOC2.0部分 */
#   include "../device/iccard/pboc/v2_0/PBOC_v2_0_CMD.h"
#   include "../device/iccard/pboc/v2_0/PBOC_v2_0_TagMap.h"
#   include "../device/iccard/pboc/v2_0/PBOC_v2_0_Library.h"
#   include "../device/iccard/pboc/v2_0/PBOC_v2_0_AppAdapter.h"
/* PBOC3.0部分 */
#   include "../device/iccard/pboc/v3_0/PBOC_v3_0_CMD.h"
#   include "../device/iccard/pboc/v3_0/PBOC_v3_0_AppAdapter.h"

// PBOC_Library部分 
using namespace zhou_yb::device::iccard::pboc;
using namespace zhou_yb::device::iccard::pboc::base;
using namespace zhou_yb::device::iccard::pboc::v1_0;
using namespace zhou_yb::device::iccard::pboc::v2_0;
using namespace zhou_yb::device::iccard::pboc::v3_0;

#ifndef NO_INCLUDE_HEALTHCARD_SOURCE
// 居民健康卡部分 
#include "../device/iccard/health_card/HealthCardAppAdapter.h"
using namespace zhou_yb::device::iccard::health_card;
#endif // end NO_INCLUDE_HEALTHCARD_SOURCE

#endif // end NO_INCLUDE_PBOC_SOURCE

#endif // end NO_INCLUDE_ICCARD_SOURCE
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_PINPAD_SOURCE
/* 密码键盘部分 */
#   include "../device/cmd_adapter/PinDevCmdAdapter.h"
#   include "../device/pinpad/device/DesPinDevAdapter.h"
#   include "../device/pinpad/device/HashPinDevAdapter.h"
#   include "../device/pinpad/device/PinDevAdapter.h"
#   include "../device/pinpad/device/RsaPinDevAdapter.h"
#   include "../device/pinpad/device/Sm2_PinDevAdapter.h"
#   include "../device/pinpad/device/Sm4_PinDevAdapter.h"
#endif
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_DEVICE_H_
//=========================================================
