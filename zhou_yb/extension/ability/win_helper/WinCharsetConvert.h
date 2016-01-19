//========================================================= 
/**@file WinCharsetConvert.h 
 * @brief 字符集转换
 * 
 * @date 2013-11-13 21:44:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 字符集转换 Windows 
class WinCharsetConvert
{
protected:
    WinCharsetConvert() {}
public:
    //----------------------------------------------------- 
    /// 将Unicode数据转换为多字节数据 
    static size_t WideCharToMultiByteEx(UINT codePage, const wchar_t* src, size_t srclen, ByteBuilder& dst)
    {
        // 计算长度 
        size_t len = WideCharToMultiByte(codePage, 0, src, srclen, NULL, 0, NULL, NULL);
        size_t lastLen = dst.GetLength();
        dst.Append(static_cast<byte>(0x00), len);
        //转换 
        WideCharToMultiByte(codePage, 0, src, srclen, 
            reinterpret_cast<LPSTR>(const_cast<byte*>(dst.GetBuffer())+lastLen), len, NULL, NULL);  
        return len;
    }
    /// 将多字节数据转换为Unicode数据(注意:Unicode最后的两个 \0\0 不会补,由ByteBuilder补足) 
    static size_t MultiByteToWideCharEx(UINT codePage, const char* src, size_t srclen, ByteBuilder& dst)
    {
        // 获得需要的缓冲区长度 
        size_t len = MultiByteToWideChar(codePage, 0, src, srclen, NULL, 0);
        // Unicode每个wchar_t占两个字节 
        len *= 2;
        size_t lastLen = dst.GetLength();
        dst.Append(static_cast<byte>(0x00), len);
        // 转换为Unicode 
        MultiByteToWideChar(codePage, 0, src, srclen, 
            reinterpret_cast<LPWSTR>(const_cast<byte*>(dst.GetBuffer()) + lastLen), len/2);

        return len;
    }
    //----------------------------------------------------- 
    /// Unicode转UTF8
    static size_t UnicodeToUTF8(const wchar_t* src, size_t srclen, ByteBuilder& dst)
    {
        return WideCharToMultiByteEx(CP_UTF8, src, srclen, dst);
    }
    /// Unicode转UTF8
    static size_t UnicodeToUTF8(const ByteArray& src, ByteBuilder& dst)
    {
        if(src.GetLength() < 1)
            return 0;
        wchar_t* psrc = reinterpret_cast<wchar_t*>(const_cast<byte*>(src.GetBuffer()));
        return UnicodeToUTF8(psrc, wcslen(psrc), dst);
    }
    /// UTF8转Unicode
    static size_t UTF8ToUnicode(const char* src, size_t srclen, ByteBuilder& dst)
    {
        return MultiByteToWideCharEx(CP_UTF8, src, srclen, dst);
    }
    /// Unicode转系统默认编码 
    static size_t UnicodeToAscii(const wchar_t* src, size_t srclen, ByteBuilder& dst)
    {
        return WideCharToMultiByteEx(CP_ACP, src, srclen, dst);
    }
    /// Unicode转系统默认编码 
    static size_t UnicodeToAscii(const ByteArray& src, ByteBuilder& dst)
    {
        if(src.GetLength() < 1)
            return 0;
        wchar_t* psrc = reinterpret_cast<wchar_t*>(const_cast<byte*>(src.GetBuffer()));
        return UnicodeToAscii(psrc, wcslen(psrc), dst);
    }
    /// 系统默认编码转Unicode
    static size_t AsciiToUnicode(const char* src, size_t srclen, ByteBuilder& dst)
    {
        return MultiByteToWideCharEx(CP_ACP, src, srclen, dst);
    }
    /// 系统默认编码转UTF8 
    static size_t AsciiToUTF8(const char* src, size_t srclen, ByteBuilder& dst)
    {
        ByteBuilder tmp(64);
        AsciiToUnicode(src, srclen, tmp);

        return UnicodeToUTF8(tmp, dst);
    }
    /// UTF8转系统默认编码 
    static size_t UTF8ToAscii(const char* src, size_t srclen, ByteBuilder& dst)
    {
        ByteBuilder tmp(64);
        UTF8ToUnicode(src, srclen, tmp);

        return UnicodeToAscii(tmp, dst);
    }
    /// UTF8转系统默认编码 
    static size_t UTF8ToAscii(const ByteArray& src, ByteBuilder& dst)
    {
        if(src.GetLength() < 1)
            return 0;
        return UTF8ToAscii(src.GetString(), src.GetLength(), dst);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
//=========================================================
