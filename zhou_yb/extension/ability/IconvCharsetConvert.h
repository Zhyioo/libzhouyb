//========================================================= 
/**@file IconvCharsetConvert.h 
 * @brief iconv库字符集转换
 * 
 * @date 2013-11-13 21:44:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICONVCHARSETCONVERT_H_
#define _LIBZHOUYB_ICONVCHARSETCONVERT_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
#include <iconv.h>
//--------------------------------------------------------- 
/// 字符集转换Linux iconv 
class IconvCharsetConvert
{
protected:
    IconvCharsetConvert() {}
public:
    //----------------------------------------------------- 
    /**
     * @brief 将数据从一种格式转换为另外一种格式 
     * @param [in] from 转换前的格式 
     * @param [in] to 转换后的格式
     * @param [in] memSize 需要预分配的缓冲区倍数(相对与srclen) 
     * @param [in] src 转换前的数据格式 
     * @param [in] srclen 数据长度 
     * @param [out] dst 转换后的数据 
     * @return 转换的数据长度(字节char)
     */
    static size_t Convert(const char* from, const char* to, size_t memSize, const char* src, size_t srclen, ByteBuilder& dst)
    {
        iconv_t cvt = iconv_open(to, from);
        if(cvt == ctype_cast(iconv_t)(-1))
            return 0;

        size_t cvtRet = 0;
        char* in = const_cast<char*>(src);
        size_t inlen = srclen;
        size_t outlen = memSize * srclen;
        size_t lastlen = dst.GetLength();
        // 预先追加数据
        dst.Append(static_cast<byte>(0x00), outlen);
        char* out = const_cast<char*>(dst.GetString() + lastlen);

        lastlen = outlen;
        cvtRet = iconv(cvt, &in, &inlen, &out, &outlen);
        dst.RemoveTail(outlen);
        iconv_close(cvt);

        if(cvtRet != SIZE_EOF)
            cvtRet = lastlen - outlen;

        return cvtRet;
    }
    //----------------------------------------------------- 
    /// UTF转GBK
    static size_t UTF8ToGBK(const char* src, size_t srclen, ByteBuilder& dst)
    {
        return Convert("UTF-8", "GBK", 1, src, srclen, dst);
    }
    /// GBK转UTF8
    static size_t GBKToUTF8(const char* src, size_t srclen, ByteBuilder& dst)
    {
        return Convert("GBK", "UTF-8", 2, src, srclen, dst);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICONVCHARSETCONVERT_H_
//=========================================================
