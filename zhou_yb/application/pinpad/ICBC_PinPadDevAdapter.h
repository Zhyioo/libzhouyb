//========================================================= 
/**@file ICBC_PinPadDevAdapter.h 
 * @brief 工行国密键盘指令集封装 
 * 
 * @date 2015-01-26   20:32:23
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICBC_PINPADDEVADAPTER_H_
#define _LIBZHOUYB_ICBC_PINPADDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../device/cmd_adapter/PinDevCmdAdapter.h"
using zhou_yb::device::cmd_adapter::PinDevCmdAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace pinpad {
//--------------------------------------------------------- 
/// 工行国密键盘指令集 
class ICBC_PinPadDevAdapter : public DevAdapterBehavior<PinDevCmdAdapter>
{
protected:
    //----------------------------------------------------- 
    ByteBuilder _sendBuff;
    ByteBuilder _recvBuff;
    //----------------------------------------------------- 
    /**
     * @brief 获取导出密钥的模式字节标志位 
     * @param [in] keyIndex 密钥索引 [1-5]
     * @param [in] isExportExist 是否导出现有密钥 
     * @param [in] isExportPrivateK 是否导出私钥 
     * @param [in] isExportPublicK 是否导出公钥 
     */
    byte _getKeyMode(byte keyIndex, bool isExportExist, bool isExportPrivateK, bool isExportPublicK)
    {
        byte flag = 0x00;
        // 设置密钥索引 
        if(keyIndex < 1 || keyIndex > 5)
            keyIndex = 0x0F;
        
        flag = (keyIndex << 4) & 0x0F0;
        BitConvert::Set(flag, 2, static_cast<uint>(isExportExist));
        BitConvert::Set(flag, 1, static_cast<uint>(isExportPrivateK));
        BitConvert::Set(flag, 0, static_cast<uint>(isExportPublicK));

        return flag;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief SM2部分 
     */ 
    /**
     * @brief 生成SM2密钥 
     * @param [in] keyIndex 密钥索引 [1-5,0x0F表示设备不保存密钥]
     * @param [out] publicKey 生成的公钥(为NULL表示不导出该密钥)
     * @param [out] privateKey [default:NULL] 生成的私钥(为NULL表示不导出该密钥) 
     * @param [in] isExist 是否导出设备中存在的密钥 
     */
    bool GenerateKEY_SM2(byte keyIndex, ByteBuilder* publicKey, ByteBuilder* privateKey = NULL, bool isExist = false)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "密钥索引:<" << static_cast<int>(keyIndex) << ">\n");

        ASSERT_Device();

        // SM2公钥长度 
        const size_t SM2_PUBLIC_KEY_LEN = 65;
        // SM2私钥长度 
        const size_t SM2_PRIVATE_KEY_LEN = 32;

        // 输出密钥都为空,则认为不需要导出密钥 
        if(publicKey == NULL && privateKey == NULL)
            return _logRetValue(true);

        byte P1 = _getKeyMode(keyIndex, isExist, privateKey != NULL, publicKey != NULL);

        _sendBuff.Clear();
        DevCommand::FromAscii("1B 53 31 00 00 0D", _sendBuff);
        _sendBuff[3] = P1;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        _pDev->IsAllowFold = true;
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        if(publicKey != NULL)
        {
            StringConvert::Left(_recvBuff, SM2_PUBLIC_KEY_LEN, *publicKey);
            LOGGER(
            _log << "公钥:<";
            _log.WriteStream(*publicKey) << ">\n");
        }
        if(privateKey != NULL)
        {
            StringConvert::Right(_recvBuff, SM2_PRIVATE_KEY_LEN, *privateKey);
            LOGGER(
            _log << "私钥:<";
            _log.WriteStream(*privateKey) << ">\n");
        }

        return _logRetValue(true);
    }
    /**
     * @brief 导入SM2密钥 
     * @param [in] keyIndex 导入的密钥索引 [1,5]
     * @param [in] sm2Key 导入的SM2密钥 (公钥+私钥)
     */
    bool DownloadKEY_SM2(byte keyIndex, const ByteArray& sm2Key)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log << "密钥索引:<" << static_cast<int>(keyIndex) << ">\n";
        _log << "密钥:<";
        _log.WriteStream(sm2Key) << ">\n");

        ASSERT_Device();

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("1B 53 32", _sendBuff);
        _sendBuff += keyIndex;
        _sendBuff += sm2Key;
        _sendBuff += static_cast<byte>(0x0D);

        ASSERT_FuncErrRet(_pDev->Write(_sendBuff), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuff), DeviceError::RecvErr);

        return _logRetValue(true);
    }
    //@}
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace pinpad
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICBC_PINPADDEVADAPTER_H_
//========================================================= 