//========================================================= 
/**@file Key_Provider.h 
 * @brief 密钥相关算法操作
 * 
 * @date 2014-3-18   15:26:07 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_KEY_PROVIDER_H_
#define _LIBZHOUYB_KEY_PROVIDER_H_
//--------------------------------------------------------- 
#include "DES_Provider.h"
#include "PBOC_Provider.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// 密钥相关算法 
class Key_Provider
{
private:
    //----------------------------------------------------- 
    Key_Provider() {}
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 计算KCV的值 
     */ 
    inline static bool KCV(const ByteArray& key_8_16_24, ByteBuilder& kcv_8)
    {
        byte iv[DES_BLOCK_SIZE] = { 0 };
        return PBOC_Provider::Encrypt(key_8_16_24, ByteArray(iv, DES_BLOCK_SIZE), kcv_8) > 0;
    }
    /**
     * @brief 格式化ANSI标准的卡号信息 
     *
     * - 格式:
     *  - 截掉卡号的最后一位
     *  - 向前取12位
     *  - 将数据转换为6字节压缩数字
     *  - 在6字节数字前补两字节0x00
     *  - 获取到的密码格式为: "06111111FFFFFFFF"
     *  - 将卡号数据与密码做异或操作
     *  - 使用密钥加密异或后的结果即为加密的密码
     * .
     * 
     * @param [in] cardNumber 输入的卡号数据
     * @param [out] cardId 获取到的8字节卡号格式数据
     * @retval 返回截取的卡号长度
     */
    static size_t FormatAnsiCardID(const ByteArray& cardNumber, ByteBuilder& cardId)
    {
        const size_t FORMAT_LEN = 12;

        ByteBuilder tmp(16);
        size_t len = cardNumber.GetLength();
        if(len <= FORMAT_LEN)
        {
            // 前补 '0'
            tmp.Append(static_cast<byte>('0'), FORMAT_LEN - len);
            tmp += cardNumber;
        }
        else
        {
            tmp += cardNumber.SubArray(len - FORMAT_LEN - 1, FORMAT_LEN);
            len = FORMAT_LEN;
        }

        cardId.Append(0x00, 2);
        ByteConvert::FromAscii(tmp, cardId);

        return len;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace security
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_KEY_PROVIDER_H_
//========================================================= 