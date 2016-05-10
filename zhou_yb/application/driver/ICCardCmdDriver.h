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
class ICCardCmdDriver : 
    public DevAdapterBehavior<IICCardDevice>, 
    public CommandCollection, 
    public RefObject
{
public:
    ICCardCmdDriver()
    {
        _Registe("PowerOn", (*this), &ICCardCmdDriver::PowerOn);
        _Registe("Apdu", (*this), &ICCardCmdDriver::Apdu);
        _Registe("ApduArray", (*this), &ICCardCmdDriver::ApduArray);
        _Registe("PowerOff", (*this), &ICCardCmdDriver::PowerOff);
    }

    /**
     * @brief 给卡片上电
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Arg 卡片上电时的参数
     * .
     *
     * @retval Atr
     */
    LC_CMD_METHOD(PowerOn)
    {
        string poweronArg = arg["Arg"].To<string>();

        ByteBuilder atr(16);
        if(!_pDev->PowerOn(poweronArg.c_str(), &atr))
            return false;
        
        ByteBuilder tmp(8);
        ByteConvert::ToAscii(atr, tmp);
        rlt.PutValue("Atr", tmp.GetString());
        return true;
    }
    /**
     * @brief 交互指令
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - sApdu 需要发送的指令
     * .
     * 
     * @retval rApdu
     */
    LC_CMD_METHOD(Apdu)
    {
        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);

        string sApdu = arg["sApdu"].To<string>();
        ByteConvert::ToAscii(sApdu.c_str(), sCmd);

        if(!_pDev->Apdu(sCmd, rCmd))
            return false;
        ByteBuilder tmp(8);
        ByteConvert::ToAscii(rCmd, tmp);
        rlt.PutValue("rApdu", tmp.GetString());
        return true;
    }
    /**
     * @brief 交互多条APDU
     * 
     * @param [in] arglist APDU指令列表
     * - 参数
     *  - sApdu 需要发送的指令串
     * .
     * 
     * @retval rApdu
     */
    LC_CMD_METHOD(ApduArray)
    {
        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);

        bool bApdu = true;
        list<string> apduList;
        size_t count = arg.GetValue("sApdu", apduList);
        for(list<string>::iterator itr = apduList.begin();itr != apduList.end(); ++itr)
        {
            if(bApdu)
            {
                sCmd.Clear();
                ByteConvert::ToAscii(itr->c_str(), sCmd);

                rCmd.Clear();
                // 如果第一条指令交互都失败,则不再继续发送后续的指令
                bApdu = _pDev->Apdu(sCmd, rCmd);
                sCmd.Clear();
                ByteConvert::ToAscii(rCmd, sCmd);
                rlt.PutValue("rApdu", sCmd.GetString());
            }
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
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDCMDDRIVER_H_
//========================================================= 