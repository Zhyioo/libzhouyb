//========================================================= 
/**@file PSBC_PinCmdDriver.h
 * @brief 邮储国密键盘驱动
 * 
 * @date 2016-04-02   18:07:39
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PSBC_PINCMDDRIVER_H_
#define _LIBZHOUYB_PSBC_PINCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"

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
/// 邮储国密键盘命令驱动
class PSBC_PinCmdDriver : 
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public CommandCollection,
    public ILastErrBehavior,
    public LoggerBehavior,
    public RefObject
{
protected:
    LastErrExtractor _appErr;
    LastErrExtractor _lastErr;
    DevAdapterInvoker<IInteractiveTrans> _adapter;
    LoggerInvoker _logInvoker;
    ComICCardCmdAdapter _cmdAdapter;
    PinDevCmdAdapter _pinCmdAdapter;
    PSBC_PinSm4DevAdapter _sm4Adapter;
    PSBC_PinDesDevAdapter _desAdapter;
    PSBC_PinPadDevAdapter _guomiAdapter;
    PSBC_PinManagerDevAdapter _managerAdapter;
public:
    PSBC_PinCmdDriver()
    {
        // 同一层次不需要叠加
        _appErr.IsLayerMSG = false;
        _appErr.IsFormatMSG = false;
        _appErr.Select(_sm4Adapter, "SM4");
        _appErr.Select(_desAdapter, "DES");
        _appErr.Select(_guomiAdapter, "GuoMi");
        _appErr.Select(_managerAdapter, "Manager");

        _lastErr.IsLayerMSG = true;
        _lastErr.IsFormatMSG = false;
        _lastErr.Select(_cmdAdapter, "PIN_CMD");
        _lastErr.Select(_appErr);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _cmdAdapter, _sm4Adapter, _desAdapter, _guomiAdapter, _managerAdapter;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType>::select(_adapter),
            _cmdAdapter, _pinCmdAdapter;

        _sm4Adapter.SelectDevice(_pinCmdAdapter);
        _desAdapter.SelectDevice(_pinCmdAdapter);
        _guomiAdapter.SelectDevice(_cmdAdapter);
        _managerAdapter.SelectDevice(_pinCmdAdapter);

        _Registe("ResetMK", (*this), &PSBC_PinCmdDriver::ResetMK);
        _Registe("ResetKey", (*this), &PSBC_PinCmdDriver::ResetKey);
        _Registe("UpdateMainKey", (*this), &PSBC_PinCmdDriver::UpdateMainKey);
        _Registe("SetPinLength", (*this), &PSBC_PinCmdDriver::SetPinLength);
        _Registe("DownloadMK", (*this), &PSBC_PinCmdDriver::DownloadMK);
        _Registe("DownloadWK", (*this), &PSBC_PinCmdDriver::DownloadWK);
        _Registe("WaitPassword_Ansi98", (*this), &PSBC_PinCmdDriver::WaitPassword_Ansi98);
        _Registe("GetPassword_Ansi98", (*this), &PSBC_PinCmdDriver::GetPassword_Ansi98);
        _Registe("Evaluation", (*this), &PSBC_PinCmdDriver::Evaluation);
        _Registe("GenerateKEY", (*this), &PSBC_PinCmdDriver::GenerateKEY);
        _Registe("GeneratePIN", (*this), &PSBC_PinCmdDriver::GeneratePIN);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    /**
     * @brief 初始化密钥为指定值
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Algorithm [DES][SM4] 算法标识
     *  - MkIndex 主密钥索引
     *  - Key 重置的新密钥
     * .
     */
    LC_CMD_METHOD(ResetMK)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["MkIndex"].To<byte>();
        string key = arg["Key"].To<string>();

        ByteBuilder keyBuff(16);
        DevCommand::FromAscii(key.c_str(), keyBuff);
        // 重置DES所有密钥
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
     * @brief 恢复密钥为出厂设置
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Algorithm [DES][SM4] 算法标识
     *  - MkIndex 主密钥索引
     * .
     */
    LC_CMD_METHOD(ResetKey)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["MkIndex"].To<byte>();

        // 重置DES所有密钥
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
     * @brief 明文修改主密钥
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Algorithm [DES][SM4] 算法标识
     *  - MkIndex 主密钥索引
     *  - OldKey 原来的主密钥明文
     *  - NewKey 重置的新密钥明文
     * .
     */
    LC_CMD_METHOD(UpdateMainKey)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["MkIndex"].To<byte>();
        string oldKey = arg["OldKey"].To<string>();
        string newKey = arg["NewKey"].To<string>();

        ByteBuilder oldKeyBuff(16);
        DevCommand::FromAscii(oldKey.c_str(), oldKeyBuff);

        ByteBuilder newKeyBuff(16);
        DevCommand::FromAscii(newKey.c_str(), newKeyBuff);

        // 重置DES所有密钥
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
     * @brief 设置输入的密码长度
     *
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Length 需要设置的输入长度
     * .
     */
    LC_CMD_METHOD(SetPinLength)
    {
        size_t len = arg["Length"].To<size_t>();
        // 设置密码长度
        ByteBuilder tmp(2);
        _managerAdapter.SetPinLength(len);
        if(_pDev->Read(tmp) || tmp.GetLength() < 1 || tmp[0] != 0xAA)
            return false;
        return true;
    }
    /**
     * @brief 密文下载主密钥
     * 
     * @param [in] arglist
     * - 参数:
     *  - Algorithm [DES][SM4][AUTO] 算法标识
     *  - MkIndex 主密钥索引,算法为[AUTO]时该参数为13字符以内的密钥ID
     *  - KEY 主密钥密文
     *  - KCV 主密钥KCV
     * .
     */
    LC_CMD_METHOD(DownloadMK)
    {
        string algorithm = arg["Algorithm"].To<string>();
        string keyId = arg["MkIndex"].To<string>();
        string key = arg["KEY"].To<string>();
        string kcv = arg["KCV"].To<string>();

        ByteBuilder keyBuff(16);
        ByteBuilder kcvBuff(16);

        DevCommand::FromAscii(key.c_str(), keyBuff);
        DevCommand::FromAscii(kcv.c_str(), kcvBuff);

        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            byte mkIndex = ArgConvert::FromString<byte>(keyId);
            if(!_desAdapter.UpdateEncryptedMainKey(mkIndex, keyBuff, kcvBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            byte mkIndex = ArgConvert::FromString<byte>(keyId);
            if(!_sm4Adapter.UpdateEncryptedMainKey(mkIndex, keyBuff, kcvBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "AUTO", true))
        {
            const size_t MAC_KEYID_LENGTH = 13;
            ByteBuilder mkId(MAC_KEYID_LENGTH);
            mkId = keyId.c_str();
            ByteConvert::Fill(mkId, MAC_KEYID_LENGTH, false, '0');
            // 设置主密钥ID
            if(!_guomiAdapter.SetMK_ID(mkId))
                return false;
            if(!_guomiAdapter.DownloadMK(keyBuff, mkId, kcvBuff))
                return false;
        }
        return true;
    }
    /**
     * @brief 密文下载工作密钥
     * 
     * @param [in] arglist
     * - 参数:
     *  - Algorithm [DES][SM4][AUTO] 算法标识
     *  - MkIndex 主密钥索引
     *  - WkIndex 工作密钥索引
     *  - KEY 工作密钥密文
     *  - KCV 工作密钥KCV
     * .
     * @retval KCV [AUTO]时,KCV为空则设备生成KCV返回
     */
    LC_CMD_METHOD(DownloadWK)
    {
        string algorithm = arg["Algorithm"].To<string>();
        string sMkIndex = arg["MkIndex"].To<string>();
        string sWkIndex = arg["WkIndex"].To<string>();
        string key = arg["KEY"].To<string>();
        string kcv = arg["KCV"].To<string>();

        ByteBuilder keyBuff(16);
        ByteBuilder kcvBuff(16);

        DevCommand::FromAscii(key.c_str(), keyBuff);
        DevCommand::FromAscii(kcv.c_str(), kcvBuff);

        if(StringConvert::Compare(algorithm.c_str(), "DES", true))
        {
            byte mkIndex = ArgConvert::FromString<byte>(sMkIndex);
            byte wkIndex = ArgConvert::FromString<byte>(sWkIndex);
            if(!_desAdapter.UpdateEncryptedWorkKey(mkIndex, wkIndex, keyBuff, kcvBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "SM4", true))
        {
            byte mkIndex = ArgConvert::FromString<byte>(sMkIndex);
            byte wkIndex = ArgConvert::FromString<byte>(sWkIndex);
            if(!_sm4Adapter.UpdateEncryptedWorkKey(mkIndex, wkIndex, kcvBuff, kcvBuff))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "AUTO", true))
        {
            const size_t MAC_KEYID_LENGTH = 13;
            ByteBuilder keyId(MAC_KEYID_LENGTH);
            // 设置主密钥ID
            keyId = sMkIndex.c_str();
            ByteConvert::Fill(keyId, MAC_KEYID_LENGTH, false, '0');
            if(!_guomiAdapter.SetMK_ID(keyId))
                return false;
            // 设置工作密钥ID
            keyId = sWkIndex.c_str();
            ByteConvert::Fill(keyId, MAC_KEYID_LENGTH, false, '0');
            if(!_guomiAdapter.SetWK_ID(keyId))
                return false;
            // KCV为空则设备返回一个KCV
            if(kcvBuff.IsEmpty())
            {
                if(!_guomiAdapter.DownloadWK_KCV(keyBuff, keyId, &kcvBuff))
                    return false;
                ByteBuilder tmp(8);
                ByteConvert::ToAscii(kcvBuff, tmp);
                rlt.PushValue("KCV", tmp.GetString());
            }
            else
            {
                if(!_guomiAdapter.DownloadWK(keyBuff, keyId, kcvBuff))
                    return false;
            }
        }
        return true;
    }
    /**
     * @brief 输入密码
     * 
     * @param [in] arglist
     * - 参数:
     *  - Algorithm [DES][SM4] 算法标识
     *  - IsVoiceAgain 是否再次输入
     *  - MkIndex 主密钥索引
     *  - WkIndex 工作密钥索引
     *  - CardNumber 帐号信息
     * .
     */
    LC_CMD_METHOD(WaitPassword_Ansi98)
    {
        string algorithm = arg["Algorithm"].To<string>();
        bool isAgain = arg["IsVoiceAgain"].To<bool>();
        byte mkIndex = arg["MkIndex"].To<byte>();
        byte wkIndex = arg["WkIndex"].To<byte>();
        string tr2 = arg["CardNumber"].To<string>();

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
     * @brief 获取输入的密文密码
     * @retval Pwd
     */
    LC_CMD_METHOD(GetPassword_Ansi98)
    {
        ByteBuilder pin(16);
        size_t pinlen = 0;
        if(!_managerAdapter.GetPassword(pin, &pinlen))
            return false;

        ByteBuilder tmp(8);
        ByteConvert::ToAscii(pin, tmp);
        rlt.PushValue("Pwd", tmp.GetString());
        return true;
    }
    /**
     * @brief 等待用户输入(明文输入密码)
     * @date 2016-05-04 20:37
     * @param [in] arglist
     * - 参数
     *  - IsVoice 是否进行语音提示
     * .
     */
    LC_CMD_METHOD(InputInformation)
    {
        bool isVoice = arg["IsVoice"].To<bool>(true);
        return _managerAdapter.InputInformation(isVoice);
    }
    /**
     * @brief 评价
     * 
     * @param [in] arglist
     * - 参数
     *  - IsVoice 是否播放语音
     * .
     * @retval EvaluationId 评价ID
     * @retval Evaluation 评价描述字符 满意/不满意/非常满意
     */
    LC_CMD_METHOD(Evaluation)
    {
        string voice = arg["IsVoice"].To<string>(false);
        PSBC_PinManagerDevAdapter::EvaluationStatus status = PSBC_PinManagerDevAdapter::Unknown;
        // 没有该字段名称
        if(voice.length() < 1)
        {
            if(!_managerAdapter.Evaluation(status))
                return false;
        }
        else
        {
            bool isVoice = ArgConvert::FromString<bool>(voice);
            if(!_managerAdapter.EvaluationVoice(status, isVoice))
                return false;
        }
        
        string evaluationId = ArgConvert::ToString<int>(static_cast<int>(status));
        rlt.PushValue("EvaluationId", evaluationId);
        rlt.PushValue("Evaluation", PSBC_PinManagerDevAdapter::EvaluationTostring(status));
        return true;
    }
    /**
     * @brief 生成公钥
     * 
     * @param [in] arglist
     * - 参数
     *  - Algorithm [SM2][RSA] 算法标识
     *  - RsaSize RSA密钥的位数,默认为2048
     * .
     * @retval PublicKey
     */
    LC_CMD_METHOD(GenerateKEY)
    {
        string algorithm = arg["Algorithm"].To<string>();
        ByteBuilder pk(512);
        if(StringConvert::Compare(algorithm.c_str(), "SM2", true))
        {
            if(!_guomiAdapter.GenerateKEY_SM2(pk))
                return false;
        }
        else if(StringConvert::Compare(algorithm.c_str(), "RSA", true))
        {
            uint bit = arg["RsaSize"].To<uint>(2048);
            if(!_guomiAdapter.GenerateKEY_RSA(bit, pk))
                return false;
        }
        ByteBuilder tmp(512);
        ByteConvert::ToAscii(pk, tmp);
        rlt.PushValue("PublicKey", tmp.GetString());
        return true;
    }
    /**
     * @brief 生成密钥
     * 
     * @param [in] arglist
     * - 参数
     *  - Algorithm [DES][SM4] 算法标识
     *  - WkIndex 加密的工作密钥ID
     *  - CardNumber 帐号信息
     *  - PadByte 后补字节
     *  - PinLength 输入的密码长度
     *  - IsNeedOK 是否需要按回车自动返回
     *  - Timeout 超时时间(秒)
     * .
     * @retval PIN
     */
    LC_CMD_METHOD(GeneratePIN)
    {
        string algorithm = arg["Algorithm"].To<string>();
        string sWkIndex = arg["WkIndex"].To<string>();
        string sCardNumber = arg["CardNumber"].To<string>();
        byte padByte = arg["PadByte"].To<byte>();
        size_t pinLength = arg["PinLength"].To<size_t>();
        bool isNeedOk = arg["IsNeedOK"].To<bool>();
        uint timeoutS = arg["Timeout"].To<uint>();

        // 设置算法
        PSBC_PinPadDevAdapter::AlgorithmMode mode = PSBC_PinPadDevAdapter::UnknownMode;
        ByteArray sMode(algorithm.c_str(), algorithm.length());
        if(StringConvert::Compare(sMode, "DES", true))
        {
            mode = PSBC_PinPadDevAdapter::DES3_Mode;
        }
        else if(StringConvert::Compare(sMode, "3DES", true))
        {
            mode = PSBC_PinPadDevAdapter::DES3_Mode;
        }
        else if(StringConvert::Compare(sMode, "SM2", true))
        {
            mode = PSBC_PinPadDevAdapter::SM2_Mode;
        }
        else if(StringConvert::Compare(sMode, "SM4", true))
        {
            mode = PSBC_PinPadDevAdapter::SM4_Mode;
        }
        else if(StringConvert::Compare(sMode, "RSA", true))
        {
            mode = PSBC_PinPadDevAdapter::RSA_Mode;
        }
        if(!_guomiAdapter.SetAlgorithmMode(mode))
            return false;
        const size_t MAC_KEYID_LENGTH = 13;
        ByteBuilder wkId(MAC_KEYID_LENGTH);
        wkId = sWkIndex.c_str();
        ByteConvert::Fill(wkId, MAC_KEYID_LENGTH, false, '0');
        if(!_guomiAdapter.SetWK_ID(wkId))
            return false;
        if(!_guomiAdapter.SetPinblock(padByte))
            return false;
        if(!_guomiAdapter.SetPinLength(pinLength))
            return false;

        wkId = sCardNumber.c_str();
        const size_t CARD_LENGTH = 12;
        if(wkId.GetLength() > CARD_LENGTH)
            wkId.RemoveTail();
        ByteConvert::Fill(wkId, CARD_LENGTH, false, '0');
        ByteBuilder pinBlock(8);
        if(!_guomiAdapter.GetPwd(wkId, pinBlock, _itobyte(timeoutS)))
            return false;

        ByteBuilder tmp(8);
        ByteConvert::ToAscii(pinBlock, tmp);
        rlt.PushValue("PIN", tmp.GetString());
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