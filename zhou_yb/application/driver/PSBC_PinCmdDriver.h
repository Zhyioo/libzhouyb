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
/// 邮储国密键盘命令驱动
class PSBC_PinCmdDriver : 
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public CommandCollection,
    public ILastErrBehavior,
    public ILoggerBehavior,
    public RefObject
{
protected:
    LastErrExtractor _appErr;
    LastErrExtractor _lastErr;
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
        _lastErr.Select(_cmdAdapter, "Cmd");
        _lastErr.Select(_appErr);

        _Bind("ResetKey", (*this), &PSBC_PinCmdDriver::ResetKey);
        _Bind("SetPinLength", (*this), &PSBC_PinCmdDriver::SetPinLength);
        _Bind("DownloadMK", (*this), &PSBC_PinCmdDriver::DownloadMK);
        _Bind("DownloadWK", (*this), &PSBC_PinCmdDriver::DownloadWK);
        _Bind("WaitPassword_Ansi98", (*this), &PSBC_PinCmdDriver::WaitPassword_Ansi98);
        _Bind("GetPassword_Ansi98", (*this), &PSBC_PinCmdDriver::GetPassword_Ansi98);
    }
    /// 适配设备
    virtual void SelectDevice(const Ref<IInteractiveTrans>& dev)
    {
        BaseDevAdapterBehavior::SelectDevice(dev);
        _pinCmdAdapter.SelectDevice(_pDev);
        _cmdAdapter.SelectDevice(_pDev);
        _sm4Adapter.SelectDevice(_pinCmdAdapter);
        _desAdapter.SelectDevice(_pinCmdAdapter);
        _guomiAdapter.SelectDevice(_cmdAdapter);
        _managerAdapter.SelectDevice(_pinCmdAdapter);
    }
    /// 释放设备
    virtual void ReleaseDevice()
    {
        BaseDevAdapterBehavior::ReleaseDevice();
        _pinCmdAdapter.ReleaseDevice();
        _cmdAdapter.ReleaseDevice();
    }
    /// 选择日志
    virtual void SelectLogger(const LoggerAdapter& log)
    {
        _cmdAdapter.SelectLogger(log);
        _sm4Adapter.SelectLogger(log);
        _desAdapter.SelectLogger(log);
        _guomiAdapter.SelectLogger(log);
        _managerAdapter.SelectLogger(log);
    }
    /// 获取日志
    virtual const LoggerAdapter& GetLogger()
    {
        return _cmdAdapter.GetLogger();
    }
    /// 释放日志
    virtual void ReleaseLogger(const LoggerAdapter* plog = NULL)
    {
        _cmdAdapter.ReleaseLogger(plog);
        _sm4Adapter.ReleaseLogger(plog);
        _desAdapter.ReleaseLogger(plog);
        _guomiAdapter.ReleaseLogger(plog);
        _managerAdapter.ReleaseLogger(plog);
    }

    /// 错误信息
    virtual int GetLastErr() const
    {
        return _lastErr.GetLastErr();
    }
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage()
    {
        return _lastErr.GetErrMessage();
    }
    /// 重置错误信息
    virtual void ResetErr()
    {
        return _lastErr.ResetErr();
    }

    /**
     * @brief 初始化密钥为指定值
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - Algorithm [DES][SM4] 算法标识
     *  - KeyIndex 主密钥索引
     *  - Key 重置的新密钥
     * .
     */
    LC_CMD_METHOD(ResetMK)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["KeyIndex"].To<byte>();
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
     *  - KeyIndex 主密钥索引
     * .
     */
    LC_CMD_METHOD(ResetKey)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["KeyIndex"].To<byte>();

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
     *  - KeyIndex 主密钥索引
     *  - OldKey 原来的主密钥明文
     *  - NewKey 重置的新密钥明文
     * .
     */
    LC_CMD_METHOD(UpdateMainKey)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["KeyIndex"].To<byte>();
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
     *  - Algorithm [DES][SM4] 算法标识
     *  - KeyIndex 主密钥索引 
     *  - KEY 主密钥密文
     *  - KCV 主密钥KCV
     * .
     */
    LC_CMD_METHOD(DownloadMK)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["KeyIndex"].To<byte>();
        string key = arg["KEY"].To<string>();
        string kcv = arg["KCV"].To<string>();

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
     * @brief 密文下载工作密钥
     * 
     * @param [in] arglist
     * - 参数:
     *  - Algorithm [DES][SM4] 算法标识
     *  - MkIndex 主密钥索引
     *  - WkIndex 工作密钥索引
     *  - KEY 工作密钥密文
     *  - KCV 工作密钥KCV
     * .
     */
    LC_CMD_METHOD(DownloadWK)
    {
        string algorithm = arg["Algorithm"].To<string>();
        byte mkIndex = arg["MkIndex"].To<byte>();
        byte wkIndex = arg["WkIndex"].To<byte>();
        string key = arg["KEY"].To<string>();
        string kcv = arg["KCV"].To<string>();

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

    /**
     * @brief 评价
     * 
     * @param [in] arglist
     *
     * @return 评价描述字符 满意/不满意/非常满意
     *   
     */
    LC_CMD_METHOD(Evaluation)
    {
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