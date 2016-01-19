//========================================================= 
/**@file HidICCardCmdAdapter.h 
 * @brief 南天HID IC卡设备指令适配(自动处理大数据分包的接收并
 *  将分开的包统一接收并组合在一起)
 * 
 * @date 2012-06-30   09:30:44 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "HidCmdAdapter.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//--------------------------------------------------------- 
/// 0203指令集数据适配器(根据指令中标识的长度控制读取的包数目) 
template<class THidDevice>
class HidICCardCmdAdapter : public HidCmdAdapter<THidDevice>
{
public:
    //----------------------------------------------------- 
    HidICCardCmdAdapter() 
        : HidCmdAdapter<THidDevice>() {}
    HidICCardCmdAdapter(const Ref<THidDevice>& dev)
        : HidCmdAdapter<THidDevice>(dev) {}
    //----------------------------------------------------- 
    /// 读数据
    virtual bool Read(ByteBuilder& data)
    {
        if(!HidCmdAdapter<THidDevice>::IsValid())
            return false;

        ByteBuilder lenBytes(8);
        HidCmdAdapter<THidDevice>::_tmpBuffer.Clear();
        // 长度中包含状态码 
        /* 02 xx xx xx xx(4x长度) xx xx xx xx(4x状态吗) xx(Nx数据) xx(2x异或值) 03 */
        bool bRet = HidCmdAdapter<THidDevice>::_pDev->Read(HidCmdAdapter<THidDevice>::_tmpBuffer);
        // 读取成功,处理后续的分包 
        if(bRet && HidCmdAdapter<THidDevice>::_tmpBuffer[0] == 0x02)
        {
            /* 解析出所有数据长度 */
            int packLen = 0;
            // 提取出长度的数据(+1跳过STX,-len跳过数据域,-3跳过校验值和ETX)
            ByteConvert::Fold(HidCmdAdapter<THidDevice>::_tmpBuffer.GetBuffer(1), 4, lenBytes);
            packLen = lenBytes[0];
            packLen <<= BIT_OFFSET;
            packLen += lenBytes[1];

            packLen *= 2;
            // 0x02 2位异或值 0x03 4位长度
            packLen += 8;

            /* HID数据包有效数据长度 */
            int validDataLen = static_cast<int>(HidCmdAdapter<THidDevice>::_pDev->GetRecvLength());

            data.Append(HidCmdAdapter<THidDevice>::_tmpBuffer.GetBuffer(), _min(packLen, validDataLen));
            // 没有后续的包,全部数据已经接收 
            if(packLen <= validDataLen)
            {
                return true;
            }
            // 继续接收剩下的数据 
            packLen -= validDataLen;
            while(packLen > 0)
            {
                HidCmdAdapter<THidDevice>::_tmpBuffer.Clear();
                if(HidCmdAdapter<THidDevice>::_pDev->Read(HidCmdAdapter<THidDevice>::_tmpBuffer) == false)
                    return false;

                // 最后一个包 
                if(packLen <= validDataLen)
                {
                    data.Append(HidCmdAdapter<THidDevice>::_tmpBuffer.GetBuffer(), packLen);
                    break;
                }
                else
                {
                    data += HidCmdAdapter<THidDevice>::_tmpBuffer;
                }

                packLen -= validDataLen;
            }
        }

        return bRet;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace cmd_adapter
} // namespace device
} // namespace zhou_yb
//=========================================================
