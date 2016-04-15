//========================================================= 
/**@file DES_Provider.h 
 * @brief DES算法实现 
 * 
 * @date 2014-03-14   19:58:48 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_DES_PROVIDER_H_
#define _LIBZHOUYB_DES_PROVIDER_H_
//--------------------------------------------------------- 
#include "security.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
// 数据块字节大小 
#define DES_BLOCK_SIZE 8
// 8字节 
#define DES_LONG unsigned int
//--------------------------------------------------------- 
extern DES_LONG des_SPtrans[8][64];
extern DES_LONG des_skb[8][64];
//--------------------------------------------------------- 
#define DES_PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
    (b)^=(t),\
    (a)^=((t)<<(n)))

#define DES_HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
    (a)=(a)^(t)^(t>>(16-(n))))

static int des_shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};

#define DES_c2l(c,l)    (l =((DES_LONG)(*((c)++)))    , \
             l|=((DES_LONG)(*((c)++)))<< 8L, \
             l|=((DES_LONG)(*((c)++)))<<16L, \
             l|=((DES_LONG)(*((c)++)))<<24L)

#define c2ln(c,l1,l2,n) { \
            c+=n; \
            l1=l2=0; \
            switch (n) { \
            case 8: l2 =((DES_LONG)(*(--(c))))<<24L; \
            case 7: l2|=((DES_LONG)(*(--(c))))<<16L; \
            case 6: l2|=((DES_LONG)(*(--(c))))<< 8L; \
            case 5: l2|=((DES_LONG)(*(--(c))));     \
            case 4: l1 =((DES_LONG)(*(--(c))))<<24L; \
            case 3: l1|=((DES_LONG)(*(--(c))))<<16L; \
            case 2: l1|=((DES_LONG)(*(--(c))))<< 8L; \
            case 1: l1|=((DES_LONG)(*(--(c))));     \
                } \
            }

#define DES_l2c(l,c)    (*((c)++)=_itobyte(l), \
             *((c)++)=_itobyte((l)>> 8L), \
             *((c)++)=_itobyte((l)>>16L), \
             *((c)++)=_itobyte((l)>>24L))

#define DES_n2l(c,l)    (l =((DES_LONG)(*((c)++)))<<24L, \
             l|=((DES_LONG)(*((c)++)))<<16L, \
             l|=((DES_LONG)(*((c)++)))<< 8L, \
             l|=((DES_LONG)(*((c)++))))

#define DES_l2n(l,c)    (*((c)++)=_itobyte((l)>>24L), \
             *((c)++)=_itobyte((l)>>16L), \
             *((c)++)=_itobyte((l)>> 8L), \
             *((c)++)=_itobyte((l)     ))

#define DES_l2cn(l1,l2,c,n) { \
            c+=n; \
            switch (n) { \
            case 8: *(--(c))=_itobyte((l2)>>24L); \
            case 7: *(--(c))=_itobyte((l2)>>16L); \
            case 6: *(--(c))=_itobyte((l2)>> 8L); \
            case 5: *(--(c))=_itobyte((l2)     ); \
            case 4: *(--(c))=_itobyte((l1)>>24L); \
            case 3: *(--(c))=_itobyte((l1)>>16L); \
            case 2: *(--(c))=_itobyte((l1)>> 8L); \
            case 1: *(--(c))=_itobyte((l1)     ); \
                } \
            }

#define DES_D_ENCRYPT(L,R,S)    \
    U.l=R^s[S+1]; \
    T.s[0]=((U.s[0]>>4)|(U.s[1]<<12))&0x3f3f; \
    T.s[1]=((U.s[1]>>4)|(U.s[0]<<12))&0x3f3f; \
    U.l=(R^s[S  ])&0x3f3f3f3fL; \
    L^= des_SPtrans[1][(T.c[0])]| \
        des_SPtrans[3][(T.c[1])]| \
        des_SPtrans[5][(T.c[2])]| \
        des_SPtrans[7][(T.c[3])]| \
        des_SPtrans[0][(U.c[0])]| \
        des_SPtrans[2][(U.c[1])]| \
        des_SPtrans[4][(U.c[2])]| \
        des_SPtrans[6][(U.c[3])];

#define DES_IP(l,r) \
{ \
    DES_LONG tt; \
    DES_PERM_OP(r,l,tt, 4,0x0f0f0f0fL); \
    DES_PERM_OP(l,r,tt,16,0x0000ffffL); \
    DES_PERM_OP(r,l,tt, 2,0x33333333L); \
    DES_PERM_OP(l,r,tt, 8,0x00ff00ffL); \
    DES_PERM_OP(r,l,tt, 1,0x55555555L); \
}

#define DES_FP(l,r) \
{ \
    DES_LONG tt; \
    DES_PERM_OP(l,r,tt, 1,0x55555555L); \
    DES_PERM_OP(r,l,tt, 8,0x00ff00ffL); \
    DES_PERM_OP(l,r,tt, 2,0x33333333L); \
    DES_PERM_OP(r,l,tt,16,0x0000ffffL); \
    DES_PERM_OP(l,r,tt, 4,0x0f0f0f0fL); \
}
//--------------------------------------------------------- 
typedef byte des_cblock[8];
//--------------------------------------------------------- 
typedef struct des_ks_struct
{
    union
    {
        DES_LONG pad[2];
    } ks;
} des_key_schedule[16];
//--------------------------------------------------------- 
/// DES算法 
class DES_Provider : public ISecurityAlgorithm
{
protected:
    //----------------------------------------------------- 
    /// 密钥 
    byte _key[DES_BLOCK_SIZE];
    /// 向量 
    ByteBuilder _iv;
    //----------------------------------------------------- 
    static void des_set_key(des_cblock* key, des_key_schedule schedule)
    {
        DES_LONG c,d,t,s;
        byte *in;
        DES_LONG *k;
        int i;

        k = reinterpret_cast<DES_LONG*>(schedule);
        in = reinterpret_cast<byte*>(key);

        DES_c2l(in,c);
        DES_c2l(in,d);

        DES_PERM_OP (d,c,t,4,0x0f0f0f0fL);
        DES_HPERM_OP(c,t,-2,0xcccc0000L);
        DES_HPERM_OP(d,t,-2,0xcccc0000L);
        DES_PERM_OP (d,c,t,1,0x55555555L);
        DES_PERM_OP (c,d,t,8,0x00ff00ffL);
        DES_PERM_OP (d,c,t,1,0x55555555L);
        d = static_cast<DES_LONG>((((d&0x000000ffL)<<16L)| (d&0x0000ff00L)     |
             ((d&0x00ff0000L)>>16L)|((c&0xf0000000L)>>4L)));
        c&=0x0fffffffL;

        for(i = 0; i < 16; i++)
        {
            if(des_shifts2[i])
            {
                c = ((c >> 2L) | (c << 26L)); d = ((d >> 2L) | (d << 26L));
            }
            else
            {
                c = ((c >> 1L) | (c << 27L)); d = ((d >> 1L) | (d << 27L));
            }
            c &= 0x0fffffffL;
            d &= 0x0fffffffL;

            s = des_skb[0][(c)& 0x3f] |
                des_skb[1][((c >> 6) & 0x03) | ((c >> 7L) & 0x3c)] |
                des_skb[2][((c >> 13) & 0x0f) | ((c >> 14L) & 0x30)] |
                des_skb[3][((c >> 20) & 0x01) | ((c >> 21L) & 0x06) |
                ((c >> 22L) & 0x38)];
            t = des_skb[4][(d)& 0x3f] |
                des_skb[5][((d >> 7L) & 0x03) | ((d >> 8L) & 0x3c)] |
                des_skb[6][(d >> 15L) & 0x3f] |
                des_skb[7][((d >> 21L) & 0x0f) | ((d >> 22L) & 0x30)];

            /* table contained 0213 4657 */
            *(k++) = ((t << 16L) | (s & 0x0000ffffL)) & 0xffffffffL;
            s = ((s >> 16L) | (t & 0xffff0000L));

            s = (s << 4L) | (s >> 28L);
            *(k++) = s & 0xffffffffL;
        }
    }
    static void des_encrypt(DES_LONG* data, des_key_schedule ks, bool encrypt)
    {
        DES_LONG l,r,u;
        /* for XENIX delay */
        char delay_str [2];

        union fudge 
        {
            DES_LONG  l;
            unsigned short s[2];
            byte  c[4];
        } U,T;
        int i;
        DES_LONG *s;

        u=data[0];
        r=data[1];

        DES_IP(u,r);

        l=(r<<1)|(r>>31);
        r=(u<<1)|(u>>31);

        /* clear the top bits on machines with 8byte longs */
        l&=0xffffffffL;
        r&=0xffffffffL;

        s = reinterpret_cast<DES_LONG*>(ks);
        if (encrypt)
        {
            for(i = 0; i < 32; i += 4)
            {
                DES_D_ENCRYPT(l, r, i + 0); /*  1 */
                /* for XENIX delay */
                strcpy(delay_str, "");
                DES_D_ENCRYPT(r, l, i + 2); /*  2 */
            }
        }
        else
        {
            for(i = 30; i > 0; i -= 4)
            {
                DES_D_ENCRYPT(l, r, i - 0); /* 16 */
                /* for XENIX delay */
                strcpy(delay_str, "");
                DES_D_ENCRYPT(r, l, i - 2); /* 15 */
            }
        }
        l=(l>>1)|(l<<31);
        r=(r>>1)|(r<<31);
        /* clear the top bits on machines with 8byte longs */
        l&=0xffffffffL;
        r&=0xffffffffL;

        DES_FP(r,l);
        data[0]=l;
        data[1]=r;
    }
    static void des_ecb_encrypt(des_cblock* input, des_cblock* output, des_key_schedule ks, bool encrypt)
    {
        DES_LONG l;
        byte *in,*out;
        DES_LONG ll[2];

        in = reinterpret_cast<byte*>(input);
        out = reinterpret_cast<byte*>(output);
        DES_c2l(in,l); ll[0]=l;
        DES_c2l(in,l); ll[1]=l;
        des_encrypt(ll,ks,encrypt);
        l=ll[0]; DES_l2c(l,out);
        l=ll[1]; DES_l2c(l,out);
        l=ll[0]=ll[1]=0;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    DES_Provider() {}
    DES_Provider(const ByteArray& key_8, const ByteArray& cbc_iv_8 = "")
    {
        Init(key_8, cbc_iv_8);
    }
    //----------------------------------------------------- 
    /// 初始化密钥 
    void Init(const ByteArray& key_8, const ByteArray& cbc_iv_8 = "")
    {
        memset(_key, 0, sizeof(_key));
        memcpy(_key, key_8.GetBuffer(), _min(key_8.GetLength(), DES_BLOCK_SIZE));
        if(cbc_iv_8.GetLength() >= DES_BLOCK_SIZE)
            _iv = cbc_iv_8.SubArray(0, DES_BLOCK_SIZE);
    }
    /**
     * @brief Encrypt DES加密 
     * 
     * @param [in] data 需要加密的数据 
     * @param [in] dataLen 需要加密的数据长度(只取8的整数倍数据运算,多余的数据将被截掉) 
     * @param [in] dst 加密后的数据 
     * 
     * @return bool
     */
    virtual bool Encrypt(const ByteArray& data, ByteBuilder& dst)
    {
        if(_iv.IsEmpty())
            ECB_Encrypt(_key, data, dst);
        else
            CBC_Encrypt(_key, _iv, data, dst);
        return true;
    }
    /**
     * @brief Decrypt DEV解密 
     * 
     * @param [in] data 需要解密的数据
     * @param [in] dataLen 需要解密的数据长度(只取8的整数倍数据运算,多余的数据将被截掉) 
     * @param [in] dst 解密后的数据
     * 
     * @return bool
     */
    virtual bool Decrypt(const ByteArray& data, ByteBuilder& dst)
    {
        if(_iv.IsEmpty())
            ECB_Decrypt(_key, data, dst);
        else
            CBC_Decrypt(_key, _iv, data, dst);
        return true;
    }
    //----------------------------------------------------- 
    /**
     * @brief CBC_Encrypt CBC方式加密 
     * 
     * @param [in] key_8 8字节密钥 
     * @param [in] iv_8 8字节IV向量 
     * @param [in] data_8X 待加密数据
     * @param [in] dataLen 待加密的数据长度 
     * @param [in] dst_8X 加密后的数据 
     * 
     * @return size_t 加密的数据长度 
     */
    static size_t CBC_Encrypt(const ByteArray& key_8, const ByteArray& iv_8, const ByteArray& data_8X, ByteBuilder& dst_8X)
    {
        des_key_schedule sch;
        des_set_key(reinterpret_cast<des_cblock*>(const_cast<byte*>(key_8.GetBuffer())), sch);

        int times = static_cast<int>(data_8X.GetLength() / DES_BLOCK_SIZE);
        size_t count = 0;
        byte* input;
        des_cblock output;
        byte tmp[DES_BLOCK_SIZE];
        memcpy(tmp, iv_8.GetBuffer(), DES_BLOCK_SIZE);

        for(int i = 0; i < times; ++i)
        {
            input = const_cast<byte*>(data_8X.GetBuffer())+i*DES_BLOCK_SIZE;

            // 异或数据 
            for(int j = 0;j < DES_BLOCK_SIZE; ++j)
                tmp[j] ^= input[j];

            des_ecb_encrypt(reinterpret_cast<des_cblock*>(tmp), &output, &(sch[0]), true);
            dst_8X.Append(ByteArray(reinterpret_cast<const byte*>(&output), DES_BLOCK_SIZE));
            memcpy(tmp, output, DES_BLOCK_SIZE);

            count += DES_BLOCK_SIZE;
        }
        return count;
    }
    /**
     * @brief CBC_Decrypt CBC方式解密
     *
     * @param [in] key_8 8字节密钥
     * @param [in] iv_8 8字节IV向量
     * @param [in] data_8X 待解密数据
     * @param [in] dataLen 待解密的数据长度
     * @param [in] dst_8X 解密后的数据
     *
     * @return size_t 解密的数据长度
     */
    static size_t CBC_Decrypt(const ByteArray& key_8, const ByteArray& iv_8, const ByteArray& data_8X, ByteBuilder& dst_8X)
    {
        des_key_schedule sch;
        des_set_key(reinterpret_cast<des_cblock*>(const_cast<byte*>(key_8.GetBuffer())), sch);

        int times = static_cast<int>(data_8X.GetLength() / DES_BLOCK_SIZE);
        size_t count = 0;
        byte* input;
        des_cblock output;
        byte tmp[DES_BLOCK_SIZE];
        memcpy(tmp, iv_8.GetBuffer(), DES_BLOCK_SIZE);

        for(int i = 0; i < times; ++i)
        {
            input = const_cast<byte*>(data_8X.GetBuffer())+i*DES_BLOCK_SIZE;
            des_ecb_encrypt(reinterpret_cast<des_cblock*>(input), &output, &(sch[0]), false);
            // 异或还原数据 
            for(int j = 0;j < DES_BLOCK_SIZE; ++j)
                tmp[j] ^= output[j];
            dst_8X.Append(ByteArray(reinterpret_cast<const byte*>(&output), DES_BLOCK_SIZE));

            count += DES_BLOCK_SIZE;
        }
        return count;
    }
    //----------------------------------------------------- 
    /**
     * @brief ECB_Encrypt ECB方式加密
     * 
     * @param [in] key_8 8字节密钥 
     * @param [in] buff_8X 待加密数据 
     * @param [in] dataLen 待加密数据的长度 
     * @param [in] dst_8X 加密后的数据 
     * 
     * @return size_t 加密的数据长度 
     */
    static size_t ECB_Encrypt(const ByteArray& key_8, const ByteArray& buff_8X, ByteBuilder& dst_8X)
    {
        des_key_schedule sch;
        des_set_key(reinterpret_cast<des_cblock*>(const_cast<byte*>(key_8.GetBuffer())), sch);
        
        int times = static_cast<int>(buff_8X.GetLength() / DES_BLOCK_SIZE);
        size_t count = 0;
        byte tmpBuff[DES_BLOCK_SIZE] = {0};
        for(int i = 0;i < times; ++i)
        {
            des_ecb_encrypt(reinterpret_cast<des_cblock*>(const_cast<byte*>(buff_8X.GetBuffer(i*DES_BLOCK_SIZE))),
                reinterpret_cast<des_cblock*>(tmpBuff), &(sch[0]), true);

            dst_8X.Append(ByteArray(tmpBuff, DES_BLOCK_SIZE));
            count += DES_BLOCK_SIZE;
        }
        return count;
    }
    /**
     * @brief ECB_Encrypt ECB方式解密
     *
     * @param [in] key_8 8字节密钥
     * @param [in] buff_8X 待解密数据
     * @param [in] dataLen 待解密数据的长度
     * @param [in] dst_8X 解密后的数据
     *
     * @return size_t 解密的数据长度
     */
    static size_t ECB_Decrypt(const ByteArray& key_8, const ByteArray& buff_8X, ByteBuilder& dst_8X)
    {
        des_key_schedule sch;
        des_set_key(reinterpret_cast<des_cblock*>(const_cast<byte*>(key_8.GetBuffer())), sch);
        int times = static_cast<int>(buff_8X.GetLength() / DES_BLOCK_SIZE);
        size_t count = 0;
        byte tmpBuff[DES_BLOCK_SIZE] = { 0 };
        for(int i = 0;i < times; ++i)
        {
            des_ecb_encrypt(reinterpret_cast<des_cblock*>(const_cast<byte*>(buff_8X.GetBuffer(i*DES_BLOCK_SIZE))),
                reinterpret_cast<des_cblock*>(tmpBuff), &(sch[0]), false);
            dst_8X.Append(ByteArray(tmpBuff, DES_BLOCK_SIZE));
            count += DES_BLOCK_SIZE;
        }
        return count;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
#undef DES_PERM_OP
#undef DES_HPERM_OP
#undef DES_c2l
#undef DES_l2c
#undef DES_n2l
#undef DES_l2n
#undef DES_l2cn
#undef DES_D_ENCRYPT
#undef DES_IP
#undef DES_FP
//--------------------------------------------------------- 
} // namespace security
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_DES_PROVIDER_H_
//========================================================= 