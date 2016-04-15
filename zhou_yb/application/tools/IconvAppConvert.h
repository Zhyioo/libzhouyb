//========================================================= 
/**@file IconvAppConvert.h
 * @brief Iconv应用层转换工具  
 * 
 * @date 2015-10-31   20:18:12
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICONVAPPCONVERT_H_
#define _LIBZHOUYB_ICONVAPPCONVERT_H_
//--------------------------------------------------------- 
#include "../../device/idcard/IDCardCharsetConvert.h"
using zhou_yb::device::idcard::IDCardCharsetConvert;

#include "../../extension/ability/IconvCharsetConvert.h"
using zhou_yb::extension::ability::IconvCharsetConvert;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace tools {
//--------------------------------------------------------- 
/// 二代证字符集转换器
class IDCardConvert
{
protected:
    IDCardConvert() {}
public:
    /// Unicode转UTF8编码 
    static size_t UnicodeToUTF8(const wchar_t* src, size_t srclen, ByteBuilder& dst)
    {
        size_t cvtLen = 0;
        ByteBuilder gbk(64);

        cvtLen = IDCardCharsetConvert::UnicodeToGBK(src, srclen, gbk);
        if(cvtLen != 0)
        {
            cvtLen = IconvCharsetConvert::GBKToUTF8(gbk.GetString(), gbk.GetLength(), dst);
        }

        return cvtLen;
    }
};
//--------------------------------------------------------- 
/// IC卡TLV数据格式转换 
class PbocTlvConvert
{
protected:
    PbocTlvConvert() {}
public:
    /// GBK编码转UTF8 
    static size_t GbkToUTF8(const char* gbk, size_t gbklen, ByteBuilder& dst)
    {
        IconvCharsetConvert::GBKToUTF8(gbk, gbklen, dst);

        size_t len = 0;
        for(size_t i = 0;i < gbklen; ++i)
        {
            // 是汉字，长度按照1计算
            if((gbk[i] & 0x80) == 0x80)
                ++i;
            ++len;
        }
        return len;
    }
};
//--------------------------------------------------------- 
} // namespace tools
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICONVAPPCONVERT_H_
//========================================================= 
