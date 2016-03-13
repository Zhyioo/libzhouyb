//========================================================= 
/**@file ICBC_HidIDCardTCmdAdapter.h 
 * @brief 工行HID二代证阅读器指令集适配器 
 *
 * 指令中不包含"AA AA AA 69 96"指令头,发送时剔除,接收时过滤即可 
 * 
 * @date 2014-11-06   22:27:28 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../device/cmd_adapter/HidCmdAdapter.h"
using zhou_yb::device::cmd_adapter::HidCmdAdapter;
//---------------------------------------------------------
namespace zhou_yb {
namespace application {
namespace idcard {
//--------------------------------------------------------- 
/// 工行二代证指令适配器
template<class THidDevice>
class ICBC_HidIDCardTCmdAdapter : public HidCmdAdapter<THidDevice, 2>
{
public:
    //----------------------------------------------------- 
    ICBC_HidIDCardTCmdAdapter()
        : HidCmdAdapter<THidDevice, 2>() {}
    ICBC_HidIDCardTCmdAdapter(const Ref<HidDevice>& dev)
        : HidCmdAdapter<THidDevice, 2>(dev) {}
    //----------------------------------------------------- 
    /// 写数据 
    virtual bool Write(const ByteArray& data)
    {
        if(!IsValid())
            return false;

        // 前导指令长度 
        const size_t CMD_LEN = 5;
        return HidCmdAdapter::Write(data.SubArray(CMD_LEN));
    }
    //----------------------------------------------------- 
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        if(!IsValid())
            return false;

        size_t lastLen = data.GetLength();
        data += "\xAA\xAA\xAA\x96\x69";
        // 预补2字节长度位 
        data.Append((byte)0x00, 2);
        
        bool bRet = HidCmdAdapter::Read(data);
        if(!bRet) data.RemoveTail(data.GetLength() - lastLen);
        
        ushort rlen = (ushort)(data.GetLength() - lastLen);
        // 指令头和长度位的长度为定值7 
        rlen -= 7;
        // 取高字节 
        data[lastLen + 5] = _itobyte(rlen >> BIT_OFFSET);
        // 取低字节 
        data[lastLen + 6] = _itobyte(rlen);
        
        return bRet;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 默认类型 
typedef ICBC_HidIDCardTCmdAdapter<HidDevice> ICBC_HidIDCardCmdAdapter;
//---------------------------------------------------------
} // namespace idcard
} // namespace application 
} // namespace zhou_yb 
//=========================================================
