//========================================================= 
/**@file SM2_Provider.h
* @brief 基于OPEN SSL封装的RSA算法实现
*
* @date 2015-01-20   18:24:31
* @author Zhyioo
* @version 1.0
*/
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SM2_PROVIDER_H_
#define _LIBZHOUYB_SM2_PROVIDER_H_
//--------------------------------------------------------- 
#include "OpenSSL_Base.h"

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>

#include "../SM3_Provider.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// 基于OPEN SSL的SM2算法封装 
class SM2_Provider
{
protected:
    //----------------------------------------------------- 
    typedef openssl_obj<BIGNUM, BN_free> openssl_BN;
    typedef openssl_obj<BN_CTX, BN_CTX_free> openssl_BN_CTX;
    typedef openssl_obj<EC_GROUP, EC_GROUP_free> openssl_EC_GROUP;
    typedef openssl_obj<EC_POINT, EC_POINT_free> openssl_EC_POINT;
    //----------------------------------------------------- 
    static size_t SM3_KDF(const ByteArray& share, size_t keyLen, ByteBuilder& key)
    {
        byte counter[4] = { 0, 0, 0, 1 };
        ByteBuilder digest(SM3_DIGEST_SIZE);
        
        SM3_Provider sm3;
        size_t len = 0;
        size_t tmp = 0;
        for(size_t i = 0; i < keyLen;i += SM3_DIGEST_SIZE)
        {
            sm3.Update(share);
            sm3.Update(ByteArray(counter, 4));
            sm3.Final(digest);

            tmp = _min(SM3_DIGEST_SIZE, keyLen - i);
            key.Append(digest.SubArray(0, tmp));

            len += tmp;
        }

        return len;
    }
    //----------------------------------------------------- 
    /// EC数据对象 
    openssl_BN_CTX _ctx;
    /// EC曲线 
    openssl_EC_GROUP _ec_group;
    /// EC深度 
    uint _degree;
    //----------------------------------------------------- 
    /// 初始化EC 
    bool InitEC(const ByteArray& p, const ByteArray& a, const ByteArray& b, const ByteArray& Gx, const ByteArray& Gy, const ByteArray& n, uint degree)
    {
        _ctx.Free();
        _ec_group.Free();

        _ctx = BN_CTX_new();
        _ec_group = EC_GROUP_new(EC_GFp_mont_method());

        openssl_BN _p = BN_new();
        openssl_BN _a = BN_new();
        openssl_BN _b = BN_new();
        openssl_BN _Gx = BN_new();
        openssl_BN _Gy = BN_new();
        openssl_BN _tGy = BN_new();
        openssl_BN _n = BN_new();

        openssl_EC_POINT P;
        openssl_EC_POINT Q;

        ASSERT_Func(!_ctx.IsNull() && !_ec_group.IsNull() && !_p.IsNull() &&
            !_a.IsNull() && !_b.IsNull() && !_Gx.IsNull() && !_Gy.IsNull() &&
            !_n.IsNull());

        ASSERT_Func(BN_bin2bn(p.GetBuffer(), p.GetLength(), _p));
        ASSERT_Func(BN_is_prime_ex(_p, BN_prime_checks, _ctx, NULL) == 1);

        ASSERT_Func(BN_bin2bn(a.GetBuffer(), a.GetLength(), _a));
        ASSERT_Func(BN_bin2bn(b.GetBuffer(), b.GetLength(), _b));

        ASSERT_Func(EC_GROUP_set_curve_GFp(_ec_group, _p, _a, _b, _ctx));

        P = EC_POINT_new(_ec_group);
        Q = EC_POINT_new(_ec_group);

        ASSERT_Func(!P.IsNull() && !Q.IsNull());

        ASSERT_Func(BN_bin2bn(Gx.GetBuffer(), Gx.GetLength(), _Gx));
        ASSERT_Func(EC_POINT_set_compressed_coordinates_GFp(_ec_group, P, _Gx, 0, _ctx));
        ASSERT_Func(EC_POINT_is_on_curve(_ec_group, P, _ctx));

        ASSERT_Func(BN_bin2bn(n.GetBuffer(), n.GetLength(), _n));
        ASSERT_Func(EC_GROUP_set_generator(_ec_group, P, _n, BN_value_one()));
        ASSERT_Func(EC_POINT_get_affine_coordinates_GFp(_ec_group, P, _Gx, _tGy, _ctx));

        ASSERT_Func(BN_bin2bn(Gy.GetBuffer(), Gy.GetLength(), _tGy));
        // 比较计算出来的G(y)和输入的G(y)是否一致 
        ASSERT_Func(BN_cmp(_Gy, _tGy));

        _degree = static_cast<uint>(EC_GROUP_get_degree(_ec_group));
        ASSERT_Func(_degree == static_cast<int>(degree));

        ASSERT_Func(EC_GROUP_get_order(_ec_group, _n, _ctx));
        ASSERT_Func(EC_GROUP_precompute_mult(_ec_group, _ctx));
        ASSERT_Func(EC_POINT_mul(_ec_group, Q, _n, NULL, NULL, _ctx));
        ASSERT_Func(EC_POINT_is_at_infinity(_ec_group, Q));

        return true;
    }
    //----------------------------------------------------- 
    SM2_Provider() {}
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 根据指定的数据生成SM2算法 
    static SM2_Provider Create(const ByteArray& p, const ByteArray& a, const ByteArray& b, const ByteArray& Gx, const ByteArray& Gy, const ByteArray& n, uint degree)
    {
        SM2_Provider sm2;
        if(sm2.InitEC(p, a, b, Gx, Gy, n, degree))
            return sm2;

        // 返回一个空无效对象 
        return SM2_Provider();
    }
    /// 创建标准的SM2算法 
    static SM2_Provider Default()
    {
        const size_t LENGTH = 32;
        ByteBuilder p(LENGTH);
        ByteBuilder a(LENGTH);
        ByteBuilder b(LENGTH);
        ByteBuilder Gx(LENGTH);
        ByteBuilder Gy(LENGTH);
        ByteBuilder n(LENGTH);

        ByteConvert::FromAscii("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF", p);
        ByteConvert::FromAscii("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC", a);
        ByteConvert::FromAscii("28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93", b);
        ByteConvert::FromAscii("32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7", Gx);
        ByteConvert::FromAscii("BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0", Gy);
        ByteConvert::FromAscii("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123", n);

        return Create(p, a, b, Gx, Gy, n, 256);
    }
    //----------------------------------------------------- 
    /// 返回对象是否有效 
    bool IsValid() const
    {
        return !(_ec_group.IsNull()) && !(_ctx.IsNull());
    }
    //----------------------------------------------------- 
    /// 加密 
    bool Encrypt(const ByteArray& randomK, const ByteArray& ecX, const ByteArray& ecY, const ByteArray& data, ByteBuilder& encryptBuff)
    {
        ASSERT_Func(IsValid());

        openssl_EC_POINT C1 = EC_POINT_new(_ec_group);
        openssl_EC_POINT PB = EC_POINT_new(_ec_group);
        openssl_EC_POINT P = EC_POINT_new(_ec_group);

        openssl_BN k = BN_new();
        openssl_BN x = BN_new();
        openssl_BN y = BN_new();

        ASSERT_Func(!C1.IsNull() && !PB.IsNull() && !P.IsNull() &&
            !k.IsNull() && !x.IsNull() && !y.IsNull());

        // 随机数 
        ASSERT_Func(BN_bin2bn(randomK.GetBuffer(), randomK.GetLength(), k));
        ASSERT_Func(EC_POINT_mul(_ec_group, C1, k, NULL, NULL, _ctx));
        ASSERT_Func(EC_POINT_get_affine_coordinates_GFp(_ec_group, C1, x, y, _ctx));

        encryptBuff += static_cast<byte>(0x04);

        ByteBuilder xBin;
        ByteBuilder yBin;

        size_t bnLen = BN_num_bytes(x);
        xBin.Append(static_cast<byte>(0x00), bnLen);
        BN_bn2bin(x, const_cast<byte*>(xBin.GetBuffer()));

        bnLen = BN_num_bytes(y);
        yBin.Append(static_cast<byte>(0x00), bnLen);
        BN_bn2bin(y, const_cast<byte*>(yBin.GetBuffer()));

        encryptBuff += xBin;
        encryptBuff += yBin;

        ASSERT_Func(BN_bin2bn(ecX.GetBuffer(), ecX.GetLength(), x));
        ASSERT_Func(BN_bin2bn(ecY.GetBuffer(), ecY.GetLength(), y));

        ASSERT_Func(EC_POINT_set_affine_coordinates_GFp(_ec_group, PB, x, y, _ctx));
        ASSERT_Func(EC_POINT_mul(_ec_group, P, NULL, PB, k, _ctx));
        ASSERT_Func(EC_POINT_get_affine_coordinates_GFp(_ec_group, P, x, y, _ctx));

        xBin.Clear();
        bnLen = BN_num_bytes(x);
        xBin.Append(static_cast<byte>(0x00), bnLen);
        BN_bn2bin(x, const_cast<byte*>(xBin.GetBuffer()));

        yBin.Clear();
        bnLen = BN_num_bytes(y);
        yBin.Append(static_cast<byte>(0x00), bnLen);
        BN_bn2bin(y, const_cast<byte*>(yBin.GetBuffer()));

        ByteBuilder C2(data.GetLength());
        xBin += yBin;
        SM3_KDF(xBin, data.GetLength(), C2);

        ByteConvert::Xor(data, C2);
        encryptBuff += C2;

        xBin.RemoveTail(yBin.GetLength());
        xBin += data;
        xBin += yBin;

        SM3_Provider sm3;
        IHashAlgorithm::Stream(sm3, xBin, encryptBuff);

        return true;
    }
    /// 解密 
    bool Decrypt(const ByteArray& decryptBuff, const ByteArray& dB, ByteBuilder& data)
    {
        ASSERT_Func(IsValid());

        // 04 X(32字节) Y(32字节) C2(密文数据) C3(32字节) 
        size_t LENGTH = _degree / 8;
        // 校验数据长度 
        ASSERT_Func(decryptBuff.GetLength() > (3 * LENGTH + 1));

        openssl_BN x = BN_new();
        openssl_BN y = BN_new();
        openssl_BN h = BN_new();

        openssl_EC_POINT C1 = EC_POINT_new(_ec_group);
        openssl_EC_POINT S = EC_POINT_new(_ec_group);
        openssl_EC_POINT P = EC_POINT_new(_ec_group);

        ASSERT_Func(!x.IsNull() && !y.IsNull() && !h.IsNull() &&
            !C1.IsNull() && !S.IsNull() && !P.IsNull());

        ASSERT_Func(BN_bin2bn(decryptBuff.GetBuffer(1), LENGTH, x));
        ASSERT_Func(BN_bin2bn(decryptBuff.GetBuffer(1) + LENGTH, LENGTH, y));

        ASSERT_Func(EC_POINT_set_affine_coordinates_GFp(_ec_group, C1, x, y, _ctx));
        ASSERT_Func(EC_POINT_is_on_curve(_ec_group, C1, _ctx));
        ASSERT_Func(EC_GROUP_get_cofactor(_ec_group, h, _ctx));
        ASSERT_Func(EC_POINT_mul(_ec_group, S, NULL, C1, h, _ctx));
        // S不能是无穷远点 
        ASSERT_Func(!EC_POINT_is_at_infinity(_ec_group, S));

        ASSERT_Func(BN_bin2bn(dB.GetBuffer(), dB.GetLength(), x));
        ASSERT_Func(EC_POINT_mul(_ec_group, P, NULL, C1, x, _ctx));
        ASSERT_Func(EC_POINT_get_affine_coordinates_GFp(_ec_group, P, x, y, _ctx));

        ByteBuilder C2(LENGTH);
        ByteBuilder C3(LENGTH);

        C2 = decryptBuff.SubArray(1 + 2 * LENGTH, LENGTH);
        C3 = decryptBuff.SubArray(decryptBuff.GetLength() - LENGTH, LENGTH);

        ByteBuilder xBin;
        ByteBuilder yBin;

        size_t bnLen = BN_num_bytes(x);
        xBin.Append(static_cast<byte>(0x00), bnLen);
        ASSERT_Func(BN_bn2bin(x, const_cast<byte*>(xBin.GetBuffer())));

        bnLen = BN_num_bytes(y);
        yBin.Append(static_cast<byte>(0x00), bnLen);
        ASSERT_Func(BN_bn2bin(y, const_cast<byte*>(yBin.GetBuffer())));

        ByteBuilder tmp(2 * LENGTH);
        tmp = xBin;
        tmp += yBin;

        size_t dataLen = decryptBuff.GetLength() - 1 - 3 * LENGTH;
        
        ByteBuilder kdf;
        SM3_KDF(tmp, dataLen, kdf);

        ByteConvert::Xor(C2, kdf);

        tmp.Clear();
        tmp += xBin;
        tmp += kdf;
        tmp += yBin;

        C2.Clear();
        SM3_Provider sm3;
        IHashAlgorithm::Stream(sm3, tmp, C2);

        // 比较HASH值 
        ASSERT_Func(C2 == C3);
        data += kdf;
        
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace security
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SM2_PROVIDER_H_
//========================================================= 