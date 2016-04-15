//========================================================= 
/**@file SM4_Provider.h 
 * @brief 国密SM4算法(参照goldboar实现) 
 * 
 * @date 2014-10-25   10:56:29 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SM4_PROVIDER_H_
#define _LIBZHOUYB_SM4_PROVIDER_H_
//--------------------------------------------------------- 
#include "security.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
// 数据块字节大小 
#define SM4_BLOCK_SIZE 16

#define SM4_GET_ULONG_BE(n,b,i)                 \
{                                               \
    (n) = (static_cast<ulong>((b)[(i)    ] << 24 ))        \
        | (static_cast<ulong>((b)[(i) + 1] << 16 ))        \
        | (static_cast<ulong>((b)[(i) + 2] <<  8 ))        \
        | (static_cast<ulong>((b)[(i) + 3]       ));       \
}

#define SM4_PUT_ULONG_BE(n,b,i)                \
{                                              \
    (b)[(i)    ] = _itobyte( (n) >> 24 );       \
    (b)[(i) + 1] = _itobyte( (n) >> 16 );       \
    (b)[(i) + 2] = _itobyte( (n) >>  8 );       \
    (b)[(i) + 3] = _itobyte( (n)       );       \
}

#define SM4_SHL(x,n) (((x) & 0xFFFFFFFF) << n)
#define SM4_ROTL(x,n) (SM4_SHL((x),n) | ((x) >> (32 - n)))
#define SM4_SWAP(a,b) { ulong t = a; a = b; b = t; t = 0; }

extern const byte SM4_SboxTable[16][16];
extern const ulong SM4_FK[4];
extern const ulong SM4_CK[32];
//--------------------------------------------------------- 
/// SM4算法 
class SM4_Provider : public ISecurityAlgorithm
{
public:
    //----------------------------------------------------- 
    /// 加密方式 
    enum SM4_Mode
    {
        /// 未知 
        SM4_Unknown = 0,
        /// 加密 
        SM4_Encrypt = 1,
        /// 解密 
        SM4_Decrypt = 2
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 加密/解密 
    SM4_Mode _mode;
    /// 子密钥 
    ulong _sk[32];
    /// 原始密钥 
    ByteBuilder _key;
    /// ECB IV向量 
    ByteBuilder _iv;
    //----------------------------------------------------- 
    /**
     * @brief look up in SboxTable and get the related value.
     * @param [in] inch 0x00~0xFF (8 bits unsigned value)
     */
    static byte _sm4Sbox(byte inch)
    {
        byte *pTable = ctype_cast(byte*)(SM4_SboxTable);
        byte retVal = static_cast<byte>(pTable[inch]);
        return retVal;
    }
    /**
     * @brief "T algorithm" == "L algorithm" + "t algorithm".
     * @param [in] ka a: a is a 32 bits unsigned value;
     */
    static ulong _sm4Lt(ulong ka)
    {
        ulong bb = 0;
        ulong c = 0;
        byte a[4];
        byte b[4];
        SM4_PUT_ULONG_BE(ka, a, 0);
        b[0] = _sm4Sbox(a[0]);
        b[1] = _sm4Sbox(a[1]);
        b[2] = _sm4Sbox(a[2]);
        b[3] = _sm4Sbox(a[3]);
        SM4_GET_ULONG_BE(bb, b, 0);
        c = bb ^ (SM4_ROTL(bb, 2)) ^ (SM4_ROTL(bb, 10)) ^ (SM4_ROTL(bb, 18)) ^ (SM4_ROTL(bb, 24));
        return c;
    }
    /**
     * @brief Calculating and getting encryption/decryption contents.
     * @param [in] x0 original contents;
     * @param [in] x1 original contents;
     * @param [in] x2 original contents;
     * @param [in] x3 original contents;
     * @param [in] rk encryption/decryption key;
     */
    static ulong _sm4F(ulong x0, ulong x1, ulong x2, ulong x3, ulong rk)
    {
        return (x0^_sm4Lt(x1^x2^x3^rk));
    }
    /**
     * @brief Calculating round encryption key.
     * @param [in] ka a: a is a 32 bits unsigned value;
     */
    static ulong _sm4CalciRK(ulong ka)
    {
        ulong bb = 0;
        ulong rk = 0;
        byte a[4];
        byte b[4];
        SM4_PUT_ULONG_BE(ka,a,0)
        b[0] = _sm4Sbox(a[0]);
        b[1] = _sm4Sbox(a[1]);
        b[2] = _sm4Sbox(a[2]);
        b[3] = _sm4Sbox(a[3]);
        SM4_GET_ULONG_BE(bb,b,0)
        rk = bb^(SM4_ROTL(bb, 13))^(SM4_ROTL(bb, 23));
        return rk;
    }
    /// 转换密钥 
    static void _sm4_setkey(ulong* SK, const byte* key)
    {
        ulong MK[4];
        ulong k[36];
        ulong i = 0;

        SM4_GET_ULONG_BE( MK[0], key, 0 );
        SM4_GET_ULONG_BE( MK[1], key, 4 );
        SM4_GET_ULONG_BE( MK[2], key, 8 );
        SM4_GET_ULONG_BE( MK[3], key, 12 );
        k[0] = MK[0]^SM4_FK[0];
        k[1] = MK[1]^SM4_FK[1];
        k[2] = MK[2]^SM4_FK[2];
        k[3] = MK[3]^SM4_FK[3];
        for(; i<32; i++)
        {
            k[i+4] = k[i] ^ (_sm4CalciRK(k[i+1]^k[i+2]^k[i+3]^SM4_CK[i]));
            SK[i] = k[i+4];
        }
    }
    /// SM4数据运算 
    static void _sm4_one_round(ulong* sk, const byte* input, byte* output)
    {
        ulong i = 0;
        ulong ulbuf[36];

        memset(ulbuf, 0, sizeof(ulbuf));
        SM4_GET_ULONG_BE( ulbuf[0], input, 0 )
        SM4_GET_ULONG_BE( ulbuf[1], input, 4 )
        SM4_GET_ULONG_BE( ulbuf[2], input, 8 )
        SM4_GET_ULONG_BE( ulbuf[3], input, 12 )
        while(i<32)
        {
            ulbuf[i+4] = _sm4F(ulbuf[i], ulbuf[i+1], ulbuf[i+2], ulbuf[i+3], sk[i]);
            i++;
        }
        SM4_PUT_ULONG_BE(ulbuf[35],output,0);
        SM4_PUT_ULONG_BE(ulbuf[34],output,4);
        SM4_PUT_ULONG_BE(ulbuf[33],output,8);
        SM4_PUT_ULONG_BE(ulbuf[32],output,12);
    }
    //----------------------------------------------------- 
    /// 设置加密密钥 
    void setkey_enc(const byte* key_16)
    {
        _mode = SM4_Encrypt;
        _sm4_setkey(_sk, key_16);
    }
    /// 设置解密密钥 
    void setkey_dec(const byte* key_16)
    {
        _mode = SM4_Decrypt;
        _sm4_setkey(_sk, key_16);
        for(int i = 0;i < 16;++i)
        {
            SM4_SWAP(_sk[i], _sk[31-i]);
        }
    }
    /// ECB运算 
    size_t ecb_crypt(const byte* input_16X, size_t len, byte* output_16X)
    {
        size_t count = 0;
        while(len > 0)
        {
            _sm4_one_round(_sk, input_16X, output_16X);
            input_16X  += 16;
            output_16X += 16;
            len -= 16;

            count += 16;
        }
        return count;
    }
    /// CBC运算 
    size_t cbc_crypt(const byte* iv_16, const byte* input_16X, size_t len, byte* output_16X)
    {
        if(_mode == SM4_Unknown)
            return 0;

        byte tmp[16];
        byte iv[16] = {0};

        size_t count = 0;

        memcpy(iv, iv_16, 16);

        if(_mode == SM4_Encrypt)
        {
            while(len > 0)
            {
                for(int i = 0;i < 16; ++i)
                    output_16X[i] = static_cast<byte>(input_16X[i] ^ iv[i]);

                _sm4_one_round(_sk, output_16X, output_16X);
                memcpy(iv, output_16X, 16);

                input_16X  += 16;
                output_16X += 16;
                len -= 16;

                count += 16;
            }
        }
        else /* SM4_DECRYPT */
        {
            while(len > 0)
            {
                memcpy(tmp, input_16X, 16);
                _sm4_one_round(_sk, input_16X, output_16X);

                for(int i = 0;i < 16; ++i)
                    output_16X[i] = static_cast<byte>(output_16X[i] ^ iv[i]);

                memcpy(iv, tmp, 16);

                input_16X  += 16;
                output_16X += 16;
                len -= 16;

                count += 16;
            }
        }

        return count;
    }
    /// 数据运算 
    bool _Crypt(const ByteArray& data, ByteBuilder& dst)
    {
        // 数据长度正确 
        ASSERT_Func((data.GetLength() % SM4_BLOCK_SIZE == 0));

        byte tmpBuff[SM4_BLOCK_SIZE] = { 0 };
        int times = static_cast<int>(data.GetLength() / SM4_BLOCK_SIZE);
        bool isCBC = !(_iv.IsEmpty());
        for(int i = 0; i < times; ++i)
        {
            if(isCBC)
                cbc_crypt(_iv.GetBuffer(), data.GetBuffer(i*SM4_BLOCK_SIZE), SM4_BLOCK_SIZE, tmpBuff);
            else
                ecb_crypt(data.GetBuffer(i*SM4_BLOCK_SIZE), SM4_BLOCK_SIZE, tmpBuff);
            dst.Append(ByteArray(tmpBuff, SM4_BLOCK_SIZE));
        }

        return true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 初始化密钥 
    bool Init(const ByteArray& key_16, const ByteArray& cbc_iv_16 = "")
    {
        _key.Clear();
        _iv.Clear();

        ASSERT_Func(key_16.GetLength() >= SM4_BLOCK_SIZE);
        if(cbc_iv_16.GetLength() >= SM4_BLOCK_SIZE)
        {
            _iv.Append(cbc_iv_16.SubArray(0, SM4_BLOCK_SIZE));
        }

        _key.Append(key_16.SubArray(0, SM4_BLOCK_SIZE));
        return true;
    }
    /// 返回对象是否有效 
    inline bool IsValid() const
    {
        return !(_key.IsEmpty());
    }
    /// 加密 
    virtual bool Encrypt(const ByteArray& data, ByteBuilder& dst)
    {
        ASSERT_Func(IsValid());
        setkey_enc(_key.GetBuffer());
        return _Crypt(data, dst);
    }
    /// 解密 
    virtual bool Decrypt(const ByteArray& data, ByteBuilder& dst)
    {
        ASSERT_Func(IsValid());
        setkey_dec(_key.GetBuffer());
        return _Crypt(data, dst);
    }
    //----------------------------------------------------- 
    /// ECB加密 
    static bool ECB_Encrypt(const ByteArray& key_16, const ByteArray& buff_16X, ByteBuilder& dst_16X)
    {
        SM4_Provider sm4;
        ASSERT_Func(sm4.Init(key_16));

        return sm4.Encrypt(buff_16X, dst_16X);
    };
    /// ECB解密 
    static bool ECB_Decrypt(const ByteArray& key_16, const ByteArray& buff_16X, ByteBuilder& dst_16X)
    {
        SM4_Provider sm4;
        ASSERT_Func(sm4.Init(key_16));

        return sm4.Decrypt(buff_16X, dst_16X);
    }
    /// CBC加密 
    static bool CBC_Encrypt(const ByteArray& key_16, const ByteArray& iv_16, const ByteArray& buff_16X, ByteBuilder& dst_16X)
    {
        SM4_Provider sm4;
        ASSERT_Func(sm4.Init(key_16, iv_16));

        return sm4.Encrypt(buff_16X, dst_16X);
    }
    /// CBC解密 
    static bool CBC_Decrypt(const ByteArray& key_16, const ByteArray& iv_16, const ByteArray& buff_16X, ByteBuilder& dst_16X)
    {
        SM4_Provider sm4;
        ASSERT_Func(sm4.Init(key_16, iv_16));

        return sm4.Decrypt(buff_16X, dst_16X);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace security
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SM4_PROVIDER_H_
//========================================================= 