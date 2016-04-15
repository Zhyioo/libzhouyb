//========================================================= 
/**@file SM3_Provider.h 
 * @brief 国密SM3算法 
 * 
 * @date 2014-10-25   10:54:24 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SM3_PROVIDER_H_
#define _LIBZHOUYB_SM3_PROVIDER_H_
//--------------------------------------------------------- 
#include "security.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// 生成的SM3数据值大小 
#define SM3_DIGEST_SIZE 32

#define SM3_GET_ULONG_BE(n,b,i)                 \
{                                               \
    (n) = (static_cast<ulong>((b)[(i)    ] << 24 ))        \
        | (static_cast<ulong>((b)[(i) + 1] << 16 ))        \
        | (static_cast<ulong>((b)[(i) + 2] <<  8 ))        \
        | (static_cast<ulong>((b)[(i) + 3]       ));       \
}

#define SM3_PUT_ULONG_BE(n,b,i)                \
{                                              \
    (b)[(i)    ] = _itobyte( (n) >> 24 );       \
    (b)[(i) + 1] = _itobyte( (n) >> 16 );       \
    (b)[(i) + 2] = _itobyte( (n) >>  8 );       \
    (b)[(i) + 3] = _itobyte( (n)       );       \
}

#define SM3_FF0(x,y,z) ( (x) ^ (y) ^ (z)) 
#define SM3_FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define SM3_GG0(x,y,z) ( (x) ^ (y) ^ (z)) 
#define SM3_GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )

#define  SM3_SHL(x,n) (((x) & 0xFFFFFFFF) << n)
#define SM3_ROTL(x,n) (SM3_SHL((x),n) | ((x) >> (32 - n)))

#define SM3_P0(x) ((x) ^  SM3_ROTL((x),9) ^ SM3_ROTL((x),17)) 
#define SM3_P1(x) ((x) ^  SM3_ROTL((x),15) ^ SM3_ROTL((x),23))

extern const byte SM3_PADDING[64];
//--------------------------------------------------------- 
/// SM3数据结构 
typedef struct _sm3_struct
{
    /* number of bytes processed */
    ulong total[2];
    /* intermediate digest state */
    ulong state[8];
    /* data block being processed */
    byte buffer[64];
    /* HMAC: inner padding */
    byte ipad[64];
    /* HMAC: outer padding */
    byte opad[64];
}
sm3_context;
//--------------------------------------------------------- 
/// SM3算法 
class SM3_Provider : public IHashAlgorithm
{
protected:
    //----------------------------------------------------- 
    /// 数据对象 
    sm3_context _ctx;
    /// SM3计算过程 
    static void _sm3_process(sm3_context *ctx, const byte* data)
    {
        ulong SS1, SS2, TT1, TT2, W[68],W1[64];
        ulong A, B, C, D, E, F, G, H;
        ulong T[64];
        ulong Temp1,Temp2,Temp3,Temp4,Temp5;
        int j;

        for(j = 0; j < 16; j++)
            T[j] = 0x79CC4519;
        for(j =16; j < 64; j++)
            T[j] = 0x7A879D8A;

        SM3_GET_ULONG_BE( W[ 0], data,  0 );
        SM3_GET_ULONG_BE( W[ 1], data,  4 );
        SM3_GET_ULONG_BE( W[ 2], data,  8 );
        SM3_GET_ULONG_BE( W[ 3], data, 12 );
        SM3_GET_ULONG_BE( W[ 4], data, 16 );
        SM3_GET_ULONG_BE( W[ 5], data, 20 );
        SM3_GET_ULONG_BE( W[ 6], data, 24 );
        SM3_GET_ULONG_BE( W[ 7], data, 28 );
        SM3_GET_ULONG_BE( W[ 8], data, 32 );
        SM3_GET_ULONG_BE( W[ 9], data, 36 );
        SM3_GET_ULONG_BE( W[10], data, 40 );
        SM3_GET_ULONG_BE( W[11], data, 44 );
        SM3_GET_ULONG_BE( W[12], data, 48 );
        SM3_GET_ULONG_BE( W[13], data, 52 );
        SM3_GET_ULONG_BE( W[14], data, 56 );
        SM3_GET_ULONG_BE( W[15], data, 60 );

        for(j = 16; j < 68; j++ )
        {
            Temp1 = W[j-16] ^ W[j-9];
            Temp2 = SM3_ROTL(W[j-3],15);
            Temp3 = Temp1 ^ Temp2;
            Temp4 = SM3_P1(Temp3);
            Temp5 =  SM3_ROTL(W[j - 13],7 ) ^ W[j-6];
            W[j] = Temp4 ^ Temp5;
        }

        for(j =  0; j < 64; j++)
        {
            W1[j] = W[j] ^ W[j+4];
        }

        A = ctx->state[0];
        B = ctx->state[1];
        C = ctx->state[2];
        D = ctx->state[3];
        E = ctx->state[4];
        F = ctx->state[5];
        G = ctx->state[6];
        H = ctx->state[7];

        for(j =0; j < 16; j++)
        {
            SS1 = SM3_ROTL((SM3_ROTL(A,12) + E + SM3_ROTL(T[j],j)), 7); 
            SS2 = SS1 ^ SM3_ROTL(A,12);
            TT1 = SM3_FF0(A,B,C) + D + SS2 + W1[j];
            TT2 = SM3_GG0(E,F,G) + H + SS1 + W[j];
            D = C;
            C = SM3_ROTL(B,9);
            B = A;
            A = TT1;
            H = G;
            G = SM3_ROTL(F,19);
            F = E;
            E = SM3_P0(TT2);
        }
    
        for(j =16; j < 64; j++)
        {
            SS1 = SM3_ROTL((SM3_ROTL(A,12) + E + SM3_ROTL(T[j],j)), 7); 
            SS2 = SS1 ^ SM3_ROTL(A,12);
            TT1 = SM3_FF1(A,B,C) + D + SS2 + W1[j];
            TT2 = SM3_GG1(E,F,G) + H + SS1 + W[j];
            D = C;
            C = SM3_ROTL(B,9);
            B = A;
            A = TT1;
            H = G;
            G = SM3_ROTL(F,19);
            F = E;
            E = SM3_P0(TT2);
        }

        ctx->state[0] ^= A;
        ctx->state[1] ^= B;
        ctx->state[2] ^= C;
        ctx->state[3] ^= D;
        ctx->state[4] ^= E;
        ctx->state[5] ^= F;
        ctx->state[6] ^= G;
        ctx->state[7] ^= H;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    SM3_Provider() { Init(); }
    /**
     * @brief SM3 Context
     */
    inline sm3_context& Context()
    {
        return _ctx;
    }
    void Init()
    {
        memset(&_ctx, 0, sizeof(_ctx));

        _ctx.total[0] = 0;
        _ctx.total[1] = 0;

        _ctx.state[0] = 0x7380166F;
        _ctx.state[1] = 0x4914B2B9;
        _ctx.state[2] = 0x172442D7;
        _ctx.state[3] = 0xDA8A0600;
        _ctx.state[4] = 0xA96F30BC;
        _ctx.state[5] = 0x163138AA;
        _ctx.state[6] = 0xE38DEE4D;
        _ctx.state[7] = 0xB0FB0E4E;
    }
    virtual bool Update(const ByteArray& input)
    {
        size_t fill;
        ulong left;
        size_t ilen = input.GetLength();
        const byte* ptr = input.GetBuffer();
        if(ilen <= 0)
            return false;

        left = _ctx.total[0] & 0x3F;
        fill = 64 - left;

        _ctx.total[0] += ilen;
        _ctx.total[0] &= 0xFFFFFFFF;

        if(_ctx.total[0] < static_cast<ulong>(ilen))
            _ctx.total[1]++;

        if(left && ilen >= fill)
        {
            memcpy(reinterpret_cast<void*>(_ctx.buffer + left), 
                reinterpret_cast<void*>(const_cast<byte*>(ptr)), fill);
            _sm3_process(&_ctx, _ctx.buffer);
            ptr += fill;
            ilen  -= fill;
            left = 0;
        }

        while(ilen >= 64)
        {
            _sm3_process(&_ctx, ptr);
            ptr += 64;
            ilen  -= 64;
        }

        if(ilen > 0)
        {
            memcpy(reinterpret_cast<void*>(_ctx.buffer + left), 
                reinterpret_cast<void*>(const_cast<byte*>(ptr)), ilen);
        }

        return true;
    }
    virtual bool Final(ByteBuilder& digest_32)
    {
        ulong last, padn;
        ulong high, low;
        byte msglen[8];
        byte digest[SM3_DIGEST_SIZE] = { 0 };

        high = (_ctx.total[0] >> 29) | (_ctx.total[1] <<  3);
        low  = (_ctx.total[0] <<  3);

        SM3_PUT_ULONG_BE(high, msglen, 0);
        SM3_PUT_ULONG_BE(low,  msglen, 4);

        last = _ctx.total[0] & 0x3F;
        padn = (last < 56)?(56 - last):(120 - last);

        Update(ByteArray(SM3_PADDING, padn));
        Update(ByteArray(msglen, 8));

        SM3_PUT_ULONG_BE(_ctx.state[0], digest, 0);
        SM3_PUT_ULONG_BE(_ctx.state[1], digest, 4);
        SM3_PUT_ULONG_BE(_ctx.state[2], digest, 8);
        SM3_PUT_ULONG_BE(_ctx.state[3], digest, 12);
        SM3_PUT_ULONG_BE(_ctx.state[4], digest, 16);
        SM3_PUT_ULONG_BE(_ctx.state[5], digest, 20);
        SM3_PUT_ULONG_BE(_ctx.state[6], digest, 24);
        SM3_PUT_ULONG_BE(_ctx.state[7], digest, 28);

        digest_32.Append(ByteArray(digest, SM3_DIGEST_SIZE));

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 带密钥的SM3运算 
class SM3_HMAC : public IHashAlgorithm
{
protected:
    //----------------------------------------------------- 
    SM3_Provider _sm3;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    void Init(const ByteArray& key)
    {
        _sm3.Init();

        ByteBuilder sum(SM3_DIGEST_SIZE);
        const byte* buff = key.GetBuffer();
        size_t len = key.GetLength();

        if(len > 64)
        {
            SM3_Provider sm3;
            sm3.Update(key);
            sm3.Final(sum);

            buff = const_cast<byte*>(sum.GetBuffer());
            len = SM3_DIGEST_SIZE;
        }

        sm3_context& ctx = _sm3.Context();

        memset(ctx.ipad, 0x36, 64);
        memset(ctx.opad, 0x5C, 64);

        for(size_t i = 0; i < len; i++)
        {
            ctx.ipad[i] = static_cast<byte>(ctx.ipad[i] ^ buff[i]);
            ctx.opad[i] = static_cast<byte>(ctx.opad[i] ^ buff[i]);
        }

        _sm3.Update(ByteArray(ctx.ipad, 64));
    }
    virtual bool Update(const ByteArray& input)
    {
        return _sm3.Update(input);
    }
    virtual bool Final(ByteBuilder& digest_32)
    {
        ByteBuilder tmp(SM3_DIGEST_SIZE);
        ASSERT_Func(_sm3.Final(tmp));

        SM3_Provider sm3;
        sm3.Update(ByteArray(_sm3.Context().opad, 64));
        sm3.Update(tmp);
        return sm3.Final(digest_32);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
#undef SM3_GET_ULONG_BE
#undef SM3_PUT_ULONG_BE
#undef SM3_FF0
#undef SM3_FF1
#undef SM3_GG0
#undef SM3_GG1
#undef SM3_SHL
#undef SM3_ROTL
#undef SM3_P0
#undef SM3_P1
//--------------------------------------------------------- 
} // namespace security 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SM3_PROVIDER_H_
//========================================================= 