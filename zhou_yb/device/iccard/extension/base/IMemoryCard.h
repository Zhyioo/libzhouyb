//========================================================= 
/**@file IMemoryCard.h 
 * @brief 常用IC卡应用层接口定义 
 * 
 * @date 2014-10-15   20:57:41 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IMEMORYCARD_H_
#define _LIBZHOUYB_IMEMORYCARD_H_
//--------------------------------------------------------- 
#include "../../../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace extension {
namespace base {
//--------------------------------------------------------- 
/// 存储卡基本操作接口 
struct IMemoryCard
{
    //----------------------------------------------------- 
    /// 校验卡密码 
    virtual bool VerifyKey(const ByteArray& key) = 0;
    /// 读数据 
    virtual bool ReadMemory(size_t adr, size_t len, ByteBuilder& buff) = 0;
    /// 写数据 
    virtual bool WriteMemory(size_t adr, const ByteArray& buff) = 0;
    //----------------------------------------------------- 
};
/// M1卡基本操作接口 
struct IMifare1Card
{
    //----------------------------------------------------- 
    /// 读取整个扇区 
    static bool ReadSector(IMifare1Card& m1Card, byte sectorIndex, ByteBuilder& data_64)
    {
        for(byte blockIndex = 0;blockIndex < 4; ++blockIndex)
        {
            if(!m1Card.ReadBlock(4 * sectorIndex + blockIndex, data_64))
                return false;
        }
        return true;
    }
    //----------------------------------------------------- 
    /**
     * @brief 认证扇区 
     * @param [in] sectorIndex 需要认证的扇区号
     * @param [in] key_6 块所在扇区6字节密钥
     * @param [in] pinType 密钥类型(A密钥或B密钥)
     *
     * @code
     // 用密钥"6字节FF"认证第4块(1扇区)
     bool bRet = AuthBlock(4, DevCommand::FromAscii("FF FF FF FF FF FF"), 'A');
     * @endcode 
     * 
     */
    virtual bool VerifySector(byte sectorIndex, const ByteArray& key_6, char pinType) = 0;
    /**
     * @brief 读块
     * @param [in] blockIndex 块号
     * @param [out] data_16 读取到的数据(16字节的整块数据)
     */
    virtual bool ReadBlock(byte blockIndex, ByteBuilder& data_16) = 0;
    /**
     * @brief 写块
     * @param [in] blockIndex 块号
     * @param [in] data_16 需要写入的数据(16字节的整块数据)
     */
    virtual bool WriteBlock(byte blockIndex, const ByteArray& data_16) = 0;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 4442卡基本操作接口
struct ISLE4442 : public IMemoryCard
{
    enum Length
    {
        KeyLength = 3
    };
};
//--------------------------------------------------------- 
/// 4428卡基本操作接口 
struct ISLE4428 : public IMemoryCard
{
    enum Length
    {
        KeyLength = 2
    };
};
//--------------------------------------------------------- 
/// AT88SC系列卡片接口 
struct IAT88SC_X
{
    //----------------------------------------------------- 
    /**
     * @brief 更新用户区数据 
     * 
     * 对于AT88SC系列卡片来说,校验擦除密码后,整个应用区数据擦除 
     * 如果只需要写入一段数据,需要进行适配 
     * 
     * @param [in] dev 需要进行适配的设备  
     * @param [in] zone 需要修改的应用区 
     * @param [in] offset 相对于该应用区的偏移量 
     * @param [in] key_2 擦除密码 
     * @param [in] buff 需要写入的数据 
     * @param [in] zone_size 应用区的最大数据长度 
     *
     * @retval DeviceError::ArgRangeErr 参数错误(起始地址+数据长度超过应用区大小)
     * @retval DeviceError::DevVerifyErr 密钥校验错 
     * @retval DeviceError::SendErr 读错 
     * @retval DeviceError::RecvErr 写错 
     * @retval DeviceError::Success 成功 
     */
    static DeviceError::ErrorCode UpdateZone(IAT88SC_X& dev, size_t zone, size_t offset, const ByteArray& key_2, const ByteArray& buff, size_t zone_size)
    {
        // 不需要写入数据 
        if((offset + buff.GetLength()) >= zone_size)
            return DeviceError::ArgRangeErr;

        if(!dev.VerifyEraseKey(zone, key_2))
            return DeviceError::DevVerifyErr;

        ByteBuilder zone_buff(zone_size);
        if(!dev.ReadZone(zone, zone_buff))
            return DeviceError::SendErr;

        memcpy(const_cast<byte*>(zone_buff.GetBuffer(offset)), buff.GetBuffer(), buff.GetLength());
        if(!dev.WriteZone(zone, zone_buff))
            return DeviceError::RecvErr;

        return DeviceError::Success;
    }
    /// 校验用户密码 
    virtual bool VerifyUsrKey(const ByteArray& usrKey_2) = 0;
    /// 读8字节代码保护区 
    virtual bool ReadProtected(ByteBuilder& data_8) = 0;
    /// 写8字节代码保护区 
    virtual bool WriteProtected(const ByteArray& data_8) = 0;
    /// 校验擦除密码 
    virtual bool VerifyEraseKey(size_t zone, const ByteArray& key) = 0;
    /// 读应用区 
    virtual bool ReadZone(size_t zone, ByteBuilder& buff) = 0;
    /// 写应用区 
    virtual bool WriteZone(size_t zone, const ByteArray& buff) = 0;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 102卡接口(实际上只负责提供应用区的地址)
struct IAT88SC102 : public IAT88SC_X
{
    /// 数据地址(后一个地址减去前一个地址就是数据的长度) 
    enum Address
    {
        /// 厂商标识(2字节) 
        AT102_MANUFACTURER_ADR = 0,
        /// 发卡商代码(8字节) 
        AT102_MANUFACTURER_CODE_ADR = 2,
        /// 用户密钥起始地址(2字节) 
        AT102_MAINKEY_ADR = 10,
        /// 密码计数器起始地址(2字节) 
        AT102_KEYCOUNT_ADR = 12,
        /// 代码保护区起始地址(8字节) 
        AT102_PROTECTED_ADR = 14,
        /// 应用一区读写属性(1字节) 
        AT102_ZONE1_PROPERTY_ADR = 22,
        /// 应用一区起始地址(63字节) 
        AT102_ZONE1_ADR = 23,
        /// 应用一区擦除密码(6字节)
        AT102_ZONE1_ERASEKEY_ADR = 86,
        /// 应用二区读写属性(1字节) 
        AT102_ZONE2_PROPERTY_ADR = 92,
        /// 应用二区起始地址(63字节)
        AT102_ZONE2_ADR = 93,
        /// 应用二区擦除密钥(4字节)
        AT102_ZONE2_ERASEKEY_ADR = 156,
        /// 应用二区错误计数器(16字节)
        AT102_ZONE2_KEYCOUNT_ADR = 160,
        /// 测试区(2字节)
        AT102_ZONE_TEST_ADR = 176,

        /// 最大地址 
        AT102_MAX_ADR = 178
    };
    /// 熔丝地址 
    enum FuseAddress
    {
        /// 厂商熔丝 
        AT102_MANUFACTURER_FUSE_ADR = 0x0B6,
        /// EC2EN熔丝 
        AT102_EC2EN_FUSE_ADR = 0x0BF,
        /// 发行熔丝 
        AT102_ISSUE_FUSE_ADR = 0x0C2
    };
    /// 长度 
    enum Length
    {
        /// 用户主密钥长度 
        AT102_USR_KEY_LENGTH = 2,
        /// 应用二区擦除密钥长度 
        AT102_ZONE2_ERASE_KEY_LENGTH = 4,
        /// 应用一区擦除密钥长度  
        AT102_ZONE1_ERASE_KEY_LENGTH = 6,
        /// 代码保护区数据长度 
        AT102_PROTECTED_LENGTH = 8,
        /// 应用区数据长度 
        AT102_ZONE_LENGTH = 64,
    };
};
//--------------------------------------------------------- 
} // namespace base
} // namespace extension
} // namespace iccard
} // namespace device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IMEMORYCARD_H_
//========================================================= 