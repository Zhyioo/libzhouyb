//========================================================= 
/**@file CCID_AT88SC102.h 
 * @brief CCID指令集的102卡驱动 
 * 
 * @date 2014-10-21   20:32:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_AT88SC102_H_
#define _LIBZHOUYB_CCID_AT88SC102_H_
//--------------------------------------------------------- 
#include "../base/IMemoryCard.h"
using namespace zhou_yb::device::iccard::extension::base;

#include "../../device/CCID_StorageCardDevAdapter.h"
using zhou_yb::device::iccard::device::CCID_ContactStorageCardDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace extension {
namespace device {
//--------------------------------------------------------- 
/// CCID设备102卡驱动 
class CCID_AT88SC102 : 
    public IMemoryCard, 
    public IAT88SC102, 
    public CCID_ContactStorageCardDevAdapter
{
public:
    //----------------------------------------------------- 
    enum _CardType
    {
        /// 卡片类型标识 
        CardType = CCID_ContactStorageCardDevAdapter::Card_AT88SC101_102_1003
    };
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief IMemoryCard成员函数 
     */ 
    /// 校验密码(用户密码) 
    virtual bool VerifyKey(const ByteArray& key)
    {
        LOG_FUNC_NAME();
        return _logRetValue(VerifyUsrKey(key));
    }
    /// 读数据 
    virtual bool ReadMemory(size_t adr, size_t len, ByteBuilder& buff)
    {
        return CCID_ContactStorageCardDevAdapter::ReadMemory(static_cast<ushort>(adr), static_cast<byte>(len), buff);
    }
    /// 写数据 
    virtual bool WriteMemory(size_t adr, const ByteArray& buff)
    {
        return CCID_ContactStorageCardDevAdapter::WriteMemory(static_cast<ushort>(adr), buff);
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief IAT88SC102 数据成员 
     */ 
    /// 校验用户密码 
    virtual bool VerifyUsrKey(const ByteArray& key_2)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Key:<"<<key_2<<">\n");

        ASSERT_FuncErrRet(key_2.GetLength() == IAT88SC102::AT102_USR_KEY_LENGTH, DeviceError::ArgLengthErr);
        ASSERT_FuncErrRet(VerifyAssertKey(0x080A, key_2, true), DeviceError::DevVerifyErr);

        return _logRetValue(true);
    }
    /// 读8字节代码保护区  
    virtual bool ReadProtected(ByteBuilder& data_8)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder tmpBuff(8);
        ASSERT_FuncErrRet(ReadMemory(IAT88SC102::AT102_PROTECTED_ADR, IAT88SC102::AT102_PROTECTED_LENGTH, tmpBuff), DeviceError::OperatorErr);

        data_8 += tmpBuff;
        LOGGER(_log.WriteLine("代码保护区:").WriteLine(tmpBuff));

        return _logRetValue(true);
    }
    /// 写8字节代码保护区 
    virtual bool WriteProtected(const ByteArray& data_8)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("写入数据:").WriteLine(data_8));
        ASSERT_FuncErrRet(data_8.GetLength() == IAT88SC102::AT102_PROTECTED_LENGTH, DeviceError::ArgRangeErr);
        ASSERT_FuncErrRet(EraseMemory(IAT88SC102::AT102_PROTECTED_ADR, IAT88SC102::AT102_PROTECTED_LENGTH), DeviceError::OperatorStatusErr);
        ASSERT_FuncErrRet(WriteMemory(IAT88SC102::AT102_PROTECTED_ADR, data_8), DeviceError::OperatorErr);

        return _logRetValue(true);
    }
    /// 校验擦除密码
    virtual bool VerifyEraseKey(uint zone, const ByteArray& key)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"Zone:<"<<zone<<">\n"
            <<"Key:<"<<key<<">\n");

        bool bRet = false;
        switch(zone)
        {
        case CCID_ContactStorageCardDevAdapter::Zone1:
            ASSERT_FuncErrInfoRet(key.GetLength() == IAT88SC102::AT102_ZONE1_ERASE_KEY_LENGTH, DeviceError::ArgRangeErr, "应用一区擦除密钥不为6字节");
            bRet = VerifyAssertKey(IAT88SC102::AT102_ZONE1_ERASEKEY_ADR, key, true);
            break;
        case  CCID_ContactStorageCardDevAdapter::Zone2:
            ASSERT_FuncErrInfoRet(key.GetLength() == IAT88SC102::AT102_ZONE2_ERASE_KEY_LENGTH, DeviceError::ArgRangeErr, "应用二区擦除密钥不为4字节");
            bRet = VerifyAssertKey(IAT88SC102::AT102_ZONE2_ERASEKEY_ADR, key, true);
            break;
        default:
            _logErr(DeviceError::ArgRangeErr, "应用区参数范围错误");
            break;
        }

        return _logRetValue(bRet);
    }
    /// 读应用区(读出的数据为64字节)
    virtual bool ReadZone(uint zone, ByteBuilder& buff)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Zone:<"<<zone<<">\n");
        bool bRet = false;
        LOGGER(size_t len = buff.GetLength());
        switch(zone)
        {
        case  CCID_ContactStorageCardDevAdapter::Zone1:
            bRet = ReadMemory(IAT88SC102::AT102_ZONE1_PROPERTY_ADR, IAT88SC102::AT102_ZONE_LENGTH, buff);
            break;
        case  CCID_ContactStorageCardDevAdapter::Zone2:
            bRet = ReadMemory(IAT88SC102::AT102_ZONE2_PROPERTY_ADR, IAT88SC102::AT102_ZONE_LENGTH, buff);
            break;
        default:
            _logErr(DeviceError::ArgRangeErr, "应用区参数范围错误");
            break;
        }

        LOGGER(
        _log.WriteLine("读到数据:");
        _log.WriteHex(buff.SubArray(len))<<endl);

        return _logRetValue(bRet);
    }
    /**
     * @brief 写应用区
     * @warning buff 数据大小为64字节,第一字节包括应用去读写属性,需要注意写入的值是否正确 
     */ 
    virtual bool WriteZone(uint zone, const ByteArray& buff)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"Zone:<"<<zone<<">\n";
        _log.WriteLine("写入数据:");
        _log.WriteLine(buff));

        ASSERT_FuncErrRet(buff.GetLength() == IAT88SC102::AT102_ZONE_LENGTH, DeviceError::ArgLengthErr);

        bool bRet = false;
        ushort adr = 0;
        switch(zone)
        {
        case CCID_ContactStorageCardDevAdapter::Zone1:
            adr = IAT88SC102::AT102_ZONE1_PROPERTY_ADR;
            break;
        case CCID_ContactStorageCardDevAdapter::Zone2:
            adr = IAT88SC102::AT102_ZONE2_PROPERTY_ADR;
            break;
        default:
            _logErr(DeviceError::ArgRangeErr, "应用区参数范围错误");
            break;
        }

        ASSERT_FuncErrRet(adr != 0, DeviceError::ArgFormatErr);

        bRet = EraseMemory(adr, IAT88SC102::AT102_ZONE_LENGTH);
        if(bRet)
        {
            bRet = WriteMemory(adr, buff);
            if(!bRet)
            {
                _logErr(DeviceError::OperatorErr, "写入数据失败");
            }
        }
        else
        {
            _logErr(DeviceError::OperatorStatusErr, "擦除数据失败");
        }

        return _logRetValue(bRet);
    }
    //@}
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace device
} // namespace extension
} // namespace iccard
} // namespace device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_AT88SC102_H_
//========================================================= 