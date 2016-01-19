//========================================================= 
/**@file CCID_SLE4428.h 
 * @brief CCID的4428卡驱动 
 * 
 * @date 2014-10-21   20:33:21 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
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
/// CCID 4428卡驱动 
class CCID_SLE4428 : 
    public ISLE4442, 
    public CCID_ContactStorageCardDevAdapter
{
public:
    //----------------------------------------------------- 
    enum _CardType
    {
        CardType = CCID_ContactStorageCardDevAdapter::Card_SLE4418_4428_5518_5528
    };
    //----------------------------------------------------- 
    /// 校验密码(2字节密钥)
    virtual bool VerifyKey(const ByteArray& key)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"密钥:<"<<key<<">\n");
        ASSERT_FuncErrRet(key.GetLength() == ISLE4428::KeyLength, DeviceError::ArgLengthErr);
        ASSERT_FuncErrRet(VerifyAssertKey(0, key, true), DeviceError::DevVerifyErr);

        return _logRetValue(true);
    }
    /// 读数据 
    virtual bool ReadMemory(size_t adr, size_t len, ByteBuilder& buff)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        const byte BLOCK_SIZE = 128;

        LOGGER(_log<<"Adr:<"<<adr<<">,Len:<"<<len<<">\n");
        // 每次读取128字节
        int times = static_cast<int>(len / BLOCK_SIZE);
        size_t count = 0;
        size_t lastLen = buff.GetLength();
        bool bRet = true;
        // 先按照整块读取 
        for(int i = 0;i < times; ++i)
        {
            if(!CCID_ContactStorageCardDevAdapter::ReadMemory(static_cast<ushort>(adr + count), BLOCK_SIZE, buff))
                break;
            count += static_cast<size_t>(BLOCK_SIZE);
        }
        // 读剩余的数据 
        bRet = CCID_ContactStorageCardDevAdapter::ReadMemory(static_cast<ushort>(adr + count), len % BLOCK_SIZE, buff);
        if(!bRet)
        {
            buff.RemoveTail(buff.GetLength() - lastLen);
        }

        return _logRetValue(bRet);
    }
    /// 写数据 
    virtual bool WriteMemory(size_t adr, ByteBuilder& buff)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        const byte BLOCK_SIZE = 128;

        LOGGER(
        _log<<"Adr:<"<<adr<<">,Len:<"<<buff.GetLength()<<">\n";
        _log.WriteLine("Data:").WriteLine(buff)<<endl);

        // 每次写入128字节
        int times = static_cast<int>(buff.GetLength() / BLOCK_SIZE);
        size_t count = 0;
        bool bRet = true;
        ByteArray tmp;
        if(buff.GetLength() > BLOCK_SIZE)
        {
            // 先按照整块写入 
            for(int i = 0;i < times; ++i)
            {
                tmp = buff.SubArray(count, BLOCK_SIZE);
                if(!CCID_ContactStorageCardDevAdapter::WriteMemory(static_cast<ushort>(adr + count), tmp))
                    return _logRetValue(false);
                
                count += static_cast<size_t>(BLOCK_SIZE);
            }
        }
        // 写剩余的数据 
        tmp = buff.SubArray(count);
        bRet = CCID_ContactStorageCardDevAdapter::WriteMemory(static_cast<ushort>(adr + count), tmp);
        return _logRetValue(bRet);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
typedef CCID_SLE4428 CCID_SLE4418;
typedef CCID_SLE4428 CCID_SLE5518;
typedef CCID_SLE4428 CCID_SLE5528;
//--------------------------------------------------------- 
} // namespace device
} // namespace extension
} // namespace iccard
} // namespace device
} // namespace zhou_yb
//========================================================= 