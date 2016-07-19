//========================================================= 
/**@file H002PinPadDevAdapter.h
 * @brief H002��׼�������������
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
/// ��Կ��Ĵ�С 
#define H002_KEY_BLOCK_SIZE (8)
/// ���ֵĲ�λ���� 
#define H002_EXTEND_BYTE (0x30)
//--------------------------------------------------------- 
/// H002������̲��ֱ�׼ָ�
class H002PinPadDevAdapter :
    public DevAdapterBehavior<IInteractiveTrans>,
    public InterruptBehavior,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// �㷨ģʽ 
    enum AlgorithmMode
    {
        /// δ֪��ģʽ 
        UnknownMode = 0,
        /// DES�㷨 
        DES_Mode,
        /// SM2�㷨 
        SM2_Mode,
        /// SM4�㷨 
        SM4_Mode,
        /// RSA�㷨 
        RSA_Mode
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// ���ͻ����� 
    ByteBuilder _sendBuffer;
    /// ���ջ����� 
    ByteBuilder _recvBuffer;
    /// �ȴ���ȡ��������
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
            // �д�������
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
        ASSERT_FuncErrInfo(bRead, DeviceError::WaitTimeOutErr, "�ȴ�ˢ�ų�ʱ");
        bRead = MagneticDevAdapter::RecvByFormat(_pDev, buff);
        ASSERT_FuncErr(bRead, DeviceError::RecvFormatErr);

        MagneticDevAdapter::MagneticMode mode = MagneticDevAdapter::UnpackRecvCmd(buff, pTr1, pTr2, pTr3);
        if(mode == MagneticDevAdapter::UnKnownMode)
        {
            _logErr(DeviceError::RecvFormatErr, "�������ݸ�ʽ����ȷ");
            return false;
        }
        return true;
    }
    /// ת����Կ�ֽ�
    inline byte _KeyByte(byte data)
    {
        return (data & 0x0F) | H002_EXTEND_BYTE;
    }
    /// ת����Կ����
    inline byte _KeyUse(AlgorithmMode mode)
    {
        return (mode == SM4_Mode) ? 0x31 : 0x30;
    }
    /// ��ȡKCV
    inline ByteArray _KeyKCV(AlgorithmMode mode, const ByteArray& kcv)
    {
        size_t len = (mode == SM4_Mode) ? 8 : 4;
        return kcv.SubArray(0, len);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief ���Ķ�ȡ����
     * @date 2016-07-14 15:38
     * 
     * @param [in] timeoutMs �ȴ�ˢ�ŵĳ�ʱʱ��(ms)
     * @param [out] pTr1 ��ȡ����һ�ŵ���Ϣ
     * @param [out] pTr2 ��ȡ���Ķ��ŵ���Ϣ
     * @param [out] pTr3 ��ȡ�������ŵ���Ϣ
     */
    bool MagRead(uint timeoutMs, ByteBuilder* pTr1, ByteBuilder* pTr2, ByteBuilder* pTr3)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "��ʱʱ��:<" << timeoutMs << "ms>\n");
        ASSERT_Device();
        ASSERT_FuncErrRet(_pDev->Write(DevCommand::FromAscii("1B 5D")), DeviceError::SendErr);
        ASSERT_FuncRet(_WaitMagCard(timeoutMs, pTr1, pTr2, pTr3));
        return _logRetValue(true);
    }
    /**
     * @brief ���Ķ�ȡ����
     * @date 2016-07-14 15:38
     * 
     * @param [in] timeoutMs �ȴ�ˢ�ŵĳ�ʱʱ��(ms)
     * @param [in] mkIndex ���ܵ�����Կ����
     * @param [in] wkIndex ���ܵĹ�����Կ����
     * @param [out] pTr1 ��ȡ����һ�ŵ�����
     * @param [out] pTr2 ��ȡ���Ķ��ŵ�����
     * @param [out] pTr3 ��ȡ�������ŵ�����
     */
    bool MagEncryptRead(uint timeoutMs, byte mkIndex, byte wkIndex, ByteBuilder* pTr1, ByteBuilder* pTr2, ByteBuilder* pTr3)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "��ʱʱ��:<" << timeoutMs << "ms>\n";
        _log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "������Կ����:<" << _hex(wkIndex) << ">\n");
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
        LOGGER(_log << "һ�ŵ�����:<";_log.WriteStream(tr1) << ">\n";
        _log << "���ŵ�����:<";_log.WriteStream(tr2) << ">\n";
        _log << "���ŵ�����:<";_log.WriteStream(tr3) << ">\n");
        return _logRetValue(true);
    }
    /// ����������
    bool Beep()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        return _logRetValue(_pDev->Write("1B 62"));
    }
    /**
     * @brief д�豸���к�
     * @date 2016-07-14 15:45
     * 
     * @param [in] serialNumber ��Ҫд������к�
     */
    bool SetSerialNumber(const ByteArray& serialNumber)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "���к�:<";
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
     * @brief ���豸���к�
     * @date 2016-07-14 15:45
     * 
     * @param [out] serialNumber ��ȡ�����豸���к�
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

        LOGGER(_log << "���к�:";
        _log.WriteStream(_recvBuffer) << endl);
        serialNumber += _recvBuffer;

        return _logRetValue(true);
    }
    /**
     * @brief дӦ�������
     * @date 2016-07-14 17:17
     * 
     * @param [in] id_M32 ��Ҫд���Ӧ�������(���32�ֽ�)
     */
    bool SetAppID(const ByteArray& id_M32)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Ӧ�ñ��:";
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
     * @brief ��ȡӦ�������
     * @date 2016-07-14 17:18
     * 
     * @param [out] id ��ȡ����Ӧ�������
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
        LOGGER(_log << "Ӧ�ñ��:<";
        _log.WriteStream(_recvBuffer) << ">\n");

        id += _recvBuffer;
        return _logRetValue(true);
    }
    /**
     * @brief ��������Կ
     * @date 2016-07-15 10:47
     * 
     * ���ú���ԿֵΪ:0x40 + MkIndex
     */
    /**
     * @brief ��������Կ
     * @date 2016-07-15 10:49
     * 
     * @param [in] mkIndex ��Ҫ���õ���Կ����,���ú���ԿֵΪ:0x40 + MkIndex(Ĭ����������)
     */
    bool ResetKey(byte mkIndex = 'A')
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "��Կ����:<" << _hex(mkIndex) << ">\n");
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
     * @brief ���ĸ�������Կ
     * @date 2016-07-15 11:28
     * 
     * @param [in] keyIndex ��Ҫ���µ�����Կ����
     * @param [in] oldMKey_8_16_24 ��Ҫ���µľ�����Կֵ
     * @param [in] newMKey_8_16_24 ��Ҫ���µ�������Կֵ
     * @param [in] newMKeyMode ������Կ������(DES/SM4)
     */
    bool UpdateMainKey(byte keyIndex, const ByteArray& oldMKey_8_16_24, const ByteArray& newMKey_8_16_24, AlgorithmMode newMKeyMode)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(keyIndex) << ">\n";
        _log << "������Կ:<";_log.WriteStream(oldMKey_8_16_24) << ">\n";
        _log << "������Կ:<";_log.WriteStream(newMKey_8_16_24) << ">\n";
        _log << "������Կ����:<" << newMKeyMode << ">\n");
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
     * @brief ���ĸ�������Կ,����KCV
     * @date 2016-07-15 13:43
     * 
     * @param [in] keyIndex ��Ҫ���µ�����Կ����
     * @param [in] oldMKey_8_16_24 ��Ҫ���µľ�����Կֵ
     * @param [in] newMKey_8_16_24 ��Ҫ���µ�������Կֵ
     * @param [in] newMKeyMode ������Կ������(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] �豸���ص�KCV
     */
    bool UpdateMainKey_KCV(byte keyIndex, const ByteArray& oldMKey_8_16_24, const ByteArray& newMKey_8_16_24, AlgorithmMode newMKeyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(keyIndex) << ">\n";
        _log << "������Կ:<";_log.WriteStream(oldMKey_8_16_24) << ">\n";
        _log << "������Կ:<";_log.WriteStream(newMKey_8_16_24) << ">\n";
        _log << "������Կ����:<" << newMKeyMode << ">\n");
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
     * @brief ������������Կ
     * @date 2016-07-15 14:22
     * 
     * @param [in] keyIndex ��Ҫ���ص�����Կ����
     * @param [in] encryptedMK_8_16_24 ����Կ����
     * @param [in] kcv_4_8 KCV(�ڲ�֧���Զ���ȡ����)
     * @param [in] keyMode ��Կ�㷨��;(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] �豸���ص�KCV
     */
    bool UpdateEncryptedMainKey(byte mkIndex, const ByteArray& encryptedMK_8_16_24, const ByteArray& kcv_4_8, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "��Կ����:<";_log.WriteStream(encryptedMK_8_16_24) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_4_8) << ">\n";
        _log << "����Կ����:<" << keyMode << ">\n");
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
     * @brief ����У�������޸�����Կ,����KCV
     * @date 2016-07-15 14:28
     * 
     * @param [in] mkIndex ��Ҫ���ص�����Կ����
     * @param [in] encryptedMK_8_16_24 ����Կ����
     * @param [in] keyMode ��Կ�㷨��;(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] �豸���ص�KCV
     */
    bool UpdateEncryptedMainKey(byte mkIndex, const ByteArray& encryptedMK_8_16_24, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "��Կ����:<";_log.WriteStream(encryptedMK_8_16_24) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_4_8) << ">\n";
        _log << "����Կ����:<" << keyMode << ">\n");
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
     * @brief �����޸Ĺ�����Կ
     * @date 2016-07-15 15:02
     * 
     * @param [in] mkIndex ���ܵ�����Կ����
     * @param [in] wkIndex ��Ҫ���µĹ�����Կ����
     * @param [in] encryptedWK_8_16_24 ������Կ����
     * @param [in] kcv_4_8 KCV(�ڲ�֧���Զ���ȡ����)
     * @param [in] keyMode ��Կ�㷨��;(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] �豸���ص�KCV
     */
    bool UpdateEncryptedWorkKey(byte mkIndex, byte wkIndex, const ByteArray& encryptedWK_8_16_24, const ByteArray& kcv_4_8, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "������Կ����:<" << _hex(wkIndex) << ">\n";
        _log << "������Կ����:<";_log.WriteStream(encryptedWK_8_16_24) << ">\n";
        _log << "KCV:<";_log.WriteStream(kcv_4_8) << ">\n";
        _log<<"�㷨��;:<"<<keyMode<<">\n");
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
     * @brief �����޸Ĺ�����Կ,����У�鷵��KCV
     * @date 2016-07-15 15:31
     * 
     * @param [in] mkIndex ���ܵ�����Կ����
     * @param [in] wkIndex ��Ҫ���µĹ�����Կ����
     * @param [in] encryptedWK_8_16_24 ������Կ����
     * @param [in] keyMode ��Կ�㷨��;(DES/SM4)
     * @param [out] pKCV_16 [default:NULL] �豸���ص�KCV
     */
    bool UpdateEncryptedWorkKey(byte mkIndex, byte wkIndex, const ByteArray& encryptedWK_8_16_24, AlgorithmMode keyMode, ByteBuilder* pKCV_16 = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "������Կ����:<" << _hex(wkIndex) << ">\n";
        _log << "������Կ����:<";_log.WriteStream(encryptedWK_8_16_24) << ">\n";
        _log << "�㷨��;:<" << keyMode << ">\n");
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
     * @brief ������������
     * @date 2016-07-15 15:45
     * 
     * @param [in] mkIndex ����Կ��
     * @param [in] wkIndex ������Կ��
     * @param [in] pwdlen ��������ĳ���
     * @param [in] isReVoice ����,�Ƿ��ٴ�����
     * @param [in] isEnter �Ƿ���Ҫ��ȷ�ϼ��󷵻�
     * @param [out] pin �������������
     * @param [out] pinLen ʵ����������볤��
     */
    bool InputPassword(byte mkIndex, byte wkIndex, size_t pwdlen, bool isReVoice, bool isEnter, ByteBuilder& pin, size_t pinLen)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "������Կ����:<" << _hex(wkIndex) << ">\n";
        _log << "��������ĳ���:<" << pwdlen << ">\n";
        _log << "�Ƿ�Ϊ�ٴ���������:<" << isReVoice << ">\n";
        _log << "�Ƿ���Ҫȷ�ϼ�����:<" << isEnter << ">\n");
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
        LOGGER(_log << "��������:<";_log.WriteStream(_recvBuffer) << ">\n");
        pinLen = _recvBuffer[0];
        pin += _recvBuffer.SubArray(1);
        return _logRetValue(true);
    }
    /**
     * @brief ������������,��������*��
     * @warning ��GetPassword���ʹ��
     * @date 2016-07-15 16:18
     * 
     * @param [in] pwdlen ������������볤��
     * @param [in] isReVoice ����,�Ƿ��ٴ�����
     * @param [in] isEnter �Ƿ���Ҫ��ȷ�ϼ��󷵻�
     */
    bool WaitPassword(size_t pwdlen, bool isReVoice, bool isEnter)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "��������ĳ���:<" << pwdlen << ">\n";
        _log << "�Ƿ�Ϊ�ٴ���������:<" << isReVoice << ">\n";
        _log << "�Ƿ���Ҫȷ�ϼ�����:<" << isEnter << ">\n");
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
     * @brief ��ȡ����
     * @warning ��Ҫ��WaitPassword���ʹ��
     * @date 2016-07-18 15:09
     * 
     * @param [in] mkIndex ����Կ����
     * @param [in] wkIndex ������Կ����
     * @param [out] pin ��ȡ����������Կ
     */
    bool GetPassword(byte mkIndex, byte wkIndex, ByteBuilder& pin)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "������Կ����:<" << _hex(wkIndex) << ">\n");
        ASSERT_Device();

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromFormat("1B )", _sendBuffer);
        _sendBuffer += _KeyByte(mkIndex);
        _sendBuffer += _KeyByte(wkIndex);
        DevCommand::FromFormat("0D 0A", _sendBuffer);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        LOGGER(_log << "��������:<";_log.WriteStream(_recvBuffer) << ">\n");
        pin += _recvBuffer;

        return _logRetValue(true);
    }
    bool GetPinblock(const ByteArray& accno_12, byte mkIndex, byte wkIndex, size_t pwdlen, bool isReVoice, bool isEnter)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "�ʺ�:<" << _log.WriteStream(accno_12) << ">\n";
        _log << "����Կ����:<" << _hex(mkIndex) << ">\n";
        _log << "������Կ����:<" << _hex(wkIndex) << ">\n";
        _log << "�������볤��:<" << pwdlen << ">\n";
        _log << "�Ƿ���ʾ�ٴ�����:<" << isReVoice << ">\n";
        _log << "�Ƿ���Ҫ��ȷ�ϼ�:<" << isEnter << ">\n");
        ASSERT_Device();
        ASSERT_FuncErrRet(accno_12.GetLength() == 12, DeviceError::ArgLengthErr, "�ʺų��ȴ���");

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