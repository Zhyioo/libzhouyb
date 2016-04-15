//========================================================= 
/**@file CCID_Mifare1Card.h 
 * @brief CCID的M1卡驱动 
 * 
 * @date 2014-10-21   20:33:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_MIFARE1CARD_H_
#define _LIBZHOUYB_CCID_MIFARE1CARD_H_
//--------------------------------------------------------- 
#include "../base/IMemoryCard.h"
using namespace zhou_yb::device::iccard::extension::base;

#include "../../device/CCID_StorageCardDevAdapter.h"
using zhou_yb::device::iccard::device::CCID_Mifare1DevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace extension {
namespace device {
//--------------------------------------------------------- 
/// CCID M1卡驱动 
class CCID_Mifare1Card : 
    public IMifare1Card, 
    public CCID_Mifare1DevAdapter
{
public:
    //----------------------------------------------------- 
    /// 认证扇区
    virtual bool VerifySector(byte sectorIndex, const ByteArray& key_6, char pinType)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"扇区号:<"<<static_cast<uint>(sectorIndex)<<">\n";
        (_log<<"密钥:<").WriteStream(key_6)<<">\n";
        _log<<"密钥类型:<"<<pinType<<">\n");

        byte keyNumber = 0;
        // keyStructure = 0x00,0x20在不同版本的设备之间保持兼容
        ASSERT_FuncErrRet(
            DownloadAuthKey(key_6, keyNumber, 0x20) || DownloadAuthKey(key_6, keyNumber, 0x00), 
            DeviceError::OperatorErr);
        pinType = _get_upper(pinType) == 'B' ? 0x61 : 0x60;
        ASSERT_FuncErrRet(AuthBlock(4*sectorIndex, pinType, keyNumber), DeviceError::DevVerifyErr);

        return _logRetValue(true);
    }
    /// 读块
    virtual bool ReadBlock(byte blockIndex, ByteBuilder& data_16)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ASSERT_FuncErrRet(CCID_Mifare1DevAdapter::ReadBlock(blockIndex, data_16, 0x10), DeviceError::RecvErr);
        return _logRetValue(true);
    }
    /// 写块
    virtual bool WriteBlock(byte blockIndex, const ByteArray& data_16)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ASSERT_FuncErrRet(CCID_Mifare1DevAdapter::WriteBlock(blockIndex, data_16), DeviceError::SendErr);

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace device
} // namespace extension
} // namespace iccard
} // namespace device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_MIFARE1CARD_H_
//========================================================= 