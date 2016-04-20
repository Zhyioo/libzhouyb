//========================================================= 
/**@file ICCardCmdDriver.h
 * @brief IC卡命令
 * 
 * @date 2016-04-14   10:15:45
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICCARDCMDDRIVER_H_
#define _LIBZHOUYB_ICCARDCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"

#include "../../device/iccard/pboc/v2_0/PBOC_v2_0_AppAdapter.h"
using zhou_yb::device::iccard::pboc::v2_0::PBOC_v2_0_AppAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// IC卡命令驱动
class ICCardCmdDriver : public DevAdapterBehavior<IICCardDevice>, public CommandCollection
{
protected:
    PBOC_v2_0_AppAdapter _icAdapter;
public:
    ICCardCmdDriver()
    {
        _Bind("PowerOn", (*this), &ICCardCmdDriver::PowerOn);
        _Bind("Apdu", (*this), &ICCardCmdDriver::Apdu);
        _Bind("ApduArray", (*this), &ICCardCmdDriver::ApduArray);
        _Bind("PowerOff", (*this), &ICCardCmdDriver::PowerOff);
    }

    virtual void SelectDevice(const Ref<IICCardDevice>& dev)
    {
        _icAdapter.SelectDevice(dev);
    }
    virtual void ReleaseDevice()
    {
        _icAdapter.ReleaseDevice();
    }

    /**
     * @brief 给卡片上电
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Arg 卡片上电时的参数
     * .
     *
     * @return sAtr
     */
    LC_CMD_METHOD(PowerOn)
    {
        ArgParser arg;
        arg.Parse(send);

        string sArg = arg["Arg"].To<string>();

        ByteBuilder atr(16);
        if(!_pDev->PowerOn(sArg.c_str(), &atr))
            return false;
        
        ByteConvert::ToAscii(atr, recv);
        return true;
    }
    /**
     * @brief 交互指令
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Apdu 需要发送的指令
     * .
     * 
     * @return rApdu
     */
    LC_CMD_METHOD(Apdu)
    {
        ArgParser arg;
        arg.Parse(send);

        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);

        string sArg = arg["Apdu"].To<string>();
        ByteConvert::ToAscii(sArg.c_str(), sCmd);

        if(!_pDev->Apdu(sCmd, rCmd))
            return false;
        ByteConvert::ToAscii(rCmd, recv);
        return true;
    }
    /**
     * @brief 交互多条APDU
     * 
     * @param [in] arglist APDU指令列表
     * 
     * @return rApduArray
     */
    LC_CMD_METHOD(ApduArray)
    {
        ArgParser arg;
        arg.Parse(send);

        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);

        bool bApdu = true;
        list<string> apduList;
        size_t count = arg.GetValue("Apdu", apduList);
        for(list<string>::iterator itr = apduList.begin();itr != apduList.end(); ++itr)
        {
            if(bApdu)
            {
                sCmd.Clear();
                ByteConvert::ToAscii(itr->c_str(), sCmd);

                rCmd.Clear();
                // 如果第一条指令交互都失败,则不再继续发送后续的指令
                bApdu = _pDev->Apdu(sCmd, rCmd);
                ByteConvert::ToAscii(rCmd, recv);
            }
            recv += SPLIT_CHAR;
        }
        if(count > 0)
        {
            recv.RemoveTail();
        }
        return bApdu;
    }
    /// 卡片下电
    LC_CMD_METHOD(PowerOff)
    {
        _pDev->PowerOff();
        return true;
    }
};
//--------------------------------------------------------- 
/// 串口IC卡命令驱动
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDCMDDRIVER_H_
//========================================================= 