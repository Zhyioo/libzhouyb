//========================================================= 
/**@file PSBC_PinCmdDriver.h
 * @brief �ʴ����ܼ�������
 * 
 * @date 2016-04-02   18:07:39
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PSBC_PINCMDDRIVER_H_
#define _LIBZHOUYB_PSBC_PINCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"
using zhou_yb::application::driver::CommandDriver;

#include "../../extension/security/Key_Provider.h"
using zhou_yb::extension::security::Key_Provider;

#include "../pinpad/PSBC_PinPadDevAdapter.h"
using zhou_yb::application::pinpad::PSBC_PinDesDevAdapter;
using zhou_yb::application::pinpad::PSBC_PinSm4DevAdapter;
using zhou_yb::application::pinpad::PSBC_PinPadDevAdapter;
using zhou_yb::application::pinpad::PSBC_PinManagerDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// �ʴ����ܼ�����������
class PSBC_PinCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>,
    public CommandCollection,
    public RefObject
{
protected:
    ComICCardCmdAdapter _cmdAdapter;
    PinDevCmdAdapter _pinCmdAdapter;
    PSBC_PinSm4DevAdapter _sm4Adapter;
    PSBC_PinDesDevAdapter _desAdapter;
    PSBC_PinPadDevAdapter _guomiAdapter;
    PSBC_PinManagerDevAdapter _managerAdapter;
public:
    PSBC_PinCmdDriver()
    {
        _Bind("ResetKey", (*this), &PSBC_PinCmdDriver::ResetKey);
        _Bind("SetPinLength", (*this), &PSBC_PinCmdDriver::SetPinLength);
        _Bind("DownloadMK", (*this), &PSBC_PinCmdDriver::DownloadMK);
        _Bind("DownloadWK", (*this), &PSBC_PinCmdDriver::DownloadWK);
        _Bind("WaitPassword_Ansi98", (*this), &PSBC_PinCmdDriver::WaitPassword_Ansi98);
        _Bind("GetPassword_Ansi98", (*this), &PSBC_PinCmdDriver::GetPassword_Ansi98);
    }
    /// �����豸
    virtual void SelectDevice(const Ref<IInteractiveTrans>& dev)
    {
        DevAdapterBehavior::SelectDevice(dev);
        _pinCmdAdapter.SelectDevice(_pDev);
        _cmdAdapter.SelectDevice(_pDev);
        _sm4Adapter.SelectDevice(_pinCmdAdapter);
        _desAdapter.SelectDevice(_pinCmdAdapter);
        _guomiAdapter.SelectDevice(_cmdAdapter);
        _managerAdapter.SelectDevice(_pinCmdAdapter);
    }
    /// �ͷ��豸
    virtual void ReleaseDevice()
    {
        DevAdapterBehavior::ReleaseDevice();
        _pinCmdAdapter.ReleaseDevice();
        _cmdAdapter.ReleaseDevice();
    }

    /**
     * @brief ��ʼ����ԿΪָ��ֵ
     * 
     * @param [in] arglist �����б�
     * - ����:
     *  - arg[0] = DES/SM4 �㷨��ʶ
     *  - arg[1] = mkIndex ����Կ����
     *  - atg[2] = key ���õ�����Կ
     * .
     */
    LC_CMD_METHOD(ResetMK)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        size_t index = 0;
        string algorithm = CommandDriver::Arg<string>(arglist, index++);
        byte mkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        string key = CommandDriver::Arg<string>(arglist, index++);

        ByteBuilder keyBuff(16);
        DevCommand::FromAscii(key.c_str(), keyBuff);
        // ����DES������Կ
        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            if(!_desAdapter.ResetMK(mkIndex, keyBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            if(!_sm4Adapter.ResetMK(mkIndex, keyBuff))
                return false;
        }
        return true;
    }
    /**
     * @brief �ָ���ԿΪ��������
     * 
     * @param [in] arglist �����б�
     * - ����:
     *  - arg[0] = DES/SM4 �㷨��ʶ
     *  - arg[1] = mkIndex ����Կ����
     * .
     */
    LC_CMD_METHOD(ResetKey)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        size_t index = 0;
        string algorithm = CommandDriver::Arg<string>(arglist, index++);
        byte mkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        // ����DES������Կ
        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            if(!_desAdapter.ResetKey(mkIndex))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            if(!_sm4Adapter.ResetKey(mkIndex))
                return false;
        }
        return true;
    }
    /**
     * @brief �����޸�����Կ
     * 
     * @param [in] arglist �����б�
     * - ����:
     *  - arg[0] = DES/SM4 �㷨��ʶ
     *  - arg[1] = mkIndex ����Կ����
     *  - arg[2] = oldKey ԭ��������Կ����
     *  - atg[3] = newKey ���õ�����Կ����
     * .
     */
    LC_CMD_METHOD(UpdateMainKey)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        size_t index = 0;
        string algorithm = CommandDriver::Arg<string>(arglist, index++);
        byte mkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        string oldKey = CommandDriver::Arg<string>(arglist, index++);
        string newKey = CommandDriver::Arg<string>(arglist, index++);

        ByteBuilder oldKeyBuff(16);
        DevCommand::FromAscii(oldKey.c_str(), oldKeyBuff);

        ByteBuilder newKeyBuff(16);
        DevCommand::FromAscii(newKey.c_str(), newKeyBuff);

        // ����DES������Կ
        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            if(!_desAdapter.UpdateMainKey(mkIndex, oldKeyBuff, newKeyBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            if(!_sm4Adapter.UpdateMainKey(mkIndex, oldKeyBuff, newKeyBuff))
                return false;
        }
        return true;
    }
    /**
     * @brief ������������볤��
     *
     * @param [in] arglist �����б�
     * - ����:
     *  - arg[0] = length
     * .
     */
    LC_CMD_METHOD(SetPinLength)
    {
        size_t len = ArgConvert::FromString<size_t>(send.GetString());
        // �������볤��
        ByteBuilder tmp(2);
        _managerAdapter.SetPinLength(len);
        if(_pDev->Read(tmp) || tmp.GetLength() < 1 || tmp[0] != 0xAA)
            return false;
        return true;
    }
    /**
     * @brief ������������Կ
     * 
     * @param [in] arglist
     * - ����:
     *  - arg[0] = DES/SM4 �㷨��ʶ
     *  - arg[1] = mkIndex ����Կ���� 
     *  - arg[2] = key ����Կ����
     *  - arg[3] = kcv ����ԿKCV
     * .
     */
    LC_CMD_METHOD(DownloadMK)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        size_t index = 0;
        string algorithm = CommandDriver::Arg<string>(arglist, index++);
        byte mkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        string key = CommandDriver::Arg<string>(arglist, index++);
        string kcv = CommandDriver::Arg<string>(arglist, index++);

        ByteBuilder keyBuff(16);
        ByteBuilder kcvBuff(16);

        DevCommand::FromAscii(key.c_str(), keyBuff);
        DevCommand::FromAscii(kcv.c_str(), kcvBuff);

        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            if(!_desAdapter.UpdateEncryptedMainKey(mkIndex, keyBuff, kcvBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            if(!_sm4Adapter.UpdateEncryptedMainKey(mkIndex, keyBuff, kcvBuff))
                return false;
        }
        return true;
    }
    /**
     * @brief �������ع�����Կ
     * 
     * @param [in] arglist
     * - ����:
     *  - arg[0] DES/SM4 �㷨��ʶ
     *  - arg[1] = mkIndex ����Կ����
     *  - arg[2] = wkIndex ������Կ����
     *  - arg[3] = key ������Կ����
     *  - arg[4] = kcv ������ԿKCV
     * .
     */
    LC_CMD_METHOD(DownloadWK)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        size_t index = 0;
        string algorithm = CommandDriver::Arg<string>(arglist, index++);
        byte mkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        byte wkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        string key = CommandDriver::Arg<string>(arglist, index++);
        string kcv = CommandDriver::Arg<string>(arglist, index++);

        ByteBuilder keyBuff(16);
        ByteBuilder kcvBuff(16);

        DevCommand::FromAscii(key.c_str(), keyBuff);
        DevCommand::FromAscii(kcv.c_str(), kcvBuff);

        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            if(!_desAdapter.UpdateEncryptedWorkKey(mkIndex, wkIndex, keyBuff, kcvBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            if(!_sm4Adapter.UpdateEncryptedWorkKey(mkIndex, wkIndex, kcvBuff, kcvBuff))
                return false;
        }
        return true;
    }
    /**
     * @brief ��������
     * 
     * @param [in] arglist
     * - ����:
     *  - arg[0] = DES/SM4 �㷨��ʶ
     *  - arg[1] = isVoiceAgain �Ƿ��ٴ�����
     *  - arg[2] = mkIndex ����Կ����
     *  - arg[3] = wkIndex ������Կ����
     *  - arg[4] = cardNumber �ʺ���Ϣ
     * .
     */
    LC_CMD_METHOD(WaitPassword_Ansi98)
    {
        list<string> arglist;
        StringHelper::Split(send.GetString(), arglist);

        size_t index = 0;
        string algorithm = CommandDriver::Arg<string>(arglist, index++);
        bool isAgain = CommandDriver::Arg<bool>(arglist, index++);
        byte mkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        byte wkIndex = _itobyte(CommandDriver::Arg<uint>(arglist, index++));
        string tr2 = CommandDriver::Arg<string>(arglist, index++);

        ByteArray sTr2(tr2.c_str(), tr2.length());
        ByteArray trBuff = sTr2;
        for(size_t i = 0;i < sTr2.GetLength(); ++i)
        {
            if(sTr2[i] == '=' || sTr2[i] == '\'')
            {
                trBuff = sTr2.SubArray(0, i);
                break;
            }
        }
        ByteBuilder cardNumber(8);
        ByteBuilder tmp(8);
        Key_Provider::FormatAnsiCardID(trBuff, tmp);
        tmp.RemoveFront(2);
        ByteConvert::ToAscii(tmp, cardNumber);

        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            if(!_desAdapter.WaitPassword_Ansi98(mkIndex, wkIndex, isAgain, cardNumber))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            if(!_sm4Adapter.WaitPassword_Ansi98(mkIndex, wkIndex, isAgain, cardNumber))
                return false;
        }

        return true;
    }
    /**
     * @brief ��ȡ�������������
     */
    LC_CMD_METHOD(GetPassword_Ansi98)
    {
        ByteBuilder pin(16);
        size_t pinlen = 0;
        if(!_managerAdapter.GetPassword(pin, &pinlen))
            return false;

        ByteConvert::ToAscii(pin, recv);
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PSBC_PINCMDDRIVER_H_
//========================================================= 