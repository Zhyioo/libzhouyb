//========================================================= 
/**@file HidMagCmdAdapter.h
 * @brief 兼容不带长度和带长度的磁条HID包数据
 * 
 * @date 2016-04-07   16:04:03
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDMAGCMDADAPTER_H_
#define _LIBZHOUYB_HIDMAGCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../device/cmd_adapter/HidCmdAdapter.h"
using zhou_yb::device::cmd_adapter::HidCmdAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace magnetic {
//--------------------------------------------------------- 
/**
 * @brief 磁条HID包指令适配器
 *
 * 1.支持不带长度字节的磁条指令
 * 2.支持指定的最大N个字节的磁条指令
 */
template<class THidDevice>
class HidMagCmdAdapter : public HidFixedCmdAdapter<THidDevice>
{
public:
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        if(!BaseDevAdapterBehavior<THidDevice>::IsValid())
            return _logRetValue(false);
        // 先读第一个包
        ByteBuilder recv(64);
        if(!_pDev->Read(recv))
            return _logRetValue(false);
        
        // 检查接收到的数据是否以磁条的指令开头

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace magnetic
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDMAGCMDADAPTER_H_
//========================================================= 