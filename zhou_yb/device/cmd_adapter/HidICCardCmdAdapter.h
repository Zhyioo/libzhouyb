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
class HidICCardCmdAdapter : 
    public IInteractiveTrans,
    public BaseDevAdapterBehavior<THidDevice>,
    public RefObject
{
public:
    //----------------------------------------------------- 
    HidICCardCmdAdapter() : BaseDevAdapterBehavior<THidDevice>() {}
    //----------------------------------------------------- 
    /// 读数据
    virtual bool Read(ByteBuilder& data)
    {
        if(!BaseDevAdapterBehavior<THidDevice>::IsValid())
            return false;

        ByteBuilder lenBytes(8);
        ByteBuilder tmp(32);
        // 长度中包含状态码 
        /* 02 xx xx xx xx(4x长度) xx xx xx xx(4x状态吗) xx(Nx数据) xx(2x异或值) 03 */
        bool bRet = BaseDevAdapterBehavior<THidDevice>::_pDev->Read(tmp);
        // 读取成功,处理后续的分包 
        if(bRet && tmp[0] == 0x02)
        {
            /* 解析出所有数据长度 */
            int packLen = 0;
            // 提取出长度的数据(+1跳过STX,-len跳过数据域,-3跳过校验值和ETX)
            ByteConvert::Fold(tmp.GetBuffer(1), 4, lenBytes);
            packLen = lenBytes[0];
            packLen <<= BIT_OFFSET;
            packLen += lenBytes[1];

            packLen *= 2;
            // 0x02 2位异或值 0x03 4位长度
            packLen += 8;

            /* HID数据包有效数据长度 */
            int validDataLen = static_cast<int>(BaseDevAdapterBehavior<THidDevice>::_pDev->GetRecvLength());

            data.Append(BaseDevAdapterBehavior<THidDevice>::_tmpBuffer.GetBuffer(), _min(packLen, validDataLen));
            // 没有后续的包,全部数据已经接收 
            if(packLen <= validDataLen)
            {
                return true;
            }
            // 继续接收剩下的数据 
            packLen -= validDataLen;
            while(packLen > 0)
            {
                tmp.Clear();
                if(!BaseDevAdapterBehavior<THidDevice>::_pDev->Read(tmp))
                    return false;

                // 最后一个包 
                if(packLen <= validDataLen)
                {
                    data.Append(tmp.SubArray(packLen));
                    break;
                }
                else
                {
                    data += tmp
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
