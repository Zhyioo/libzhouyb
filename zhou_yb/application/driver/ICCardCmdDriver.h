//========================================================= 
/**@file ICCardCmdDriver.h
 * @brief IC������
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
/// IC����������
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
     * @brief ����Ƭ�ϵ�
     * 
     * @param [in] arglist �����б�
     * - ����:
     *  - arg[0] ��Ƭ�ϵ�ʱ�Ĳ���
     * .
     *
     * @return sAtr
     */
    LC_CMD_METHOD(PowerOn)
    {
        ByteBuilder atr(16);
        if(!_pDev->PowerOn(send.GetString(), &atr))
            return false;
        
        ByteConvert::ToAscii(atr, recv);
        return true;
    }
    /**
     * @brief ����ָ��
     * 
     * @param [in] arglist �����б�
     * - ����:
     *  - arg[0] = sApdu ��Ҫ���͵�ָ��
     * .
     * 
     * @return rApdu
     */
    LC_CMD_METHOD(Apdu)
    {
        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);
        ByteConvert::ToAscii(send, sCmd);

        if(!_pDev->Apdu(sCmd, rCmd))
            return false;
        ByteConvert::ToAscii(rCmd, recv);
        return true;
    }
    /**
     * @brief ��������APDU
     * 
     * @param [in] arglist APDUָ���б�
     * 
     * @return rApduArray
     */
    LC_CMD_METHOD(ApduArray)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        list<string>::iterator itr;
        ByteBuilder sCmd(32);
        ByteBuilder rCmd(32);

        bool bApdu = true;
        for(itr = arglist.begin();itr != arglist.end(); ++itr)
        {
            if(bApdu)
            {
                sCmd.Clear();
                ByteConvert::ToAscii(itr->c_str(), sCmd);

                rCmd.Clear();
                // �����һ��ָ�����ʧ��,���ټ������ͺ�����ָ��
                bApdu = _pDev->Apdu(sCmd, rCmd);
                ByteConvert::ToAscii(rCmd, recv);
            }
            recv += SPLIT_CHAR;
        }
        recv.RemoveTail();
        return bApdu;
    }
    /// ��Ƭ�µ�
    LC_CMD_METHOD(PowerOff)
    {
        _pDev->PowerOff();
        return true;
    }
};
//--------------------------------------------------------- 
/// ����IC����������
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDCMDDRIVER_H_
//========================================================= 