//========================================================= 
/**@file Base64_Provider.h 
 * @brief Base64算法 
 * 
 * @date 2014-10-25   14:23:48 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "security.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
#define FNO_ASN1_C                            100
#define DEB_base64_encode                       50
#define DEB_base64_decode                       51
extern const byte base64_table[];
//--------------------------------------------------------- 
/// Base64算法 
class Base64_Provider : public ISecurityAlgorithm
{
public:
    //----------------------------------------------------- 
    /// 加密 
    virtual bool Encrypt(const ByteArray& data, ByteBuilder& dst)
    {
        return Base64_Provider::Encode(data, dst);
    }
    /// 解密 
    virtual bool Decrypt(const ByteArray& data, ByteBuilder& dst)
    {
        return Base64_Provider::Decode(data, dst);
    }
    //----------------------------------------------------- 
    /// base64加密 
    static bool Encode(const ByteArray& buff, ByteBuilder& base64)
    {
        size_t i,k,blk_size,remain_size;
        byte *p, left[3];
        int fno;

        fno = FNO_ASN1_C + DEB_base64_encode;

        blk_size = buff.GetLength() / 3;
        remain_size = buff.GetLength() % 3;

        p = const_cast<byte*>(buff.GetBuffer());

        for(i = 0; i < blk_size; i++)
        {
            k = (p[0] & 0xFC) >> 2;
            base64 += base64_table[k];
            k = ((p[0] & 0x03) << 4) | (p[1] >> 4);
            base64 += base64_table[k];
            k = ((p[1] & 0x0F) << 2) | (p[2] >> 6);
            base64 += base64_table[k];
            k = p[2] & 0x3F;
            base64 += base64_table[k];
            p += 3;
        }

        switch(remain_size)
        {
        case 0:
            break;

        case 1:
            left[0] = p[0];
            left[1] = 0;
            p = left;

            k = (p[0] & 0xFC) >> 2;
            base64 += base64_table[k];
            k = ((p[0] & 0x03) << 4) | (p[1] >> 4);
            base64 += base64_table[k];

            base64 += static_cast<byte>('=');
            base64 += static_cast<byte>('=');
            break;

        case 2:
            left[0] = p[0];
            left[1] = p[1];
            left[2] = 0;
            p = left;

            k = (p[0] & 0xFC) >> 2;
            base64 += base64_table[k];
            k = ((p[0] & 0x03) << 4) | (p[1] >> 4);
            base64 += base64_table[k];
            k = ((p[1] & 0x0F) << 2) | (p[2] >> 6);
            base64 += base64_table[k];
            base64 += static_cast<byte>('=');
            break;

        default:
            break;
        }
        return true;
    }
    /// base64解密 
    static bool Decode(const ByteArray& base64, ByteBuilder& buff)
    {
        size_t i, j, k, m, n, l;
        byte four_bin[4];
        char four_char[4], c;
        int fno;

        fno = FNO_ASN1_C + DEB_base64_decode;

        j = base64.GetLength();
        i = 0;
        l = 0;

        for(;;)
        {
            if((i + 4) > j)
            {
                break;
            }

            for(k = 0; k < 4; k++)
            {
                if(i == j)
                {
                    break;
                }

                c = base64[i++];
                if((c == '+') || (c == '/') || (c == '=') ||
                    ((c >= '0') && (c <= '9')) ||
                    ((c >= 'A') && (c <= 'Z')) ||
                    ((c >= 'a') && (c <= 'z')))
                {
                    four_char[k] = c;
                }
            }

            if(k != 4)
            {
                return false;
            }

            n = 0;
            for(k = 0; k < 4; k++)
            {
                if(four_char[k] != '=')
                {
                    for(m = 0; m < 64; m++)
                    {
                        if(base64_table[m] == four_char[k])
                        {
                            four_bin[k] = static_cast<byte>(m);
                        }
                    }
                }
                else
                {
                    n++;
                }
            }

            switch(n)
            {
            case 0:
                buff += (four_bin[0] << 2) | (four_bin[1] >> 4);
                buff += (four_bin[1] << 4) | (four_bin[2] >> 2);
                buff += (four_bin[2] << 6) | four_bin[3];
                break;

            case 1:
                buff += (four_bin[0] << 2) | (four_bin[1] >> 4);
                buff += (four_bin[1] << 4) | (four_bin[2] >> 2);
                break;

            case 2:
                buff += (four_bin[0] << 2) | (four_bin[1] >> 4);
                break;

            default:
                break;
            }

            if(n != 0)
            {
                break;
            }
        }
        return true;
    }
    /// base64加密文件  
    static bool EncodeFile(const char* file, ByteBuilder& base64)
    {
        if(_is_empty_or_null(file))
            return false;

        size_t len;
        byte buff[256*3];
        FILE* fp = NULL;

        if((fp = fopen(file, "rb")) == NULL)
            return false;

        while((len = fread(buff, 1, 256*3, fp)) > 0)
            Encode(ByteArray(buff, len), base64);
        fclose(fp);

        return true;
    }
    /// base64解密到文件 
    static bool DecodeFile(const ByteArray& base64, const char* file)
    {
        if(_is_empty_or_null(file) || base64.IsEmpty())
            return false;

        FILE* fp = NULL;
        if((fp = fopen(file, "wb")) == NULL)
            return false;

        const uint BUFF_SIZE = 256;
        ByteBuilder buff(BUFF_SIZE);
        int times = static_cast<int>(base64.GetLength() / BUFF_SIZE);
        int i = 0;
        for(i = 0;i < times; ++i)
        {
            buff.Clear();
            Decode(base64.SubArray(BUFF_SIZE*i, BUFF_SIZE), buff);
            fwrite(buff.GetBuffer(), 1, buff.GetLength(), fp);
        }
        buff.Clear();
        Decode(base64.SubArray(BUFF_SIZE*i, base64.GetLength() % BUFF_SIZE), buff);
        fwrite(buff.GetBuffer(), 1, buff.GetLength(), fp);
        fclose(fp);

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace security 
} // namespace extension 
} // namespace zhou_yb
//========================================================= 