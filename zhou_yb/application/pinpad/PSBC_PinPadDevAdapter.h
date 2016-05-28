//========================================================= 
/**@file PSBC_PinPadDevAdapter.h 
 * @brief 邮储国密键盘指令集封装 
 * 
 * @date 2014-11-25   19:06:49 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PSBC_PINPADDEVADAPTER_H_
#define _LIBZHOUYB_PSBC_PINPADDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace pinpad {
//--------------------------------------------------------- 
/// 密钥块的大小 
#define PSBC_KEY_BLOCK_SIZE (8)
/// 拆字的补位数据 
#define PSBC_EXTEND_BYTE (0x30)
//--------------------------------------------------------- 
/// 邮储国密键盘管理类指令 
class PSBC_PinManagerDevAdapter : public DevAdapterBehavior<IInteractiveTrans>, public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区 
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 评价状态 
    enum EvaluationStatus
    {
        /// 未知
        Unknown,
        /// 满意 
        Satisfied,
        /// 一般
        General,
        /// 不满意 
        Dissatisfied
    };
    //----------------------------------------------------- 
    /// 将评价信息转换为字符串
    static const char* EvaluationTostring(EvaluationStatus status)
    {
        switch(status)
        {
        case Satisfied:
            return "满意";
        case General:
            return "一般";
        case Dissatisfied:
            return "不满意";
        }
        return "未评价";
    }
    //----------------------------------------------------- 
    /**
     * @brief 评价(无声音) 
     * 
     * @param [out] status 评价信息
     */
    bool Evaluation(EvaluationStatus& status)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1BY"
        DevCommand::FromAscii("1B 59", _sendBuffer);
        
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 0, DeviceError::RecvFormatErr);

        status = static_cast<EvaluationStatus>(_recvBuffer[0]);
        LOGGER(_log << "评价信息:<" << EvaluationTostring(status) << ">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 评价(带语音)
     * 
     * @param [out] status 评价信息 
     * @param [in] isVoice [default:true] 是否进行语音提示,否的话则蜂鸣器提示 
     */
    bool EvaluationVoice(EvaluationStatus& status, bool isVoice = true)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "是否语音提示:<" << isVoice << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1BGS"
        DevCommand::FromAscii("1B 47 53", _sendBuffer);
        _sendBuffer += static_cast<byte>(isVoice ? 0x00 : 0x01);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 0, DeviceError::RecvFormatErr);

        status = static_cast<EvaluationStatus>(_recvBuffer[0]);
        LOGGER(_log << "评价信息:<" << EvaluationTostring(status) << ">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 输入信息 
     * 
     * @param [out] data 输入的信息 
     * @param [in] isVoice [default:true] 是否进行语音提示 
     */
    bool InputInformation(ByteBuilder& data, bool isVoice = true)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "是否语音提示:<" << isVoice << ">\n"
            <<"输入方式:<打包返回>\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1BGI";
        DevCommand::FromAscii("1B 47 49", _sendBuffer);
        _sendBuffer += static_cast<byte>(isVoice ? 0x00 : 0x01);
        _sendBuffer += static_cast<byte>(0x01);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "输入信息:\n" << _recvBuffer << endl);
        data += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 输入信息 
     * 
     * @param [in] isVoice 是否进行语音提示 
     */
    bool InputInformation(bool isVoice = true)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "是否语音提示:<" << isVoice << ">\n"
            << "输入方式:<每个按键单独返回>\n");
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1BGI";
        DevCommand::FromAscii("1B 47 49", _sendBuffer);
        _sendBuffer += static_cast<byte>(isVoice ? 0x00 : 0x01);
        _sendBuffer += static_cast<byte>(0x00);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    /**
     * @brief 确认信息 
     * 
     * @param [out] pResult [default:NULL] 确认结果 "确认"或"取消",NULL表示不需要该信息 
     */
    bool DisplayInformation(bool* pResult = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1BGC";
        DevCommand::FromAscii("1B 47 43", _sendBuffer);
        _sendBuffer += static_cast<byte>(pResult == NULL ? 0x01 : 0x00);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        if(pResult != NULL)
        {
            _recvBuffer.Clear();
            ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
            ASSERT_FuncErrRet(_recvBuffer.GetLength() > 0, DeviceError::RecvFormatErr);

            (*pResult) = (_recvBuffer[0] == 0);
        }

        return _logRetValue(true);
    }
    /**
     * @brief 设置应用编号 
     * 
     * @param [in] strNumber 需要更新的序列号 
     */
    bool SetAppNumber(const ByteArray& appNumber)
    {
        LOG_FUNC_NAME();
        LOGGER(_log.WriteLine("应用编号:").WriteLine(appNumber));
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1Bsw";
        DevCommand::FromAscii("1B 73 77", _sendBuffer);
        ByteConvert::Expand(appNumber, _sendBuffer, PSBC_EXTEND_BYTE);
        byte lrc = ByteConvert::XorVal(appNumber);
        _sendBuffer += lrc;
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 获取应用编号 
     * 
     * @param [out] appNumber 获取到的应用编号 
     */
    bool GetAppNumber(ByteBuilder& appNumber)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1Bsr";
        DevCommand::FromAscii("1B 73 72", _sendBuffer);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("应用编号:").WriteLine(_recvBuffer));
        appNumber += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 读取输入到的密文密码
     *
     * @param [out] pin 输入的密码密文
     * @param [out] pKeyLen [default:NULL] 用来加密的密钥长度 
     */
    bool GetPassword(ByteBuilder& pin, size_t* pKeyLen = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 0, DeviceError::RecvFormatErr);
        if(pKeyLen != NULL)
            (*pKeyLen) = _recvBuffer[0];
        LOGGER(_log.WriteLine("密文PIN:").WriteLine(_recvBuffer));
        pin.Append(_recvBuffer.SubArray(1));
        return _logRetValue(true);
    }
    /**
     * @brief 设置密码长度
     * 
     * @param [in] pinlen 需要设置的密码长度,32长度以内 
     */
    bool SetPinLength(byte pinlen)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();
        // "\x1BN"
        DevCommand::FromAscii("1B 4E", _sendBuffer);
        ByteConvert::Expand(ByteArray(&pinlen, 1), _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        // 接收AA
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 0 && _recvBuffer[0] == 0xAA, DeviceError::RecvFormatErr);
        return _logRetValue(true);
    }
    /**
     * @brief 取序列号 
     * 
     * @param [in] serialNumber 获取到的序列号 
     */
    bool GetSerialNumber(ByteBuilder& serialNumber)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("1B 57", _sendBuffer);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        LOGGER(_log << "SerialNumber:<";
        _log.WriteStream(_recvBuffer) << ">\n");

        serialNumber += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 取消输入
     */
    bool CancelInput()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ASSERT_FuncErrRet(_pDev->Write(DevCommand::FromAscii("83")), DeviceError::SendErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// DES相关指令 
class PSBC_PinDesDevAdapter : public DevAdapterBehavior<IInteractiveTrans>, public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区 
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 重置密钥(恢复后密钥为:8个字节3X)
     *
     * @param [in] keyIndex [default:'A'] 需要重置的密钥号 [0,8), 'A':重置所有密钥
     */
    bool ResetKey(byte keyIndex = 'A')
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "KeyIndex:<" << _hex(keyIndex) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1Bd"
        DevCommand::FromAscii("1B 64", _sendBuffer);
        if(keyIndex == 'A')
            _sendBuffer += keyIndex;
        else
            _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 明文修改主密钥 
     * 
     * @param [in] keyIndex 密钥索引 
     * @param [in] oldMKey_8_16_24 旧密钥明文 
     * @param [in] newMKey_8_16_24 新密钥明文 
     */
    bool UpdateMainKey(byte keyIndex, const ByteArray& oldMKey_8_16_24, const ByteArray& newMKey_8_16_24)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("旧密钥:").WriteLine(oldMKey_8_16_24);
        _log.WriteLine("新密钥:").WriteLine(newMKey_8_16_24));
        ASSERT_Device();
        ASSERT_FuncErrRet(oldMKey_8_16_24.GetLength() % PSBC_KEY_BLOCK_SIZE == 0, DeviceError::ArgLengthErr);
        ASSERT_FuncErrRet(newMKey_8_16_24.GetLength() % PSBC_KEY_BLOCK_SIZE == 0, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        // "\x1BkM"
        DevCommand::FromAscii("1B 6B 4D", _sendBuffer);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (_itobyte(oldMKey_8_16_24.GetLength() / PSBC_KEY_BLOCK_SIZE) + PSBC_EXTEND_BYTE);
        ByteConvert::Expand(oldMKey_8_16_24, _sendBuffer, PSBC_EXTEND_BYTE);
        _sendBuffer += (_itobyte(newMKey_8_16_24.GetLength() / PSBC_KEY_BLOCK_SIZE) + PSBC_EXTEND_BYTE);
        ByteConvert::Expand(newMKey_8_16_24, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 密文下载主密钥 
     * 
     * @param [in] keyIndex 需要下载的密钥索引 [0,8)
     * @param [in] encryptedMK 加密的密钥数据 
     * @param [in] kcv 密钥的KCV数据 
     */
    bool UpdateEncryptedMainKey(byte keyIndex, const ByteArray& encryptedMK, const ByteArray& kcv)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("密钥密文:").WriteLine(encryptedMK);
        _log.WriteLine("KCV:").WriteLine(kcv));
        ASSERT_Device();
        ASSERT_FuncErrRet(encryptedMK.GetLength() % PSBC_KEY_BLOCK_SIZE == 0, DeviceError::ArgLengthErr);
        _sendBuffer.Clear();
        // "\x1Bkm"
        DevCommand::FromAscii("1B 6B 6D", _sendBuffer);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (_itobyte(encryptedMK.GetLength() / PSBC_KEY_BLOCK_SIZE) + PSBC_EXTEND_BYTE);
        ByteConvert::Expand(encryptedMK, _sendBuffer, PSBC_EXTEND_BYTE);
        ByteConvert::Expand(kcv, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 密文下载工作密钥 
     * 
     * @param [in] mkeyIndex 需要下载的密钥区 
     * @param [in] keyIndex 需要下载的工作密钥索引
     * @param [in] encryptedWK 加密的工作密钥数据 
     * @param [in] kcv 密钥的KCV数据 
     */
    bool UpdateEncryptedWorkKey(byte mkeyIndex, byte keyIndex, const ByteArray& encryptedWK, const ByteArray& kcv)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkeyIndex) << ">\n"
            << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("密钥密文:").WriteLine(encryptedWK);
        _log.WriteLine("KCV:").WriteLine(kcv));
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1Bku"
        DevCommand::FromAscii("1B 6B 75", _sendBuffer);
        _sendBuffer += (mkeyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (_itobyte(encryptedWK.GetLength() / PSBC_KEY_BLOCK_SIZE) + PSBC_EXTEND_BYTE);
        ByteConvert::Expand(encryptedWK, _sendBuffer, PSBC_EXTEND_BYTE);
        ByteConvert::Expand(kcv, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 主密钥加密数据 
     * 
     * @param [in] keyIndex 密钥索引 
     * @param [in] iv_8 向量,ECB时为""
     * @param [in] data_8 待加密的数据 
     * @param [out] buff_8 加密后的数据 
     * @param [out] pKeyLen [default:NULL] 加密的密钥长度 
     */
    bool MKeyEncrypt(byte keyIndex, const ByteArray& iv_8, const ByteArray& data_8, ByteBuilder& buff_8, size_t* pKeyLen = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("IV:").WriteLine(iv_8);
        _log.WriteLine("数据:").WriteLine(data_8));
        ASSERT_Device();
        ASSERT_FuncErrRet(iv_8.GetLength() == PSBC_KEY_BLOCK_SIZE || iv_8.IsEmpty(), DeviceError::ArgFormatErr);
        ASSERT_FuncErrRet(data_8.GetLength() == PSBC_KEY_BLOCK_SIZE, DeviceError::ArgFormatErr);

        _sendBuffer.Clear();
        // "\x1Bhm"
        DevCommand::FromAscii("1B 68 6D", _sendBuffer);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        // ECB方式 
        if(iv_8.IsEmpty())
        {
            _sendBuffer += static_cast<byte>(0x10);
            _sendBuffer.Append(static_cast<byte>(PSBC_EXTEND_BYTE), 2*PSBC_KEY_BLOCK_SIZE);
        }
        else
        {
            _sendBuffer += static_cast<byte>(0x20);
            ByteConvert::Expand(iv_8, _sendBuffer, PSBC_EXTEND_BYTE);
        }
        ByteConvert::Expand(data_8, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("密文数据:").WriteLine(_recvBuffer));
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > PSBC_KEY_BLOCK_SIZE, DeviceError::RecvFormatErr);
        if(pKeyLen != NULL)
            (*pKeyLen) = _recvBuffer[0];
        buff_8.Append(_recvBuffer.SubArray(1));

        return _logRetValue(true);
    }
    /**
     * @brief 工作密钥加密数据 
     * 
     * @param [in] mkIndex 密钥需编号(主密钥)
     * @param [in] keyIndex 工作密钥号 
     * @param [in] iv_8 向量,ECB为""
     * @param [in] data_8 待加密的数据 
     * @param [out] buff_8 加密后的数据 
     * @param [out] pKeyLen [default:NULL] 加密的密钥长度 
     */
    bool WKeyEncrypt(byte mkIndex, byte keyIndex, const ByteArray& iv_8, const ByteArray& data_8, ByteBuilder& buff_8, size_t* pKeyLen = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("IV:").WriteLine(iv_8);
        _log.WriteLine("数据:").WriteLine(data_8));
        ASSERT_Device();
        ASSERT_FuncErrRet(iv_8.GetLength() == PSBC_KEY_BLOCK_SIZE || iv_8.IsEmpty(), DeviceError::ArgFormatErr);
        ASSERT_FuncErrRet(data_8.GetLength() == PSBC_KEY_BLOCK_SIZE, DeviceError::ArgFormatErr);

        _sendBuffer.Clear();
        // "\x1Bhu"
        DevCommand::FromAscii("1B 68 75", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        // ECB方式 
        if(iv_8.IsEmpty())
        {
            _sendBuffer += static_cast<byte>(0x10);
            _sendBuffer.Append(static_cast<byte>(PSBC_EXTEND_BYTE), 2 * PSBC_KEY_BLOCK_SIZE);
        }
        else
        {
            _sendBuffer += static_cast<byte>(0x20);
            ByteConvert::Expand(iv_8, _sendBuffer, PSBC_EXTEND_BYTE);
        }
        ByteConvert::Expand(data_8, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("密文数据:").WriteLine(_recvBuffer));
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > PSBC_KEY_BLOCK_SIZE, DeviceError::RecvFormatErr);
        if(pKeyLen != NULL)
            (*pKeyLen) = _recvBuffer[0];
        buff_8.Append(_recvBuffer.SubArray(1));

        return _logRetValue(true);
    }
    /**
     * @brief 等待输入密文密码 
     * 
     * @param [in] mkIndex 密钥区编号 
     * @param [in] wkIndex 工作密钥号 
     * @param [in] isReinput 是否再次输入 
     */
    bool WaitPassword(byte mkIndex, byte wkIndex, bool isReinput)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "是否再次输入:<" << isReinput << ">\n");
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1Bip"
        DevCommand::FromAscii("1B 69 70", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (wkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (static_cast<byte>(isReinput ? 0x01 : 0x00) + PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    /**
     * @brief 计算MAC 
     * 
     * @param [in] mkIndex 密钥区编号 
     * @param [in] wkIndex 密钥索引  
     * @param [in] macData 待计算的MAC数据 
     * @param [out] mac 计算后得到的MAC 
     */
    bool GetMAC(byte mkIndex, byte wkIndex, const ByteArray& macData, ByteBuilder& mac)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(wkIndex) << ">\n";
        _log.WriteLine("MAC数据:").WriteLine(macData));
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1Bim"
        DevCommand::FromAscii("1B 69 6D", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (wkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += _itobyte(macData.GetLength());
        ByteConvert::Expand(macData, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("MAC数据:").WriteLine(_recvBuffer));
        mac += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 输入密码(ANSI9.8)
     * 
     * @param [in] mkIndex 密钥区 
     * @param [in] wkIndex 工作密钥索引 
     * @param [in] isReinput 是否再次输入(语音提示)
     * @param [in] accno_12 账户信息 
     */
    bool WaitPassword_Ansi98(byte mkIndex, byte wkIndex, bool isReinput, const ByteArray& accno_12)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "是否再次输入:<" << isReinput << ">\n";
        _log.WriteLine("账户号:").WriteLine(accno_12));
        ASSERT_Device();
        ASSERT_FuncErrRet(accno_12.GetLength() == 12, DeviceError::ArgLengthErr);
        _sendBuffer.Clear();
        _recvBuffer.Clear();
        // "\x1Bic"
        DevCommand::FromAscii("1B 69 63", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (wkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (static_cast<byte>(isReinput ? 0x01 : 0x00) + PSBC_EXTEND_BYTE);
        _sendBuffer += accno_12;
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /* 封装后的接口 */
    /**
     * @brief 重置主密钥
     * 
     * @param [in] mkIndex 需要重置的主密钥ID
     * @param [in] mk_8_16_24 重新重置后的主密钥明文,为空则重置为16个0x00
     * @param [in] pad [default:0x31] 密钥偏移字节
     * @param [in] keylen [default:24] 初始密钥长度
     */
    bool ResetMK(byte mkIndex, const ByteArray& mk_8_16_24, byte pad = 0x31, size_t keylen = 0x18)
    {
        LOG_FUNC_NAME();
        // 重置密钥 
        ASSERT_FuncErrRet(ResetKey(mkIndex), DeviceError::DevInitErr);

        byte defaultKey[16] = { 0 };
        ByteArray key(mk_8_16_24);
        ByteBuilder oldKey(16);

        if(mk_8_16_24.IsEmpty())
        {
            key = ByteArray(defaultKey, 16);
        }
        ASSERT_FuncErrRet(key.GetLength() % PSBC_KEY_BLOCK_SIZE == 0, DeviceError::ArgLengthErr);
        // 等待擦除重写密钥
        Timer::Wait(DEV_OPERATOR_INTERVAL);
        oldKey.Append(static_cast<byte>(pad + mkIndex), keylen);
        bool bWait = UpdateMainKey(mkIndex, oldKey, key);
        ASSERT_FuncErrRet(bWait, DeviceError::OperatorErr);
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// SM4相关指令 
class PSBC_PinSm4DevAdapter : public DevAdapterBehavior<IInteractiveTrans>, public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区 
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 重置密钥(恢复后密钥为:8个字节3X)
     *
     * @param [in] keyIndex [default:'A'] 需要重置的密钥号 [0,8), 'A':重置所有密钥
     */
    bool ResetKey(byte keyIndex = 'A')
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "KeyIndex:<" << _hex(keyIndex) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        // "\x1BCd";
        DevCommand::FromAscii("1B 43 64", _sendBuffer);
        if(keyIndex == 'A')
            _sendBuffer += keyIndex;
        else
            _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 明文修改主密钥
     *
     * @param [in] keyIndex 密钥索引
     * @param [in] oldMKey_16 旧密钥明文
     * @param [in] newMKey_16 新密钥明文
     */
    bool UpdateMainKey(byte keyIndex, const ByteArray& oldMKey_16, const ByteArray& newMKey_16)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("旧密钥:").WriteLine(oldMKey_16);
        _log.WriteLine("新密钥:").WriteLine(newMKey_16));
        ASSERT_Device();
        ASSERT_FuncErrRet(oldMKey_16.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE), DeviceError::ArgLengthErr);
        ASSERT_FuncErrRet(newMKey_16.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE), DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        // "\x1BCM";
        DevCommand::FromAscii("1B 43 4D", _sendBuffer);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += static_cast<byte>(0x32);
        ByteConvert::Expand(oldMKey_16, _sendBuffer, PSBC_EXTEND_BYTE);
        _sendBuffer += static_cast<byte>(0x32);
        ByteConvert::Expand(newMKey_16, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 密文下载主密钥
     *
     * @param [in] keyIndex 需要下载的密钥索引 [0,8)
     * @param [in] encryptedMK 加密的密钥数据
     * @param [in] kcv 密钥的KCV数据
     */
    bool UpdateEncryptedMainKey(byte keyIndex, const ByteArray& encryptedMK, const ByteArray& kcv)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("密钥密文:").WriteLine(encryptedMK);
        _log.WriteLine("KCV:").WriteLine(kcv));
        ASSERT_Device();
        ASSERT_FuncErrRet(encryptedMK.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE), DeviceError::ArgLengthErr);
        _sendBuffer.Clear();
        // "\x1BCm";
        DevCommand::FromAscii("1B 43 6D", _sendBuffer);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += static_cast<byte>(0x32);
        ByteConvert::Expand(encryptedMK, _sendBuffer, PSBC_EXTEND_BYTE);
        ByteConvert::Expand(kcv, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 密文下载工作密钥
     *
     * @param [in] mkeyIndex 需要下载的密钥区
     * @param [in] keyIndex 需要下载的工作密钥索引
     * @param [in] encryptedWK 加密的工作密钥数据
     * @param [in] kcv 密钥的KCV数据
     */
    bool UpdateEncryptedWorkKey(byte mkeyIndex, byte keyIndex, const ByteArray& encryptedWK, const ByteArray& kcv)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkeyIndex) << ">\n"
            << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("密钥密文:").WriteLine(encryptedWK);
        _log.WriteLine("KCV:").WriteLine(kcv));
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1BCu";
        DevCommand::FromAscii("1B 43 75", _sendBuffer);
        _sendBuffer += (mkeyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += static_cast<byte>(0x32);
        ByteConvert::Expand(encryptedWK, _sendBuffer, PSBC_EXTEND_BYTE);
        ByteConvert::Expand(kcv, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 主密钥加密数据
     *
     * @param [in] keyIndex 密钥索引
     * @param [in] iv_16 向量,ECB时为""
     * @param [in] data_16 待加密的数据
     * @param [out] buff_16 加密后的数据
     * @param [out] pKeyLen [default:NULL] 加密的密钥长度
     */
    bool MKeyEncrypt(byte keyIndex, const ByteArray& iv_16, const ByteArray& data_16, ByteBuilder& buff_16, size_t* pKeyLen = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("IV:").WriteLine(iv_16);
        _log.WriteLine("数据:").WriteLine(data_16));
        ASSERT_Device();
        ASSERT_FuncErrRet(iv_16.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE) || iv_16.IsEmpty(), DeviceError::ArgFormatErr);
        ASSERT_FuncErrRet(data_16.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE), DeviceError::ArgFormatErr);

        _sendBuffer.Clear();
        // "\x1BCe";
        DevCommand::FromAscii("1B 43 65", _sendBuffer);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        // ECB方式 
        if(iv_16.IsEmpty())
        {
            _sendBuffer += static_cast<byte>(0x10);
            _sendBuffer.Append(static_cast<byte>(PSBC_EXTEND_BYTE), 4 * PSBC_KEY_BLOCK_SIZE);
        }
        else
        {
            _sendBuffer += static_cast<byte>(0x20);
            ByteConvert::Expand(iv_16, _sendBuffer, PSBC_EXTEND_BYTE);
        }
        ByteConvert::Expand(data_16, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("密文数据:").WriteLine(_recvBuffer));
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > PSBC_KEY_BLOCK_SIZE, DeviceError::RecvFormatErr);
        if(pKeyLen != NULL)
            (*pKeyLen) = _recvBuffer[0];
        buff_16.Append(_recvBuffer.SubArray(1));

        return _logRetValue(true);
    }
    /**
     * @brief 工作密钥加密数据
     *
     * @param [in] mkIndex 密钥需编号(主密钥)
     * @param [in] keyIndex 工作密钥号
     * @param [in] iv_16 向量,ECB为""
     * @param [in] data_16 待加密的数据
     * @param [out] buff_16 加密后的数据
     * @param [out] pKeyLen [default:NULL] 加密的密钥长度
     */
    bool WKeyEncrypt(byte mkIndex, byte keyIndex, const ByteArray& iv_16, const ByteArray& data_16, ByteBuilder& buff_16, size_t* pKeyLen = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(keyIndex) << ">\n";
        _log.WriteLine("IV:").WriteLine(iv_16);
        _log.WriteLine("数据:").WriteLine(data_16));
        ASSERT_Device();
        ASSERT_FuncErrRet(iv_16.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE) || iv_16.IsEmpty(), DeviceError::ArgFormatErr);
        ASSERT_FuncErrRet(data_16.GetLength() == (2 * PSBC_KEY_BLOCK_SIZE), DeviceError::ArgFormatErr);

        _sendBuffer.Clear();
        // "\x1BCu";
        DevCommand::FromAscii("1B 43 75", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (keyIndex + PSBC_EXTEND_BYTE);
        // ECB方式 
        if(iv_16.IsEmpty())
        {
            _sendBuffer += static_cast<byte>(0x10);
            _sendBuffer.Append(static_cast<byte>(PSBC_EXTEND_BYTE), 4 * PSBC_KEY_BLOCK_SIZE);
        }
        else
        {
            _sendBuffer += static_cast<byte>(0x20);
            ByteConvert::Expand(iv_16, _sendBuffer, PSBC_EXTEND_BYTE);
        }
        ByteConvert::Expand(data_16, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("密文数据:").WriteLine(_recvBuffer));
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > (2 * PSBC_KEY_BLOCK_SIZE), DeviceError::RecvFormatErr);
        if(pKeyLen != NULL)
            (*pKeyLen) = _recvBuffer[0];
        buff_16.Append(_recvBuffer.SubArray(1));

        return _logRetValue(true);
    }
    /**
     * @brief 输入密文密码
     *
     * @param [in] mkIndex 密钥区编号
     * @param [in] wkIndex 工作密钥号
     * @param [in] isReinput 是否再次输入
     */
    bool WaitPassword(byte mkIndex, byte wkIndex, bool isReinput)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "是否再次输入:<" << isReinput << ">\n");
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1BtP";
        DevCommand::FromAscii("1B 74 50", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (wkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (static_cast<byte>(isReinput ? 0x01 : 0x00) + PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    /**
     * @brief 计算MAC
     *
     * @param [in] mkIndex 密钥区编号
     * @param [in] wkIndex 密钥索引
     * @param [in] macData 待计算的MAC数据
     * @param [out] mac 计算后得到的MAC
     */
    bool GetMAC(byte mkIndex, byte wkIndex, const ByteArray& macData, ByteBuilder& mac)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(wkIndex) << ">\n";
        _log.WriteLine("MAC数据:").WriteLine(macData));
        ASSERT_Device();
        _sendBuffer.Clear();
        // "\x1Btm";
        DevCommand::FromAscii("1B 74 6D", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (wkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (_itobyte(macData.GetLength()) + PSBC_EXTEND_BYTE);
        ByteConvert::Expand(macData, _sendBuffer, PSBC_EXTEND_BYTE);
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("MAC数据:").WriteLine(_recvBuffer));
        mac += _recvBuffer;

        return _logRetValue(true);
    }
    /**
    * @brief 输入密码(ANSI9.8)
    *
    * @param [in] mkIndex 密钥区
    * @param [in] wkIndex 工作密钥索引
    * @param [in] isReinput 是否再次输入(语音提示)
    * @param [in] accno_12 账户信息
    */
    bool WaitPassword_Ansi98(byte mkIndex, byte wkIndex, bool isReinput, const ByteArray& accno_12)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥区:<" << _hex(mkIndex) << ">\n";
        _log << "密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "是否再次输入:<" << isReinput << ">\n";
        _log.WriteLine("账户号:").WriteLine(accno_12));
        ASSERT_Device();
        ASSERT_FuncErrRet(accno_12.GetLength() == 12, DeviceError::ArgLengthErr);
        _sendBuffer.Clear();
        // "\x1Btc";
        DevCommand::FromAscii("1B 74 63", _sendBuffer);
        _sendBuffer += (mkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (wkIndex + PSBC_EXTEND_BYTE);
        _sendBuffer += (static_cast<byte>(isReinput ? 0x01 : 0x00) + PSBC_EXTEND_BYTE);
        _sendBuffer += accno_12;
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /* 封装后的接口 */
    /**
     * @brief 重置主密钥
     * 
     * @param [in] mkIndex 需要重置的主密钥ID
     * @param [in] mk_16 需要重置的主密钥,为空则重置为16个0x00
     * @param [in] pad [default:0x31] 密钥字节
     */
    bool ResetMK(byte mkIndex, const ByteArray& mk_16, byte pad = 0x31)
    {
        LOG_FUNC_NAME();
        ASSERT_FuncErrRet(ResetKey(mkIndex), DeviceError::DevInitErr);

        byte defaultKey[32] = { 0 };
        ByteArray key(mk_16);
        if(mk_16.IsEmpty())
        {
            key = ByteArray(defaultKey, 16);
        }
        ASSERT_FuncErrRet(key.GetLength() == 16, DeviceError::ArgLengthErr);
        // 等待擦除重写密钥
        Timer::Wait(DEV_OPERATOR_INTERVAL);
        ByteBuilder oldKey(8);
        oldKey.Append(static_cast<byte>(pad + mkIndex), 16);
        ASSERT_FuncErrRet(UpdateMainKey(mkIndex, oldKey, key), DeviceError::OperatorErr);
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 邮储国密键盘指令集 
class PSBC_PinPadDevAdapter : public DevAdapterBehavior<IInteractiveTrans>, public RefObject
{
public:
    //----------------------------------------------------- 
    /// 算法模式 
    enum AlgorithmMode
    {
        /// 未知的模式 
        UnknownMode = 0,
        /// DES算法 
        DES_Mode,
        /// 3DES算法 
        DES3_Mode,
        /// SM2算法 
        SM2_Mode,
        /// SM4算法 
        SM4_Mode,
        /// RSA算法 
        RSA_Mode
    };
    /// 长度标识 
    enum Length
    {
        /// DES KCV长度 
        DES_KCV_Length = 4,
        /// SM4 KCV长度 
        SM4_KCV_Length = 8,
        /// 帐号信息长度
        CardID_Length = 12,
        /// 密钥ID长度 
        KeyID_Length = 13
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 发送数据的临时缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收数据的临时缓冲区 
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 将标准的SM2密钥数据转换为密码键盘格式 
     * 
     * @param [in] sm2EncryptKey 原始的SM2密文数据 
     * @param [out] encryptKey 密码键盘需要的格式 
     */
    static bool PackEncryptSM2Key(const ByteArray& sm2EncryptKey, ByteBuilder& encryptKey)
    {
        // 97为SM2加密信息基本数据的长度(不包括加密的密文) 
        const size_t SM2_BASE_LENGTH = 97;
        if(sm2EncryptKey.GetLength() < SM2_BASE_LENGTH)
            return false;

        encryptKey.Append(static_cast<byte>(0x00), 2);
        // C1
        encryptKey.Append(sm2EncryptKey.SubArray(1, 64));
        // C2
        size_t dataLen = sm2EncryptKey.GetLength() - SM2_BASE_LENGTH;
        encryptKey.Append(sm2EncryptKey.SubArray(64 + 1, dataLen));
        // 136为固件固定的加密数据长度,遇到0x00截止(故有一定的隐患BUG存在) 
        encryptKey.Append((byte)0x00, 136 - dataLen);
        // C3
        encryptKey.Append(sm2EncryptKey.SubArray(64 + dataLen + 1, 32));

        return true;
    }
    /**
     * @brief 获取授权码 
     * @param [in] len 授权码的长度 
     * @param [in] waitTimeout 超时时间(s)
     * @param [out] authCode 获取到的授权码 
     */
    bool GetAuthCode(byte len, byte waitTimeout, ByteBuilder& authCode)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log << "授权码长度:<" << static_cast<int>(len) << ">\n";
        _log << "超时时间:<" << static_cast<int>(waitTimeout) << ">\n");

        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("30 01", _sendBuffer);
        _sendBuffer += len;
        _sendBuffer += waitTimeout;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(authCode), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 产生RSA公钥 
     * @param [in] bits RSA的位长 [0-2048]
     * @param [out] rsaPublicKey 产生的RSA公钥数据 
     */ 
    bool GenerateKEY_RSA(uint bits, ByteBuilder& rsaPublicKey)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "位长:<" << bits << ">\n");

        ASSERT_Device();
        ASSERT_FuncErrRet(((bits % 8) == 0) && (bits <= 2048), DeviceError::ArgRangeErr);

        _sendBuffer.Clear();
        DevCommand::FromAscii("30 02 01", _sendBuffer);
        if(bits == 2048)
            _sendBuffer += static_cast<byte>(0x01);
        else
            _sendBuffer += _itobyte(bits / 8);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(rsaPublicKey), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 产生SM2公钥
     * @param [in] sm2PublicKey_64 产生的SM2公钥 dB
     */
    bool GenerateKEY_SM2(ByteBuilder& sm2PublicKey_64)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("30 02 02 00", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(sm2PublicKey_64), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 下载SM2主密钥数据(新设备直接使用DownloadMK下载)
     * @date 2016-05-27 15:07
     * 
     * @param [in] encryptKey 需要下载的主密钥密文
     * @warning 数据必须为C1 C2 C3的格式
     *
     * @param [in] mkID_13 需要下载的主密钥ID
     * @param [in] kcv_8 密钥的KCV
     */
    bool DownloadMK_SM2(const ByteArray& encryptKey, const ByteArray& mkID_13, const ByteArray& kcv_8)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "EncryptKey:\n" << encryptKey << endl);
        LOGGER(_log << "MkID:<";_log.WriteStream(mkID_13) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_8) << ">\n");

        // C1 C3 C2
        const size_t KCV_LENGTH = 8;
        // 随机点
        const size_t C1_LENGTH = 64;
        // HASH值
        const size_t C2_LENGTH = 32;
        // 密码键盘C3固定长度
        const size_t C3_LENGTH = 136;
        
        ASSERT_FuncErrRet(encryptKey.GetLength() >= (C1_LENGTH + C2_LENGTH), DeviceError::ArgLengthErr);
        
        _sendBuffer.Clear();
        DevCommand::FromAscii("30 03", _sendBuffer);

        size_t keylen = encryptKey.GetLength() - (C1_LENGTH + C2_LENGTH);
        _sendBuffer += _itobyte(keylen);

        DevCommand::FromAscii("10 00", _sendBuffer);
        // 将C1 C2 C3的格式调整为密码键盘需要的C1 C3 C2格式
        // C1
        _sendBuffer.Append(encryptKey.SubArray(0, C1_LENGTH));
        // C3
        _sendBuffer.Append(encryptKey.SubArray(C1_LENGTH + C2_LENGTH));
        _sendBuffer.Append(0x00, C3_LENGTH - keylen);
        // C2
        _sendBuffer.Append(encryptKey.SubArray(C1_LENGTH, C2_LENGTH));

        _sendBuffer[2] = _itobyte(_sendBuffer.GetLength() - 3);
        _sendBuffer.Append(mkID_13.SubArray(0, KeyID_Length));
        _sendBuffer.Append(kcv_8.SubArray(0, KCV_LENGTH));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_sendBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 下载次主密钥密文
     * @param [in] encryptKey 加密的工作密钥数据
     * @param [in] mkID_13 13字节次主密钥ID
     * @param [in] kcv_8 8字节KCV校验数据,完整长度为16字节,只截取前8字节
     */
    bool DownloadMK(const ByteArray& encryptKey, const ByteArray& mkID_13, const ByteArray& kcv_8)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥密文:<";
        _log.WriteStream(encryptKey) << ">\n";
        _log << "次主密钥ID:<";
        _log.WriteStream(mkID_13) << ">\n";
        _log << "密钥KCV:<";
        _log.WriteStream(kcv_8) << ">\n");

        const size_t KCV_LENGTH = 8;

        ASSERT_Device();
        ASSERT_FuncErrRet(mkID_13.GetLength() >= KeyID_Length, DeviceError::ArgLengthErr);
        ASSERT_FuncErrRet(kcv_8.GetLength() >= KCV_LENGTH, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 03", _sendBuffer);

        if(encryptKey.GetLength() == 256)
            _sendBuffer += static_cast<byte>(0x01);
        else
            _sendBuffer += _itobyte(encryptKey.GetLength());

        _sendBuffer += encryptKey;
        _sendBuffer.Append(mkID_13.SubArray(0, KeyID_Length));
        _sendBuffer.Append(kcv_8.SubArray(0, KCV_LENGTH));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 下载次主密钥密文
     * @date 2016-05-13 14:28
     * 
     * @param [in] encryptKey 加密的工作密钥数据
     * @param [in] mkID_13 13字节次主密钥ID
     * @param [out] kcv_8 [default:NULL] 设备返回的KCV
     */
    bool DownloadMK_KCV(const ByteArray& encryptKey, const ByteArray& mkID_13, ByteBuilder* kcv_8 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥密文:<";
        _log.WriteStream(encryptKey) << ">\n";
        _log << "次主密钥ID:<";
        _log.WriteStream(mkID_13) << ">\n");

        const size_t KCV_LENGTH = 8;

        ASSERT_Device();
        ASSERT_FuncErrRet(mkID_13.GetLength() >= KeyID_Length, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 03", _sendBuffer);

        if(encryptKey.GetLength() == 256)
            _sendBuffer += static_cast<byte>(0x01);
        else
            _sendBuffer += _itobyte(encryptKey.GetLength());

        _sendBuffer += encryptKey;
        _sendBuffer.Append(mkID_13.SubArray(0, KeyID_Length));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        if(kcv_8 != NULL)
        {
            (*kcv_8) += _recvBuffer;
        }

        return _logRetValue(true);
    }
    /**
     * @brief 下载工作密钥密文  
     * @param [in] encryptKey 加密的工作密钥数据 
     * @param [in] wkID_13 13字节工作密钥ID
     * @param [in] kcv_4_8 校验数据 
     * - KCV:
     *  - 4字节 DES密钥 
     *  - 8字节 SM4密钥 
     * .
     */
    bool DownloadWK(const ByteArray& encryptKey, const ByteArray& wkID_13, const ByteArray& kcv_4_8)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log << "密钥密文:<";
        _log.WriteStream(encryptKey) << ">\n";
        _log << "工作密钥ID:<";
        _log.WriteStream(wkID_13) << ">\n";
        _log << "密钥KCV:<";
        _log.WriteStream(kcv_4_8) << ">\n");

        ASSERT_Device();
        ASSERT_FuncErrRet(wkID_13.GetLength() >= KeyID_Length, DeviceError::ArgLengthErr);
        ASSERT_FuncErrRet(kcv_4_8.GetLength() == DES_KCV_Length || kcv_4_8.GetLength() == SM4_KCV_Length,
            DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 04", _sendBuffer);
        _sendBuffer += _itobyte(encryptKey.GetLength());
        _sendBuffer += encryptKey;
        _sendBuffer.Append(wkID_13.SubArray(0, KeyID_Length));
        _sendBuffer += kcv_4_8;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 下载工作密钥密文 
     * @param [in] encryptKey 加密的工作密钥数据 
     * @param [in] wkID_13 13字节工作密钥ID
     * @param [out] kcv_8 [default:NULL] 设备返回的校验数据 
     */
    bool DownloadWK_KCV(const ByteArray& encryptKey, const ByteArray& wkID_13, ByteBuilder* kcv_8 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log << "密钥密文:<";
        _log.WriteStream(encryptKey) << ">\n";
        _log << "工作密钥ID:<";
        _log.WriteStream(wkID_13) << ">\n");

        ASSERT_Device();
        ASSERT_FuncErrRet(wkID_13.GetLength() >= KeyID_Length, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 04", _sendBuffer);
        _sendBuffer += _itobyte(encryptKey.GetLength());
        _sendBuffer += encryptKey;
        _sendBuffer.Append(wkID_13.SubArray(0, KeyID_Length));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        LOGGER(_log << "KCV:";
        _log.WriteStream(_recvBuffer) << endl);

        if(kcv_8 != NULL)
        {
            kcv_8->Append(_recvBuffer);
        }

        return _logRetValue(true);
    }
    /**
     * @brief 设置帐号信息
     * @param [out] cardNo_12 帐号信息 
     */
    bool SetCardMessage(const ByteArray& cardNo_12)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "帐号信息:<" << cardNo_12.GetString() << ">\n");

        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 16", _sendBuffer);
        byte len = _itobyte(cardNo_12.GetLength());
        _sendBuffer += len;
        _sendBuffer += cardNo_12.SubArray(0, static_cast<size_t>(len));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 获取密码
     * @param [out] pinblock_16_32 获取到的密码 
     * @param [in] waitTimeoutS [default:255] 等待客户输入时间(s)
     */
    bool GetPwd(ByteBuilder& pinblock_16_32, byte waitTimeoutS = static_cast<byte>(-1))
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "超时时间:<" << static_cast<int>(waitTimeoutS) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 05", _sendBuffer);
        _sendBuffer += waitTimeoutS;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        LOGGER(_log << "PINBLOCK:<";
        _log.WriteStream(_recvBuffer) << ">\n");

        pinblock_16_32 += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 获取密码
     * 
     * @param [in] cardNo_12 帐号信息
     * @param [in] pinblock_16_32 获取到的密码
     * @param [in] waitTimeoutS [default:255] 等待客户输入时间(s)
     */
    bool GetPwd(const ByteArray& cardNo_12, ByteBuilder& pinblock_16_32, byte waitTimeoutS = static_cast<byte>(-1))
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "帐号信息:<" << cardNo_12.GetString() << ">\n");
        LOGGER(_log << "超时时间:<" << static_cast<int>(waitTimeoutS) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 05", _sendBuffer);
        _sendBuffer += waitTimeoutS;
        byte len = _itobyte(cardNo_12.GetLength());
        _sendBuffer += len;
        _sendBuffer += cardNo_12.SubArray(0, static_cast<size_t>(len));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        LOGGER(_log << "PINBLOCK:<";
        _log.WriteStream(_recvBuffer) << ">\n");

        pinblock_16_32 += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 设置次主密钥ID 
     * @param [in] mkID_13 需要设置的13字节次主密钥ID 
     */
    bool SetMK_ID(const ByteArray& mkID_13)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log << "次主密钥ID:<";
        _log.WriteStream(mkID_13) << ">\n");

        ASSERT_Device();
        ASSERT_FuncErrRet(mkID_13.GetLength() >= KeyID_Length, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 06", _sendBuffer);
        _sendBuffer.Append(mkID_13.SubArray(0, KeyID_Length));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 获取次主密钥ID
     * @param [out] mkID_13 获取到的13字节次主密钥ID
     */
    bool GetMK_ID(ByteBuilder& mkID_13)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("30 07", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(mkID_13), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 设置工作密钥ID
     * @param [in] wkID_13 需要设置的13字节工作密钥ID
     */
    bool SetWK_ID(const ByteArray& wkID_13)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log << "工作密钥ID:<";
        _log.WriteStream(wkID_13) << ">\n");

        ASSERT_Device();
        ASSERT_FuncErrRet(wkID_13.GetLength() >= KeyID_Length, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 08", _sendBuffer);
        _sendBuffer.Append(wkID_13.SubArray(0, KeyID_Length));

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 获取工作密钥ID 
     * @param [out] wkID_13 获取到的13字节工作密钥ID 
     */
    bool GetWK_ID(ByteBuilder& wkID_13)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("30 09", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(wkID_13), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 设置加密算法 
     * @param [in] mode 设置的加密算法 
     */
    bool SetAlgorithmMode(AlgorithmMode mode)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "算法:<" << static_cast<int>(mode) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 10", _sendBuffer);
        _sendBuffer += _itobyte(mode);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 设置PINBLOCK后补字节 
     * @param [in] bit 需要补的字节 
     */
    bool SetPinblock(byte bit)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "后补字节:<" << _hex(bit) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 11", _sendBuffer);
        _sendBuffer += bit;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 设置输入的密码长度 
     * @param [in] len 输入的密码长度 [4,12]
     */
    bool SetPinLength(byte len)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "设置输入长度:<" << static_cast<int>(len) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 12", _sendBuffer);
        _sendBuffer += len;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 设置用户输入密码时输入到指定长度是否需要按确认键
     * @param [in] isNeedOK 是否需要按确认键 
     */
    bool SetPwdIsNeedOK(bool isNeedOK)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "是否需要按确认键:<" << isNeedOK << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 13", _sendBuffer);
        _sendBuffer += static_cast<byte>(isNeedOK ? 0x01 : 0x02);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 取消应用
     */
    bool CancelApp()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("30 17", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace pinad 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PSBC_PINPADDEVADAPTER_H_
//========================================================= 
