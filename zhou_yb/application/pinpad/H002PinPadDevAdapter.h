//========================================================= 
/**@file H002PinPadDevAdapter.h
 * @brief H002标准版密码键盘驱动
 * 
 * @date 2016-07-13   16:44:54
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_H002PINPADDEVADAPTER_H_
#define _LIBZHOUYB_H002PINPADDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../device/magnetic/MagneticDevAdapter.h"
using zhou_yb::device::magnetic::MagneticDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace pinpad {
//--------------------------------------------------------- 
/// 密钥块的大小 
#define H002_KEY_BLOCK_SIZE (8)
/// 拆字的补位数据 
#define H002_EXTEND_BYTE (0x30)
//--------------------------------------------------------- 
/// H002密码键盘部分标准指令集
class H002PinPadDevAdapter :
    public DevAdapterBehavior<IInteractiveTrans>,
    public InterruptBehavior,
    public RefObject
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
        /// SM2算法 
        SM2_Mode,
        /// SM4算法 
        SM4_Mode,
        /// RSA算法 
        RSA_Mode
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区 
    ByteBuilder _recvBuffer;
    /// 等待读取磁条数据
    bool _WaitMagCard(uint timeoutMs, ByteBuilder* pTr1, ByteBuilder* pTr2, ByteBuilder* pTr3)
    {
        Timer timer;
        byte magCmd[2] = { 0x1B, 'j' };
        ByteBuilder buff(256);
        bool bRead = false;
        while(timer.Elapsed() < timeoutMs)
        {
            if(InterruptBehavior::Implement(*this))
            {
                _logErr(DeviceError::OperatorInterruptErr);
                return false;
            }

            if(!_pDev->Write(ByteArray(magCmd, 2)))
            {
                _logErr(DeviceError::SendErr);
                return false;
            }
            buff.Clear();
            while(buff.GetLength() < 2)
            {
                if(!_pDev->Read(buff))
                {
                    _logErr(DeviceError::RecvErr);
                    return false;
                }
            }
            // 有磁条数据
            if(buff[0] == 0x1B)
            {
                if(buff[1] == 0x31)
                {
                    buff.RemoveFront(2);
                    bRead = true;
                    break;
                }
                else if(buff[1] == 0x73)
                {
                    bRead = true;
                    break;
                }
            }
        }
        ASSERT_FuncErrInfo(bRead, DeviceError::WaitTimeOutErr, "等待刷磁超时");
        bRead = MagneticDevAdapter::RecvByFormat(_pDev, buff);
        ASSERT_FuncErr(bRead, DeviceError::RecvFormatErr);

        MagneticDevAdapter::MagneticMode mode = MagneticDevAdapter::UnpackRecvCmd(buff, pTr1, pTr2, pTr3);
        if(mode == MagneticDevAdapter::UnKnownMode)
        {
            _logErr(DeviceError::RecvFormatErr, "磁条数据格式不正确");
            return false;
        }
        return true;
    }
    /// 转换密钥字节
    inline byte _KeyByte(byte data)
    {
        return (data & 0x0F) | H002_EXTEND_BYTE;
    }
    /// 转换密钥类型
    inline byte _KeyUse(AlgorithmMode mode)
    {
        return (mode == SM4_Mode) ? 0x31 : 0x30;
    }
    /// 截取KCV
    inline ByteArray _KeyKCV(AlgorithmMode mode, const ByteArray& kcv)
    {
        size_t len = (mode == SM4_Mode) ? 8 : 4;
        return kcv.SubArray(0, len);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 明文读取磁条
     * @date 2016-07-14 15:38
     * 
     * @param [in] timeoutMs 等待刷磁的超时时间(ms)
     * @param [out] pTr1 获取到的一磁道信息
     * @param [out] pTr2 获取到的二磁道信息
     * @param [out] pTr3 获取到的三磁道信息
     */
    bool MagRead(uint timeoutMs, ByteBuilder* pTr1, ByteBuilder* pTr2, ByteBuilder* pTr3)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "超时时间:<" << timeoutMs << "ms>\n");
        ASSERT_Device();
        ASSERT_FuncErrRet(_pDev->Write(DevCommand::FromAscii("1B 5D")), DeviceError::SendErr);
        ASSERT_FuncRet(_WaitMagCard(timeoutMs, pTr1, pTr2, pTr3));
        return _logRetValue(true);
    }
    /**
     * @brief 密文读取磁条
     * @date 2016-07-14 15:38
     * 
     * @param [in] timeoutMs 等待刷磁的超时时间(ms)
     * @param [in] mkIndex 加密的主密钥索引
     * @param [in] wkIndex 加密的工作密钥索引
     * @param [out] pTr1 获取到的一磁道数据
     * @param [out] pTr2 获取到的二磁道数据
     * @param [out] pTr3 获取到的三磁道数据
     */
    bool MagEncryptRead(uint timeoutMs, byte mkIndex, byte wkIndex, ByteBuilder* pTr1, ByteBuilder* pTr2, ByteBuilder* pTr3)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "超时时间:<" << timeoutMs << "ms>\n";
        _log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "工作密钥索引:<" << _hex(wkIndex) << ">\n");
        ASSERT_Device();
        ASSERT_FuncErrRet(_pDev->Write(DevCommand::FromAscii("1B 5D")), DeviceError::SendErr);

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("1B 5C", _sendBuffer);
        _sendBuffer += mkIndex;
        _sendBuffer += wkIndex;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ByteBuilder tr1(128);
        ByteBuilder tr2(128);
        ByteBuilder tr3(128);
        ASSERT_FuncRet(_WaitMagCard(timeoutMs, &tr1, &tr2, &tr3));
        if(pTr1 != NULL)
            ByteConvert::Fold(tr1, *pTr1);
        if(pTr2 != NULL)
            ByteConvert::Fold(tr2, *pTr2);
        if(pTr3 != NULL)
            ByteConvert::Fold(tr3, *pTr3);
        LOGGER(_log << "一磁道数据:<";_log.WriteStream(tr1) << ">\n";
        _log << "二磁道数据:<";_log.WriteStream(tr2) << ">\n";
        _log << "三磁道数据:<";_log.WriteStream(tr3) << ">\n");
        return _logRetValue(true);
    }
    /// 蜂鸣器控制
    bool Beep()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        return _logRetValue(_pDev->Write("1B 62"));
    }
    /**
     * @brief 写设备序列号
     * @date 2016-07-14 15:45
     * 
     * @param [in] serialNumber 需要写入的序列号
     */
    bool SetSerialNumber(const ByteArray& serialNumber)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "序列号:<";
        _log.WriteStream(serialNumber) << ">\n");
        ASSERT_Device();
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B W SN", _sendBuffer);
        _sendBuffer += serialNumber;
        DevCommand::FromAscii("0D 0A", _sendBuffer);
        ASSERT_FuncErr(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErr(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        return _logRetValue(true);
    }
    /**
     * @brief 读设备序列号
     * @date 2016-07-14 15:45
     * 
     * @param [out] serialNumber 获取到的设备序列号
     */
    bool GetSerialNumber(ByteBuilder& serialNumber)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B W 0D 0A", _sendBuffer);
        ASSERT_FuncErr(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErr(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        LOGGER(_log << "序列号:";
        _log.WriteStream(_recvBuffer) << endl);
        serialNumber += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief 写应用区编号
     * @date 2016-07-14 17:17
     * 
     * @param [in] id_M32 需要写入的应用区编号(最大32字节)
     */
    bool SetAppID(const ByteArray& id_M32)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "应用编号:";
        _log.WriteStream(id_M32) << endl);
        ASSERT_Device();
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B s w", _sendBuffer);
        ByteConvert::Expand(id_M32.SubArray(0, 32), _sendBuffer, ExByte);
        DevCommand::FromAscii("0D 0A", _sendBuffer);

        ASSERT_FuncErr(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErr(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        return _logRetValue(true);
    }
    /**
     * @brief 获取应用区编号
     * @date 2016-07-14 17:18
     * 
     * @param [out] id 获取到的应用区编号
     */
    bool GetAppID(ByteBuilder& id)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B s r 0D 0A", _sendBuffer);
        ASSERT_FuncErr(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErr(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "应用编号:<";
        _log.WriteStream(_recvBuffer) << ">\n");

        id += _recvBuffer;
        return _logRetValue(true);
    }
    /**
     * @brief 重置主密钥
     * @date 2016-07-15 10:47
     * 
     * 重置后密钥值为:0x40 + MkIndex
     */
    /**
     * @brief 重置主密钥
     * @date 2016-07-15 10:49
     * 
     * @param [in] mkIndex 需要重置的密钥索引,重置后密钥值为:0x40 + MkIndex(默认重置所有)
     */
    bool ResetKey(byte mkIndex = 'A')
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << _hex(mkIndex) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B 64", _sendBuffer);
        if(mkIndex == 'A')
        {
            _sendBuffer += mkIndex;
        }
        else
        {
            _sendBuffer += _KeyByte(mkIndex);
        }
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        return _logRetValue(true);
    }
    /**
     * @brief 明文更新主密钥
     * @date 2016-07-15 11:28
     * 
     * @param [in] keyIndex 需要更新的主密钥索引
     * @param [in] oldMKey_8_16_24 需要更新的旧主密钥值
     * @param [in] newMKey_8_16_24 需要更新的新主密钥值
     * @param [in] newMKeyMode 新主密钥的类型(DES/SM4)
     */
    bool UpdateMainKey(byte keyIndex, const ByteArray& oldMKey_8_16_24, const ByteArray& newMKey_8_16_24, AlgorithmMode newMKeyMode)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(keyIndex) << ">\n";
        _log << "旧主密钥:<";_log.WriteStream(oldMKey_8_16_24) << ">\n";
        _log << "新主密钥:<";_log.WriteStream(newMKey_8_16_24) << ">\n";
        _log << "新主密钥类型:<" << newMKeyMode << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B kM", _sendBuffer);
        _sendBuffer += _KeyByte(keyIndex);
        _sendBuffer += _KeyByte(oldMKey_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(oldMKey_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyByte(newMKey_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(newMKey_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyUse(newMKeyMode);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    /**
     * @brief 明文更新主密钥,返回KCV
     * @date 2016-07-15 13:43
     * 
     * @param [in] keyIndex 需要更新的主密钥索引
     * @param [in] oldMKey_8_16_24 需要更新的旧主密钥值
     * @param [in] newMKey_8_16_24 需要更新的新主密钥值
     * @param [in] newMKeyMode 新主密钥的类型(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] 设备返回的KCV
     */
    bool UpdateMainKey_KCV(byte keyIndex, const ByteArray& oldMKey_8_16_24, const ByteArray& newMKey_8_16_24, AlgorithmMode newMKeyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(keyIndex) << ">\n";
        _log << "旧主密钥:<";_log.WriteStream(oldMKey_8_16_24) << ">\n";
        _log << "新主密钥:<";_log.WriteStream(newMKey_8_16_24) << ">\n";
        _log << "新主密钥类型:<" << newMKeyMode << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B kN", _sendBuffer);
        _sendBuffer += _KeyByte(keyIndex);
        _sendBuffer += _KeyByte(oldMKey_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(oldMKey_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyByte(newMKey_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(newMKey_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyUse(newMKeyMode);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "KCV:<";_log.WriteStream(_recvBuffer) << ">\n");
        if(pKCV_16 != NULL)
        {
            pKCV_16->Append(_recvBuffer);
        }
        return _logRetValue(true);
    }
    /**
     * @brief 密文下载主密钥
     * @date 2016-07-15 14:22
     * 
     * @param [in] keyIndex 需要下载的主密钥索引
     * @param [in] encryptedMK_8_16_24 主密钥密文
     * @param [in] kcv_4_8 KCV(内部支持自动截取长度)
     * @param [in] keyMode 密钥算法用途(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] 设备返回的KCV
     */
    bool UpdateEncryptedMainKey(byte mkIndex, const ByteArray& encryptedMK_8_16_24, const ByteArray& kcv_4_8, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "密钥密文:<";_log.WriteStream(encryptedMK_8_16_24) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_4_8) << ">\n";
        _log << "主密钥类型:<" << keyMode << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();
        if(pKCV_16 == NULL)
        {
            DevCommand::FromFormat("1B km", _sendBuffer);
        }
        else
        {
            DevCommand::FromFormat("1B kn", _sendBuffer);
        }
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(encryptedMK_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(encryptedMK_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        ByteConvert::Expand(_KeyKCV(keyMode, kcv_4_8), _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyUse(keyMode);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        if(pKCV_16 != NULL)
        {
            LOGGER(_log << "KCV:<";_log.WriteStream(_recvBuffer) << ">\n");
            pKCV_16->Append(_recvBuffer);
        }
        return _logRetValue(true);
    }
    /**
     * @brief 不带校验密文修改主密钥,返回KCV
     * @date 2016-07-15 14:28
     * 
     * @param [in] mkIndex 需要下载的主密钥索引
     * @param [in] encryptedMK_8_16_24 主密钥密文
     * @param [in] keyMode 密钥算法用途(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] 设备返回的KCV
     */
    bool UpdateEncryptedMainKey(byte mkIndex, const ByteArray& encryptedMK_8_16_24, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "密钥密文:<";_log.WriteStream(encryptedMK_8_16_24) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_4_8) << ">\n";
        _log << "主密钥类型:<" << keyMode << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B ko", _sendBuffer);
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(encryptedMK_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(encryptedMK_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyUse(keyMode);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "KCV:<";_log.WriteStream(_recvBuffer) << ">\n");
        if(pKCV_16 != NULL)
        {
            pKCV_16->Append(_recvBuffer);
        }
        return _logRetValue(true);
    }
    /**
     * @brief 密文修改工作密钥
     * @date 2016-07-15 15:02
     * 
     * @param [in] mkIndex 加密的主密钥索引
     * @param [in] wkIndex 需要更新的工作密钥索引
     * @param [in] encryptedWK_8_16_24 工作密钥密文
     * @param [in] kcv_4_8 KCV(内部支持自动截取长度)
     * @param [in] keyMode 密钥算法用途(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] 设备返回的KCV
     */
    bool UpdateEncryptedWorkKey(byte mkIndex, byte wkIndex, const ByteArray& encryptedWK_8_16_24, const ByteArray& kcv_4_8, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "工作密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "工作密钥密文:<";_log.WriteStream(encryptedWK_8_16_24) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_4_8) << ">\n";
        _log<<"算法用途:<"<<keyMode<<">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        if(pKCV_16 == NULL)
        {
            DevCommand::FromFormat("1B ku", _sendBuffer);
        }
        else
        {
            DevCommand::FromFormat("1B kv", _sendBuffer);
        }
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(wkIndex);
        _sendBuffer += _KeyByte(encryptedWK_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(encryptedWK_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        ByteConvert::Expand(_KeyKCV(keyMode, kcv_4_8), _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyByte(keyMode);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        if(pKCV_16 != NULL)
        {
            LOGGER(_log << "KCV:<";_log.WriteStream(_recvBuffer) << ">\n");
            pKCV_16->Append(_recvBuffer);
        }
        return _logRetValue(true);
    }
    /**
     * @brief 密文修改工作密钥,不带校验返回KCV
     * @date 2016-07-15 15:31
     * 
     * @param [in] mkIndex 加密的主密钥索引
     * @param [in] wkIndex 需要更新的工作密钥索引
     * @param [in] encryptedWK_8_16_24 工作密钥密文
     * @param [in] keyMode 密钥算法用途(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] 设备返回的KCV
     */
    bool UpdateEncryptedWorkKey(byte mkIndex, byte wkIndex, const ByteArray& encryptedWK_8_16_24, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "工作密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "工作密钥密文:<";_log.WriteStream(encryptedWK_8_16_24) << ">\n";
        _log << "算法用途:<" << keyMode << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B kw", _sendBuffer);
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(wkIndex);
        _sendBuffer += _KeyByte(encryptedWK_8_16_24.GetLength() / H002_KEY_BLOCK_SIZE);
        ByteConvert::Expand(encryptedWK_8_16_24, _sendBuffer, H002_EXTEND_BYTE);
        _sendBuffer += _KeyUse(keyMode);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        if(pKCV_16 != NULL)
        {
            LOGGER(_log << "KCV:<";_log.WriteStream(_recvBuffer) << ">\n");
            pKCV_16->Append(_recvBuffer);
        }
        return _logRetValue(true);
    }
    /**
     * @brief 密文输入密码
     * @date 2016-07-15 15:45
     * 
     * @param [in] mkIndex 主密钥号
     * @param [in] wkIndex 工作密钥号
     * @param [in] pwdlen 期望输入的长度
     * @param [in] isReVoice 语音,是否再次输入
     * @param [in] isEnter 是否需要按确认键后返回
     * @param [out] pin 输入的密文密码
     * @param [out] pinLen 实际输入的密码长度
     */
    bool InputPassword(byte mkIndex, byte wkIndex, size_t pwdlen, bool isReVoice, bool isEnter, ByteBuilder& pin, size_t pinLen)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "工作密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "期望输入的长度:<" << pwdlen << ">\n";
        _log << "是否为再次输入语音:<" << isReVoice << ">\n";
        _log << "是否需要确认键返回:<" << isEnter << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B ip", _sendBuffer);
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(wkIndex);
        _sendBuffer += isReVoice ? 0x31 : 0x30;
        _sendBuffer += isEnter ? 0x31 : 0x30;
        _sendBuffer += _KeyByte(pwdlen);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() > 1, DeviceError::RecvFormatErr);
        LOGGER(_log << "密文密码:<";_log.WriteStream(_recvBuffer) << ">\n");
        pinLen = _recvBuffer[0];
        pin += _recvBuffer.SubArray(1);
        return _logRetValue(true);
    }
    /**
     * @brief 加密输入密码,按键返回*号
     * @warning 与GetPassword配对使用
     * @date 2016-07-15 16:18
     * 
     * @param [in] pwdlen 期望输入的密码长度
     * @param [in] isReVoice 语音,是否再次输入
     * @param [in] isEnter 是否需要按确认键后返回
     */
    bool WaitPassword(size_t pwdlen, bool isReVoice, bool isEnter)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "期望输入的长度:<" << pwdlen << ">\n";
        _log << "是否为再次输入语音:<" << isReVoice << ">\n";
        _log << "是否需要确认键返回:<" << isEnter << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B iq", _sendBuffer);
        _sendBuffer += isReVoice ? 0x31 : 0x30;
        _sendBuffer += _KeyByte(pwdlen);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        return _logRetValue(true);
    }
    /**
     * @brief 获取密文
     * @warning 需要在WaitPassword配对使用
     * @date 2016-07-18 15:09
     * 
     * @param [in] mkIndex 主密钥索引
     * @param [in] wkIndex 工作密钥索引
     * @param [out] pin 获取到的密文密钥
     */
    bool GetPassword(byte mkIndex, byte wkIndex, ByteBuilder& pin)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "工作密钥索引:<" << _hex(wkIndex) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B )", _sendBuffer);
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(wkIndex);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "密文密码:<";_log.WriteStream(_recvBuffer) << ">\n");
        pin += _recvBuffer;

        return _logRetValue(true);
    }
    bool GetPinblock(const ByteArray& accno_12, byte mkIndex, byte wkIndex, size_t pwdlen, bool isReVoice, bool isEnter)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "帐号:<" << _log.WriteStream(accno_12) << ">\n";
        _log << "主密钥索引:<" << _hex(mkIndex) << ">\n";
        _log << "工作密钥索引:<" << _hex(wkIndex) << ">\n";
        _log << "期望密码长度:<" << pwdlen << ">\n";
        _log << "是否提示再次输入:<" << isReVoice << ">\n";
        _log << "是否需要按确认键:<" << isEnter << ">\n");
        ASSERT_Device();
        ASSERT_FuncErrRet(accno_12.GetLength() == 12, DeviceError::ArgLengthErr, "帐号长度错误");

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B ic", _sendBuffer);
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(wkIndex);
        _sendBuffer += _KeyByte(pwdlen);
        _sendBuffer += accno_12;


        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace pinpad
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_H002PINPADDEVADAPTER_H_
//========================================================= 