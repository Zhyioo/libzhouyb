//========================================================= 
/**@file security.h 
 * @brief 数据安全接口 
 * 
 * @date 2015-01-25   09:49:35 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// 散列算法接口 
struct IHashAlgorithm
{
    //----------------------------------------------------- 
    /// 计算字符串的散列值 
    static bool String(IHashAlgorithm& hashAlgorithm, const char* src, ByteBuilder& digest)
    {
        ASSERT_Func(hashAlgorithm.Update(ByteArray(src)));
        return hashAlgorithm.Final(digest);
    }
    /// 计算文件的散列值 
    static bool File(IHashAlgorithm& hashAlgorithm, const char* fileName, ByteBuilder& digest)
    {
        if(_is_empty_or_null(fileName))
            return false;

        FILE *file;
        size_t len;
        byte buffer[1024];

        if((file = fopen(fileName, "rb")) == NULL)
            return false;

        while((len = fread(buffer, 1, 1024, file)) > 0)
            ASSERT_Func(hashAlgorithm.Update(ByteArray(buffer, len)));

        fclose(file);

        return hashAlgorithm.Final(digest);;
    }
    /// 计算16进制数据的散列值 
    static bool Stream(IHashAlgorithm& hashAlgorithm, const ByteArray& buff, ByteBuilder& digest)
    {
        ASSERT_Func(hashAlgorithm.Update(buff));
        return hashAlgorithm.Final(digest);
    }
    //----------------------------------------------------- 
    /// 更新散列值 
    virtual bool Update(const ByteArray& input) = 0;
    /// 获取最终的散列值 
    virtual bool Final(ByteBuilder& digest) = 0;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 数据基本加解密接口 
struct ISecurityAlgorithm
{
    /// 加密 
    virtual bool Encrypt(const ByteArray& data, ByteBuilder& dst) = 0;
    /// 解密 
    virtual bool Decrypt(const ByteArray& data, ByteBuilder& dst) = 0;
};
//--------------------------------------------------------- 
} // namespace security 
} // namespace extension 
} // namespace zhou_yb
//========================================================= 