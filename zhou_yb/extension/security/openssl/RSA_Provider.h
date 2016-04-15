//========================================================= 
/**@file RSA_Provider.h 
 * @brief 基于OPEN SSL封装的RSA算法实现 
 * 
 * @date 2015-01-20   18:24:31 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_RSA_PROVIDER_H_
#define _LIBZHOUYB_RSA_PROVIDER_H_
//--------------------------------------------------------- 
#include "OpenSSL_Base.h"

#include <openssl/rsa.h>
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// 基于OPEN SSL的RSA算法封装 
class RSA_Provider
{
protected:
    //----------------------------------------------------- 
    typedef openssl_obj<RSA, RSA_free> openssl_RSA;
    //----------------------------------------------------- 
    /// RSA数据指针 
    openssl_RSA _rsa;
    //----------------------------------------------------- 
    /// 不允许手动创建,只能从静态函数创建返回 
    RSA_Provider() {}
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// RSA密钥类型 
    enum RSA_KeyMode
    {
        /// 公钥 
        PublicKEY = 0,
        /// 私钥 
        PrivateKEY
    };
    //----------------------------------------------------- 
    /**
     * @brief 产生RSA密钥(公私钥) 
     */ 
    static RSA_Provider New(int num = 1024, ulong e = RSA_F4)
    {
        RSA_Provider rsa;
        rsa._rsa = RSA_generate_key(num, e, NULL, NULL);

        return rsa;
    }
    /**
     * @brief 产生公钥 
     */ 
    static RSA_Provider CreatePublicKey(const ByteArray& m, const ByteArray& e)
    {
        RSA_Provider rsa;
        rsa._rsa = RSA_new();

        if(!rsa.IsValid())
            return RSA_Provider();

        BN_bin2bn(m.GetBuffer(), m.GetLength(), rsa._rsa->n);
        BN_bin2bn(e.GetBuffer(), e.GetLength(), rsa._rsa->e);

        return rsa;
    }
    /**
     * @brief 产生私钥 
     */ 
    static RSA_Provider CreatePrivateKey(const ByteArray& m, const ByteArray& e, 
        const ByteArray& p, const ByteArray& q,
        const ByteArray& dp, const ByteArray& dq, 
        const ByteArray& iq, const ByteArray& d)
    {
        RSA_Provider rsaNull;
        RSA_Provider rsa;
        rsa._rsa = RSA_new();

        if(!rsa.IsValid())
            return rsaNull;

        BN_bin2bn(m.GetBuffer(), m.GetLength(), rsa._rsa->n);
        BN_bin2bn(e.GetBuffer(), e.GetLength(), rsa._rsa->e);
        BN_bin2bn(p.GetBuffer(), p.GetLength(), rsa._rsa->p);
        BN_bin2bn(q.GetBuffer(), q.GetLength(), rsa._rsa->q);
        BN_bin2bn(dp.GetBuffer(), dp.GetLength(), rsa._rsa->dmp1);
        BN_bin2bn(dq.GetBuffer(), dq.GetLength(), rsa._rsa->dmq1);
        BN_bin2bn(iq.GetBuffer(), iq.GetLength(), rsa._rsa->iqmp);
        BN_bin2bn(d.GetBuffer(), d.GetLength(), rsa._rsa->d);

        if(!RSA_check_key(rsa._rsa))
            return rsaNull;

        return rsa;
    }
    //----------------------------------------------------- 
    /// 返回对象是否有效 
    bool IsValid() const
    {
        return !(_rsa.IsNull());
    }
    /// RSA加密 
    bool Encrypt(const ByteArray& data, ByteBuilder& dst, RSA_KeyMode keyMode, int padding = RSA_NO_PADDING)
    {
        ASSERT_Func(IsValid());

        size_t len = RSA_size(_rsa);
        dst.Append(static_cast<byte>(0x00), len);
        byte* dstBuff = const_cast<byte*>(dst.GetBuffer(dst.GetLength() - len));

        int iRet = 0;
        switch(keyMode)
        {
        case PrivateKEY:
            iRet = RSA_private_encrypt(data.GetLength(), data.GetBuffer(), dstBuff, _rsa, padding);
            break;
        case PublicKEY:
        default:
            iRet = RSA_public_encrypt(data.GetLength(), data.GetBuffer(), dstBuff, _rsa, padding);
            break;
        }

        if(!iRet)
        {
            dst.RemoveTail(len);
        }

        return true;
    }
    /// RSA解密 
    bool Decrypt(const ByteArray& data, ByteBuilder& dst, RSA_KeyMode keyMode, int padding = RSA_NO_PADDING)
    {
        ASSERT_Func(IsValid());

        size_t len = RSA_size(_rsa);
        dst.Append(static_cast<byte>(0x00), len);
        byte* dstBuff = const_cast<byte*>(dst.GetBuffer(dst.GetLength() - len));

        int iRet = 0;

        switch(keyMode)
        {
        case PrivateKEY:
            iRet = RSA_private_decrypt(data.GetLength(), data.GetBuffer(), dstBuff, _rsa, padding);
            break;
        case PublicKEY:
        default:
            iRet = RSA_public_decrypt(data.GetLength(), data.GetBuffer(), dstBuff, _rsa, padding);
            break;
        }

        if(!iRet)
        {
            dst.RemoveTail(len);
        }


        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace security
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_RSA_PROVIDER_H_
//========================================================= 