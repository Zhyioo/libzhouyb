//========================================================= 
/**@file LC_Provider.h
 * @brief LC相关算法 
 * 
 * @date 2015-05-14   22:35:47
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_PROVIDER_H_
#define _LIBZHOUYB_LC_PROVIDER_H_
//--------------------------------------------------------- 
#include "../../../extension/security/security.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace lc {
//--------------------------------------------------------- 
/// LC相关安全算法 
class LC_Provider 
{
protected:
    LC_Provider() {}
public:
    /// 8字节随机数变换加密操作 
    static void RandomConvert(const ByteArray& random_8X, ByteBuilder& buff)
    {
        byte bit = 0;
        byte tmp = 0;
        byte randomArr[8] = { 0 };

        int count = static_cast<int>(random_8X.GetLength() / 8.0 + 0.5);
        for(int c = 0;c < count; ++c)
        {
            memcpy(randomArr, random_8X.GetBuffer(8 * c), _min(random_8X.GetLength() - 8 * c, 8));

            // 取所有字节的从高往低的第一位组成一个新的字节
            size_t i = 0;
            size_t j = 0;
            /* 生成第i个字节 */
            for(i = 0;i < 8; ++i)
            {
                bit = 0;
                /* 取每个字节的从高到低的第j位 */
                for(j = 0;j < 8; ++j)
                {
                    // 获取第j个字符的第i位的位(不移位)
                    tmp = (randomArr[j] >> (7 - i)) & 0x01;
                    bit |= (tmp << j);
                }
                buff += bit;
            }
        }
    }
    /// 校验设备认证数据 
    static bool Verify(const ByteArray& random_X, const ByteArray& buff_X)
    {
        if(random_X.GetLength() != buff_X.GetLength())
            return false;

        byte bit = 0;
        byte tmp = 0;
        size_t len = random_X.GetLength();
        for(size_t i = 0;i < len; ++i)
        {
            bit = random_X[i];
            bit &= 0x0F0;
            tmp = BitConvert::LeftShift(buff_X[i], random_X[i] & 0x0F);
            tmp &= 0x0F0;
            if(bit != tmp)
                return false;
        }
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace lc
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_PROVIDER_H_
//========================================================= 
