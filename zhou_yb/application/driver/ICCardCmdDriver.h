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
#include "CommonCmdDriver.h"

#include "../../device/iccard/pboc/v2_0/PBOC_v2_0_AppAdapter.h"
using zhou_yb::device::iccard::pboc::v2_0::PBOC_v2_0_AppAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// IC卡命令驱动
class ICCardCmdDriver : 
    public CommandCollection, 
    public DeviceBehavior,
    public InterruptBehavior,
    public RefObject
{
protected:
    Ref<IICCardDevice> _pDev;

    list<Ref<IICCardDevice> > _icList;
    list<string> _argList;
public:
    ICCardCmdDriver()
    {
        _Registe("IsCardPresent", (*this), &ICCardCmdDriver::IsCardPresent);
        _Registe("SelectSLOT", (*this), &ICCardCmdDriver::SelectSLOT);
        _Registe("WaitForCard", (*this), &ICCardCmdDriver::WaitForCard);
        _Registe("PowerOn", (*this), &ICCardCmdDriver::PowerOn);
        _Registe("Apdu", (*this), &ICCardCmdDriver::Apdu);
        _Registe("ApduArray", (*this), &ICCardCmdDriver::ApduArray);
        _Registe("PowerOff", (*this), &ICCardCmdDriver::PowerOff);
    }
    //----------------------------------------------------- 
    /// 选择设备,返回当前已经选择的设备
    size_t SelectDevice(const Ref<IICCardDevice>& ic, const char* powerOnArg = NULL)
    {
        size_t count = _icList.size();
        if(ic.IsNull())
            return count;
        list<Ref<IICCardDevice> >::iterator itr;
        for(itr = _icList.begin();itr != _icList.end(); ++itr)
        {
            if((*itr) == ic)
                return count;
        }
        _icList.push_back(ic);
        _argList.push_back(_strput(powerOnArg));

        return (count + 1);
    }
    /// 释放设备,返回当前剩余的设备
    size_t ReleaseDevice(const Ref<IICCardDevice>& ic)
    {
        size_t count = _icList.size();
        size_t index = list_helper<Ref<IICCardDevice> >::position(_icList, ic);
        if(index == SIZE_EOF)
            return count;
        list_helper<Ref<IICCardDevice> >::erase_at(_icList, index);
        list_helper<string>::erase_at(_argList, index);

        return (count - 1);
    }
    void ReleaseDevice()
    {
        _icList.clear();
        _argList.clear();
        _pDev.Free();
    }
    //----------------------------------------------------- 
    /// 等待放入IC卡
    bool WaitForCard(uint timeoutMs)
    {
        Timer timer;
        size_t index = 0;
        list<Ref<IICCardDevice> >::iterator itr;
        list<string>::iterator argItr;
        while(timer.Elapsed() < timeoutMs)
        {
            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                _logErr(DeviceError::OperatorInterruptErr);
                return false;
            }
            argItr = _argList.begin();
            for(itr = _icList.begin();itr != _icList.end(); ++itr)
            {
                if((*itr)->PowerOn(argItr->c_str(), NULL))
                {
                    _pDev = (*itr);
                    return true;
                }
                ++argItr;
            }
        }
        return false;
    }
    /// 当前激活的IC卡
    inline Ref<IICCardDevice> ActiveIC()
    {
        return _pDev;
    }
    //----------------------------------------------------- 
    /**
     * @brief 判断读卡器上是否有卡
     * @date 2016-05-04 21:18
     * 
     * @param [in] arglist 
     * - 参数
     *  - SLOT 需要判断的卡片类型
     * .
     * 
     * @return True|False 有无卡
     */
    LC_CMD_METHOD(IsCardPresent)
    {
        size_t slot = arg["SLOT"].To<size_t>(0);
        list<Ref<IICCardDevice> >::iterator itr = list_helper<Ref<IICCardDevice> >::index_of(_icList, slot);
        if(itr == _icList.end())
        {
            _logErr(DeviceError::ArgRangeErr);
            return false;
        }
        list<string>::iterator argItr = list_helper<string>::index_of(_argList, slot);
        return (*itr)->PowerOn(argItr->c_str(), NULL);
    }
    /**
     * @brief 选择卡槽号
     * @date 2016-05-04 21:21
     * 
     * @param [in] arglist
     * - 参数
     *  - Contact 接触式
     *  - Contactless 非接
     *  - PSAM1 PSAM卡1
     *  - PSAM2 PSAM卡2
     * .
     */
    LC_CMD_METHOD(SelectSLOT)
    {
        size_t slot = arg["SLOT"].To<size_t>(0);
        list<Ref<IICCardDevice> >::iterator itr = list_helper<Ref<IICCardDevice> >::index_of(_icList, slot);
        if(itr == _icList.end())
        {
            _logErr(DeviceError::ArgRangeErr);
            return false;
        }

        _pDev = (*itr);
        return true;
    }
    /**
     * @brief 等待放卡
     * @date 2016-05-11 21:16
     * 
     * @param [in] arglist
     * - 参数
     *  - Timeout 等待放卡的超时时间
     * .
     * 
     * @retval SLOT 实际放入的IC卡索引
     */
    LC_CMD_METHOD(WaitForCard)
    {
        ASSERT_Func(!_pDev.IsNull());

        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        return WaitForCard(timeoutMs);
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
        ASSERT_Func(!_pDev.IsNull());

        size_t index = list_helper<Ref<IICCardDevice> >::position(_icList, _pDev);
        list<string>::iterator argItr = list_helper<string>::index_of(_argList, index);
        if(argItr == _argList.end())
        {
            _logErr(DeviceError::ArgRangeErr);
            return false;
        }

        ByteBuilder atr(16);
        if(!_pDev->PowerOn(argItr->c_str(), &atr))
            return false;
        
        ByteBuilder tmp(8);
        ByteConvert::ToAscii(atr, tmp);
        rlt.PushValue("Atr", tmp.GetString());
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
        ASSERT_Func(!_pDev.IsNull());

        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);

        string sApdu = arg["sApdu"].To<string>();
        ByteConvert::ToAscii(sApdu.c_str(), sCmd);

        if(!_pDev->Apdu(sCmd, rCmd))
            return false;
        ByteBuilder tmp(8);
        ByteConvert::ToAscii(rCmd, tmp);
        rlt.PushValue("rApdu", tmp.GetString());
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
        ASSERT_Func(!_pDev.IsNull());

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
                rlt.PushValue("rApdu", sCmd.GetString());
            }
        }
        return bApdu;
    }
    /// 卡片下电
    LC_CMD_METHOD(PowerOff)
    {
        ASSERT_Func(!_pDev.IsNull());

        _pDev->PowerOff();
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDCMDDRIVER_H_
//========================================================= 