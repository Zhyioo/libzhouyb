//========================================================= 
/**@file IDCardCharsetConvert.h 
 * @brief 二代证UNICODE转GBK字符集 
 * 
 * @date 2012-06-23   12:17:21 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace idcard {
//--------------------------------------------------------- 
extern const wchar_t uniAUnicode[];
extern const size_t  uniAUnicodeLen;
extern const wchar_t uniAGbkcode[];
extern const size_t  uniAGbkcodeLen;
extern const wchar_t gbkAGbkcode[];
extern const size_t  gbkAGbkcodeLen;
extern const wchar_t gbkAUnicode[];
extern const size_t  gbkAUnicodeLen;
//--------------------------------------------------------- 
/// 身份证字库转换
class IDCardCharsetConvert
{
protected:
    IDCardCharsetConvert() {};
    //-----------------------------------------------------
    /// 二分查找 
    static size_t _binarySearch(const wchar_t *table, size_t tablen, wchar_t code)
    {
        size_t head = 0;
        size_t tail = tablen - 1;
        size_t middle = 0;

        if((code < table[head]) || (code > table[tail])) 
            return SIZE_EOF;

        while(head <= tail)
        {
            middle = (head + tail) / 2;
            if(code == table[middle]) 
            {
                return middle;
            } 
            else if( code > table[middle]) 
            {
                head = middle + 1;
            } 
            else 
            {
                tail = middle - 1;
            }
        }

        return SIZE_EOF;
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    /// Unicode转GBK 
    static size_t UnicodeToGBK(const wchar_t* unicode, size_t unicodeLen, ByteBuilder& gbk)
    {
        if(unicode == NULL || unicodeLen < 1)
            return 0;
        size_t index = 0;
        size_t translen = 0;
        wchar_t wChar = 0x03F3F;
        for(size_t i = 0;i < unicodeLen; ++i)
        {
            if((unicode[i]) < 0x80) 
            {
                gbk += _itobyte(unicode[i]);
                ++translen;
            } 
            else
            {
                index = _binarySearch(uniAUnicode, uniAUnicodeLen, unicode[i]);
                if(index == SIZE_EOF)
                    wChar = 0x03F3F;
                else
                    wChar = uniAGbkcode[index];
                gbk += _itobyte(wChar >> BIT_OFFSET);
                gbk += _itobyte(wChar);

                translen += 2;
            }
        }

        return translen;
    }
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
} // namespace idcard 
} // namespace device 
} // namespace zhou_yb
//=========================================================
