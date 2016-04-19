//========================================================= 
/**@file Convert.h 
 * @brief 转换类,提供一些基础的转换操作
 * @warning 转换目的地址为ByteBuilder类型的都是追加操作,之前的数据不受影响
 * 
 * 提供对字节缓冲区的转换、字符串的一些操作,
 * 字节缓冲区格式的转换,
 * 字符串的截取、剔除空格、删除等操作.
 * 
 * @date 2011-10-17   14:50:05 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CONVERT_H_
#define _LIBZHOUYB_CONVERT_H_
//--------------------------------------------------------- 
#include <string>
using std::string;

#include <wchar.h>
#include <time.h>
#include "ByteBuilder.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base {
    //--------------------------------------------------------- 
#ifndef NOUSING_STATIC_TRANSTABLE

    /// 字节到16进制Ascii码转换对照表 
    extern const char byte_hex_table[];
    /// 所有ASCII码字符小写形式 
    extern const byte char_lower_table[256];
    /// 所有ASCII码字符大写形式 
    extern const byte char_upper_table[256];
    /// Ascii码转BCD码表 
    extern const byte char_ascii_to_bcd_table[256];

#endif
//---------------------------------------------------------
/// 以16进制输出数据
template<class T>
string _hex(const T& obj, size_t len = sizeof(T))
{
    byte* pObj = reinterpret_cast<byte*>(const_cast<T*>(&obj));
    byte b = 0x00;
    ByteBuilder tmp(2 * len);

    for(size_t i = len;i > 0; --i)
    {
        b = pObj[i - 1] & 0x0F0;
        b >>= HALF_BIT_OFFSET;
        tmp += (b < 0x0A) ? (b + '0') : (b - 0x0A + 'A');

        b = pObj[i - 1] & 0x0F;
        tmp += (b < 0x0A) ? (b + '0') : (b - 0x0A + 'A');
    }

    return tmp.GetString();
}
//--------------------------------------------------------- 
/// 以16进制输出数字(0xNum)
template<class T>
string _hex_num(const T& num, size_t len = sizeof(T))
{
    string format = "0x";
    format += _hex(num, len);
    return format;
}
//--------------------------------------------------------- 
/// 以二进制位的方式输出 "00000000 11111111"
template<class T>
string _bit(const T& obj, size_t len = sizeof(T))
{
    byte* pObj = reinterpret_cast<byte*>(const_cast<T*>(&obj));
    ByteBuilder tmp((BIT_OFFSET + 1) * len);

    for(size_t i = len;i > 0; --i)
    {
        for(size_t j = 1;j <= static_cast<size_t>(BIT_OFFSET); ++j)
        {
            tmp += static_cast<byte>((pObj[i - 1] & (0x01 << (BIT_OFFSET - j))) == 0x00 ? '0' : '1');
        }
        tmp += static_cast<byte>(' ');
    }
    // 移除最后的空格  
    tmp.RemoveTail();

    return tmp.GetString();
}
//--------------------------------------------------------- 
/// 多字节转Unicode
class CharConverter
{
protected:
    ByteBuilder _str;
public:
    CharConverter() {}
    CharConverter(size_t buffsize) : _str(buffsize) {}

    inline const char* to_char(const wchar_t* wstr)
    {
        return to_char(wstr, _wcslen(wstr));
    }
    const char* to_char(const wchar_t* wstr, size_t len)
    {
        char* pDst = NULL;
        _str = "";
        if(len > 0)
        {
            _str.Clear();
            // 1Unicode 2byte = 3UTF8 3byte
            _str.Append(static_cast<byte>(0x00), 3 * len + 8);
            pDst = reinterpret_cast<char*>(const_cast<byte*>(_str.GetBuffer()));
            size_t cvtlen = wcstombs(pDst, wstr, len);
            if(cvtlen == SIZE_EOF)
                return "";
            // 实际长度和总长度 
            len = _str.GetLength();
            if(len > cvtlen)
                _str.RemoveTail(len - cvtlen);
        }
        return _strput(pDst);
    }
    inline const wchar_t* to_wchar(const char* str)
    {
        return to_wchar(str, _strlen(str));
    }
    const wchar_t* to_wchar(const char* str, size_t len)
    {
        wchar_t* pDst = NULL;
        _str = "";
        if(len > 0)
        {
            _str.Clear();
            // 1Unicode 2byte = 3UTF8 3byte
            _str.Append(static_cast<byte>(0x00), len*2);
            pDst = reinterpret_cast<wchar_t*>(const_cast<byte*>(_str.GetBuffer()));
            size_t cvtlen = mbstowcs(pDst, str, len);
            if(cvtlen == SIZE_EOF)
                return L"";
            // 实际长度和总长度 
            len = _str.GetLength();
            cvtlen *= 2;
            if(len > cvtlen)
                _str.RemoveTail(len - cvtlen);
        }
        return pDst == NULL ? L"" : pDst;
    }
#ifdef UNICODE
    inline const char_t* to_wchar(const wchar_t* str) { return str; }
    inline const char_t* to_wchar(const wchar_t* str, size_t) { return str; }

    inline const char_t* to_char_t(const wchar_t* str) { return str; }
    inline const char_t* to_char_t(const wchar_t* str, size_t) { return str; }

    const char_t* to_char_t(const char* str) { return to_wchar(str); }
    const char_t* to_char_t(const char* str, size_t len) { return to_wchar(str, len); }
#else
    inline const char_t* to_char(const char* str) { return str; }
    inline const char_t* to_char(const char* str, size_t) { return str; }

    inline const char_t* to_char_t(const wchar_t* str) { return to_char(str); }
    inline const char_t* to_char_t(const wchar_t* str, size_t len) { return to_char(str, len); }

    const char_t* to_char_t(const char* str) { return str; }
    const char_t* to_char_t(const char* str, size_t) { return str; }
#endif
};
//--------------------------------------------------------- 
/// 位转换  
class BitConvert
{
protected:
    //----------------------------------------------------- 
    /// 位标志 
    static byte BitFlag[BIT_OFFSET];
    /// 填充位标志 
    static byte BitFillFlag[BIT_OFFSET + 1];
    //----------------------------------------------------- 
    /// 全局的数据,用于处理空引用 
    static byte _Byte;
    /// 引用的对象 
    byte* _pByte;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 取特定的位(0或非0),索引范围为:[0,7] 
    static inline uint Get(byte bit, size_t index)
    {
        index %= BIT_OFFSET;
        return static_cast<uint>(bit & BitFlag[index]);
    }
    /// 设置特定的位(0或非0),索引范围为:[0,7] 
    static inline void Set(byte& bit, size_t index, uint val)
    {
        index %= BIT_OFFSET;
        if(val)
            bit |= BitFlag[index];
        else
            bit &= ~BitFlag[index];
    }
    /// 是否包含指定的掩码
    static inline bool IsMask(byte bit, byte mask)
    {
        return ((bit & mask) == mask);
    }
    /// 取左边N位 索引范围为:[0-8]
    static inline byte Left(byte bit, size_t count)
    {
        bit &= ~(BitFillFlag[static_cast<int>(count % (BIT_OFFSET + 1))]);
        return bit;
    }
    /// 取右边N位 索引范围为:[0-8]
    static inline byte Right(byte bit, size_t count)
    {
        bit &= BitFillFlag[static_cast<int>(count % (BIT_OFFSET + 1))];
        return bit;
    }
    /// 循环左移[0-7] 
    static inline byte LeftShift(byte bit, size_t count)
    {
        int c = static_cast<int>(count % BIT_OFFSET);
        if(c < 1) return bit;
        byte tmp = (bit << c) | (bit >> (BIT_OFFSET - c));
        return tmp;
    }
    /// 循环右移[0-7] 
    static inline byte RightShift(byte bit, size_t count)
    {
        int c = static_cast<int>(count % BIT_OFFSET);
        if(c < 1) return bit;
        byte tmp = (bit >> c) | (bit << (BIT_OFFSET - c));
        return tmp;
    }
    //----------------------------------------------------- 
    BitConvert() : _pByte(&_Byte) {}
    BitConvert(byte& bit) : _pByte(&bit) {}
    //----------------------------------------------------- 
    /// 取特定的位(0或非0) 
    inline uint get(size_t index) const
    {
        index %= BIT_OFFSET;
        return static_cast<uint>((*_pByte) & BitFlag[index]);
    }
    /// 设置特定的位(0或非0)
    inline void set(size_t index, uint val)
    {
        index %= BIT_OFFSET;
        if(val)
            (*_pByte) |= BitFlag[index];
        else
            (*_pByte) &= ~BitFlag[index];
    }
    //----------------------------------------------------- 
    /// 取特定的位(0或非0) 
    inline uint operator [](size_t index) const
    {
        return get(index);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/**
 * @brief 字节转换 
 */
class ByteConvert
{
protected:
    /// 只允许调用static函数
    ByteConvert(){}
public:
    //----------------------------------------------------- 
    /**
     * @brief 调整有效数据大小,超长则截断数据,不够则填充特定值  
     * @param [out] src 数据源 
     * @param [in] len 需要调整的长度
     * @param [in] isLeft [default:false] 数据是否左对齐 
     * @param [in] fill [default:0x00] 数据不足时填充的数据 
     * @return 0 表示长度刚好合适,其他:截断或填充的位数 
     */ 
    static size_t Fill(ByteBuilder& src, size_t len, bool isLeft = false, byte fill = 0x00)
    {
        size_t fillLen = 0;
        size_t i = 0;
        if(src.GetLength() == len)
            return fillLen;
        // 需要填充数据 
        if(src.GetLength() < len)
        {
            fillLen = len - src.GetLength();
            src.Append(fill, fillLen);
            // 右对齐需要移动数据 
            if(!isLeft)
            {
                // 数据后移 
                for(i = (len - 1);i >= fillLen; --i)
                    src[i] = src[i - fillLen];
                for(i = 0;i < fillLen; ++i)
                    src[i] = fill;
            }
        }
        else
        {
            fillLen = src.GetLength() - len;

            if(isLeft)
                src.RemoveTail(fillLen);
            else
                src.RemoveFront(fillLen);
        }
        return fillLen;
    }
    /// 取指定字节数目的随机数 
    static void Random(ByteBuilder& dst, size_t count)
    {
        time_t tRand;
        time(&tRand);
        srand(static_cast<uint>(tRand));

        while(count-- > 0)
        {
            dst += _itobyte(rand() % 256);
        }
    }
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 单个字节 转 Ascii码
     *
     * 将单个字节byte转换成Ascii的数据(2倍长度)
     *
     * @param [in] bit 要转换的字节
     * @param [out] dst 转换后存放的缓冲区地址
     */
#ifdef NOUSING_STATIC_TRANSTABLE
    /// 将单个字节转换为16进制形式的字符串 
    static void ToAscii(byte bit, ByteBuilder& dst)
    {
        byte tmp = 0;
        for (int j = 0; j < 2; ++j)
        {
            if(j == 0)
                tmp = (bit & 0xf0) >> HALF_BIT_OFFSET;
            else
                tmp = bit & 0x0f;

            if (tmp <= 0x09)
                dst += tmp + '0';
            else 
            {
                tmp -= 0x0A;
                dst += tmp + 'A';
            }
        }
    }
#else
    /// 将单个字节转换为16进制形式的字符串 
    static void ToAscii(byte bit, ByteBuilder& dst)
    {
        dst += byte_hex_table[static_cast<int>((bit & 0x0F0) >> HALF_BIT_OFFSET)];
        dst += byte_hex_table[static_cast<int>(bit & 0x0F)];
    }
#endif
    //@}
    //-----------------------------------------------------
    /// 将对象以字节形式存储到缓冲区中
    template<class T>
    static void FromObject(const T& obj, ByteBuilder& dst)
    {
        byte* pTmp = reinterpret_cast<byte*>(const_cast<T*>(&obj));
        // 多字节标签
        for(ushort i = sizeof(obj);i > 0; --i)
        {
            dst += pTmp[i - 1];
        }
    }
    /// 将字节转换为对象 
    template<class T>
    static void ToObject(const ByteArray& src, T& obj)
    {
        if(src.GetLength() < sizeof(obj))
            return ;

        byte* pTmp = reinterpret_cast<byte*>(&obj);
        for(ushort i = sizeof(obj);i > 0; --i)
        {
            pTmp[i - 1] = src[sizeof(obj) - i];
        }
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 取校验和(异或)
     */ 
    static byte XorVal(const ByteArray& src)
    {
        byte xorVal = 0x00;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            xorVal ^= src[i];
        }

        return xorVal;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**
     * @brief 取或值  
     */ 
    static byte OrVal(const ByteArray& src)
    {
        byte xorVal = 0x00;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            xorVal |= src[i];
        }

        return xorVal;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 取于值 
     */ 
    static byte AndVal(const ByteArray& src)
    {
        byte xorVal = 0xFF;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            xorVal &= src[i];
        }

        return xorVal;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 计算异或值  
     */ 
    /// 计算异或值(将dst与src进行异或,异或的结果保存在dst中) 
    static void Xor(const ByteArray& src, ByteArray& dst)
    {
        if(src.GetLength() < 1)
            return;

        size_t dstOffset = 0;
        size_t srcOffset = 0;

        while(dstOffset < dst.GetLength())
        {
            if(srcOffset >= src.GetLength())
                srcOffset = 0;
            dst[dstOffset++] ^= src[srcOffset++];
        }
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 计算或值(将dst与src进行异或,异或的结果保存在dst中) 
     */ 
    static void Or(const ByteArray& src, ByteArray& dst)
    {
        if(src.GetLength() < 1)
            return;

        size_t dstOffset = 0;
        size_t srcOffset = 0;

        while(dstOffset < dst.GetLength())
        {
            if(srcOffset >= src.GetLength())
                srcOffset = 0;
            dst[dstOffset++] |= src[srcOffset++];
        }
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 计算于值(将dst与src进行异或,异或的结果保存在dst中) 
     */ 
    static void And(const ByteArray& src, ByteArray& dst)
    {
        if(src.GetLength() < 1)
            return;

        size_t dstOffset = 0;
        size_t srcOffset = 0;

        while(dstOffset < dst.GetLength())
        {
            if(srcOffset >= src.GetLength())
                srcOffset = 0;
            dst[dstOffset++] &= src[srcOffset++];
        }
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 对数据取反  
     */ 
    static void Not(ByteArray& buff)
    {
        for(size_t i = 0;i < buff.GetLength(); ++i)
        {
            buff[i] = ~buff[i];
        }
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**
     * @brief Expand 对字节进行扩展
     *
     * 将单个字节分开存储到两个字节中
     * 
     * @param [in] src 输入数据
     * @param [out] dst 输出 
     * @param [in] bit [default:0x00] 扩展的字节(只取高4位添加到新字节的高4位中) 
     * 
     * @return size_t 扩展的长度 
     */
    static size_t Expand(const ByteArray& src, ByteBuilder& dst, byte bit = 0x00)
    {
        byte tmp = 0x00;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            // 取高4位
            tmp = (src[i]>>HALF_BIT_OFFSET) & 0x0F;
            dst += static_cast<byte>(tmp | bit);
            // 取低4位 
            tmp = src[i] & 0x0F;
            dst += static_cast<byte>(tmp | bit);
        }

        return 2 * src.GetLength();
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 取字节低4位进行折叠  
     */ 
    static size_t Fold(const ByteArray& src, ByteBuilder& dst)
    {
        byte tmp = 0x00;
        size_t len = src.GetLength() / 2;
        size_t offset = dst.GetLength();

        for(size_t i = 0;i < len; ++i)
        {
            dst += static_cast<byte>(0x00);
            // 取前4位
            dst[offset + i] = static_cast<byte>((src[2 * i] << HALF_BIT_OFFSET) & 0x0F0);
            // 取后4位
            tmp = src[2*i+1];
            tmp &= 0x0F;

            dst[offset + i] |= tmp;
        }

        return len;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 字节数组 转 16进制字符串
     *
     * - 将字节缓冲区的数据转换为可显示的16进制数据
     * - 转换后字符间带空格
     * - "\x1B\x25\x42" => "1B 25 42"
     * - dst.len = 3*src.len - 1
     *
     * @return 转换后数据缓冲区的长度
     */
    static size_t ToHex(const ByteArray& src, ByteBuilder& dst)
    {
        // 未发生转换操作
        size_t srclen = src.GetLength();
        if(srclen < 1)
            return 0;

        size_t bufsize = (3*srclen - 1);
        ByteBuilder::Malloc(dst, bufsize);

        size_t i = 0;
        for(i = 0;i < srclen - 1; ++i)
        {
            ToAscii(src[i], dst);
            dst.Append(' ');
        }
        ToAscii(src[i], dst);

        return bufsize;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief BCD码 转 Ascii码
     *
     * - 将BCD码的数据转换为Ascii码以用于显示
     * - 转换后字符之间不带空格
     * - "\x1B\x25\x42" => "1B2542"
     * - dst.len = src.len * 2;
     *
     * @return 转换后的数据缓冲区长度
     */
    static size_t ToAscii(const ByteArray& src, ByteBuilder& dst)
    {
        size_t srclen = src.GetLength();
        size_t bufsize = 2*srclen;
        ByteBuilder::Malloc(dst, bufsize);

        for(size_t i = 0;i < srclen; ++i)
        {
            ToAscii(src[i], dst);
        }
        return bufsize;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief Ascii码 转 BCD码
     *
     * - 将Ascii码的数据转换为BCD码的数据以发送给设备
     * - 转换后字符之间不带空格
     * - "1B2542" => "\x1B\x25\x42"
     * - dst.len = src.len / 2
     *
     * @return 转换后的数据缓冲区长度
     */
#ifdef NOUSING_STATIC_TRANSTABLE
    static size_t FromAscii(const ByteArray& src, ByteBuilder& dst)
    {
        size_t dstlen = src.GetLength() / 2;
        ByteBuilder::Malloc(dst, dstlen);

        byte tmp;
        int index = 0;

        for(int i = 0;i < dstlen; ++i)
        {
            for(int j = 0;j < 2; ++j)
            {
                index  = 2*i + j;
                if(src[index] >= '0' && src[index] <= '9')
                    tmp = static_cast<byte>(src[index] - '0');
                else if(src[index] >= 'A' && src[index] <= 'F')
                    tmp = static_cast<byte>(src[index] - 'A' + 0x0A);
                else if(src[index] >= 'a' && src[index] <= 'f')
                    tmp = static_cast<byte>(src[index] - 'a' + 0x0A);
                else 
                    tmp = static_cast<byte>(0x00);

                if(j == 0)
                    dst += static_cast<byte>(tmp << HALF_BIT_OFFSET);
                else 
                    dst[dst.GetLength() - 1] |= tmp;
            }
        }

        return dstlen;
    }
#else
    static size_t FromAscii(const ByteArray& src, ByteBuilder& dst)
    {
        size_t dstlen = src.GetLength() / 2;
        ByteBuilder::Malloc(dst, dstlen);

        byte tmp;
        for(size_t i = 0;i < dstlen; ++i)
        {
            tmp = char_ascii_to_bcd_table[_itobyte(src[2*i])];
            tmp <<= HALF_BIT_OFFSET;
            tmp |= char_ascii_to_bcd_table[_itobyte(src[2*i+1])];

            dst += tmp;
        }

        return dstlen;
    }
#endif
    //@}
    //-----------------------------------------------------
};// class ByteConvert
//--------------------------------------------------------- 
/**
 * @brief 字符串转换
 *
 * 提供基于C方式ASCII字符串的一些基本操作
 */
class StringConvert
{
protected:
    StringConvert() {}
public:
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 字符串提取
     * @warning dst必须足够大以防止溢出
     * 
     * 从src中提取第index个分隔符(默认为'\0')的字符串，提取的字符串置于dst中
     *
     * @param [in] src 字符串起始地址  
     * @param [out] dst 获取到的子串
     * @param [in] index 需要获取的索引位置 
     * @param [in] flag [default:'\0']
     * @return 提取时拷贝的字符数
     * @retval size_t SIZE_EOF 索引index超出范围
     */
    static size_t Split(const ByteArray& src, ByteBuilder& dst, size_t index, char flag = '\0')
    {
        // 上一个 \n 的位置
        size_t lastIndex = 0;
        // \n 的个数
        size_t count = 0;
        size_t copycount = SIZE_EOF;
        size_t i = 0;
        for(i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] == flag)
            {
                if(index == count)
                {
                    copycount = i - lastIndex;
                    dst.Append(src.SubArray(lastIndex, copycount));
                    break;
                }

                lastIndex = i + 1;
                ++count;
            }
        }
        // 对于字符串中没有分隔符则将字符串本身分隔出去
        if(copycount == SIZE_EOF && src.GetLength() > 0)
        {
            copycount = i - lastIndex;
            dst.Append(src.SubArray(lastIndex, copycount));
        }

        return copycount;
    }
    //@}
    //-----------------------------------------------------
    /**
     * @brief 获取字符串中分隔符数目
     * @return 分隔符的个数
     * @retval 0 没有分隔符或src指针为NULL
     */
    static size_t GetSplitFlagCount(const ByteArray& src, char flag)
    {
        size_t count = 0;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] == flag)
                ++count;
        }
        return count;
    }
    //-----------------------------------------------------
    /**
     * @brief 返回指定字符是否包含在指定的字符串中 
     */
    static bool ContainsChar(const ByteArray& src, char flag, bool ignoreCase = false)
    {
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(Compare(src[i], flag, ignoreCase))
                return true;
        }
        return false;
    }
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 字符串比较  
     */ 
    /// 返回指定的字符串是否包含指定的子串 
    static bool Contains(const ByteArray& src, const ByteArray& substr, bool ignoreCase = false)
    {
        size_t srclen = src.GetLength();
        size_t substrlen = substr.GetLength();

        /* ""包含于任何字符串 */
        if(substrlen < 1)
            return true;
        /* ""不包含任何子字符串 */
        if(srclen < substrlen)
            return false;

        srclen -= substrlen;
        bool isContains = false;
        for(size_t i = 0;i <= srclen; ++i)
        {
            for(size_t j = 0;j < substrlen; ++j)
            {
                isContains = Compare(src[i + j], substr[j], ignoreCase);
                if(!isContains)
                    break;
            }
            // 匹配 
            if(isContains) return true;
        }
        return false;
    }
    /// 忽略大小写的字符比较函数
    static inline bool Compare(char c1, char c2, bool ignoreCase = false)
    {
        if(ignoreCase)
            return (_get_lower(c1) == _get_lower(c2));
        return c1 == c2;
    }
    /// 忽略大小写的字符串比较函数 
    static inline bool Compare(const ByteArray& str1, const ByteArray& str2, bool ignoreCase = false)
    {
        if(str1.GetLength() != str2.GetLength())
            return false;

        if(!ignoreCase)
            return memcmp(str1.GetBuffer(), str2.GetBuffer(), str1.GetLength()) == 0;

        for(size_t i = 0;i < str1.GetLength(); ++i)
        {
            if(_get_lower(str1[i]) != _get_lower(str2[i]))
                return false;
        }
        return true;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 查找起始位置
     * @return 标识字符在源字符串中的位置
     * @retval SIZE_EOF 源字符中没有标识字符
     */
    static size_t IndexOf(const ByteArray& src, char flag)
    {
        return IndexOfAny(src, ByteArray(&flag, 1));
    }
    static size_t IndexOf(const ByteArray& src, const ByteArray& substr, bool ignoreCase = false)
    {
        if(src.IsEmpty() || src.GetLength() < substr.GetLength())
            return SIZE_EOF;

        if(substr.GetLength() < 2)
            return IndexOfAny(src, substr);

        size_t index = 0;
        size_t subIndex = 0;
        size_t currentIndex = 0;
        ByteArray subFront = substr.SubArray(0, 1);
        for(;;)
        {
            subIndex = IndexOf(src.SubArray(index), subFront, ignoreCase);
            if(subIndex == SIZE_EOF)
                return SIZE_EOF;

            // 跳过IndexOf未匹配上的部分
            index += subIndex;
            // substr比src长
            if((src.GetLength() - index) < substr.GetLength())
                return SIZE_EOF;

            subIndex = 0;
            currentIndex = index;

            while(Compare(src[currentIndex], substr[subIndex], ignoreCase))
            {
                if(++subIndex >= substr.GetLength())
                    return index;
                if(++currentIndex >= src.GetLength())
                    return SIZE_EOF;
            }

            ++index;
        }

        return SIZE_EOF;
    }
    static size_t IndexOfNot(const ByteArray& src, char flag)
    {
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] != flag)
            {
                return i;
            }
        }
        return SIZE_EOF;
    }
    /// 查找第一个匹配的标识字符
    static size_t IndexOfAny(const ByteArray& src, const ByteArray& flagArr, bool ignoreCase = false)
    {
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            for(size_t j = 0;j < flagArr.GetLength(); ++j)
            {
                if(Compare(src[i], flagArr[j], ignoreCase))
                {
                    return i;
                }
            }
        }
        return SIZE_EOF;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 查找结束位置
     * @return 最后的标识字符在源字符串中的位置
     * @retval SIZE_EOF 源字符中没有标识字符
     */
    static size_t LastIndexOf(const ByteArray& src, char flag)
    {
        return LastIndexOfAny(src, ByteArray(&flag, 1));
    }
    static size_t LastIndexOf(const ByteArray& src, const ByteArray& substr, bool ignoreCase = false)
    {
        if(src.IsEmpty() || substr.IsEmpty())
            return SIZE_EOF;

        if(substr.GetLength() < 2)
            return LastIndexOfAny(src, substr);

        size_t index = src.GetLength();
        size_t subIndex = 0;
        size_t currentIndex = 0;
        ByteArray subTail = substr.SubArray(substr.GetLength() - 1, 1);
        for(;;)
        {
            currentIndex = LastIndexOf(src.SubArray(0, index), subTail, ignoreCase);
            if(currentIndex == SIZE_EOF || currentIndex < substr.GetLength())
                return SIZE_EOF;

            // 跳过LaseIndexOf未匹配上的长度
            index = currentIndex + 1;
            subIndex = substr.GetLength() - 1;
            while(Compare(src[currentIndex], substr[subIndex], ignoreCase))
            {
                if(subIndex-- < 1)
                    return currentIndex;
                if(currentIndex-- < 1)
                    return SIZE_EOF;
            }
            --index;
        }

        return SIZE_EOF;
    }
    static size_t LastIndexOfNot(const ByteArray& src, char flag)
    {
        if(src.GetLength() < 1)
            return SIZE_EOF;

        for(size_t i = src.GetLength();i > 0; --i)
        {
            if(src[i-1] != flag)
            {
                return i-1;
            }
        }
        return SIZE_EOF;
    }
    static size_t LastIndexOfAny(const ByteArray& src, const ByteArray& flagArr, bool ignoreCase = false)
    {
        if(src.GetLength() < 1)
            return SIZE_EOF;

        for(size_t i = src.GetLength();i > 0; --i)
        {
            for(size_t j = 0;j < flagArr.GetLength(); ++j)
            {
                if(Compare(src[i-1], flagArr[j], ignoreCase))
                {
                    return i-1;
                }
            }
        }
        return SIZE_EOF;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 截取字符串的左边len长度部分   
     */ 
    static size_t Left(const ByteArray& src, size_t len, ByteBuilder& dst)
    {
        // 需要截取的长度超长 
        if(src.GetLength() < len)
            len = src.GetLength();
        dst.Append(src.SubArray(0, len));
        return len;
    }
    static size_t Left(ByteBuilder& str, size_t len)
    {
        if(str.GetLength() > len)
        {
            str.RemoveTail(str.GetLength() - len);
        }
        return str.GetLength();
    }
    //@}
    //----------------------------------------------------- 
    /**
     * @brief 截取字符及中间的数据
     * 
     * @param [in] src 需要截取的数据
     * @param [in] left 左边的标记字符
     * @param [in] right 右边的标记字符
     * 
     * @return ByteArray 截取到的数据 
     */
    static ByteArray Middle(const ByteArray& src, byte left, byte right)
    {
        // 第一个字符必须是 '[' 开头
        size_t nItem = 0;
        size_t offset = 0;
        // 查找与之匹配的右中括号位置
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] == left)
            {
                if(nItem < 1)
                {
                    offset = i;
                }
                ++nItem;
                continue;
            }
            // 查找到与之匹配的数据
            if(src[i] == right)
            {
                --nItem;
                if(nItem < 1)
                    return src.SubArray(offset, i + 1);
            }
        }
        return ByteArray();
    }
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 截取字符串的右边len长度部分 
     */ 
    static size_t Right(const ByteArray& src, size_t len, ByteBuilder& dst)
    {
        // 长度超长 
        if(src.GetLength() <= len)
        {
            dst.Append(src);
            len = src.GetLength();
        }
        else
        {
            dst.Append(src.SubArray(src.GetLength() - len));
        }
        return len;
    }
    static size_t Right(ByteBuilder& str, size_t len)
    {
        if(str.GetLength() < len)
            return str.GetLength();

        str.RemoveFront(str.GetLength() - len);
        return len;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 获取字符所在从后往前倒数的位置(最后一个索引为0) 
     */ 
    static byte& LastCharAt(ByteArray& str, size_t index)
    {
        static byte _EOF = static_cast<byte>(EOF);
        size_t len = str.GetLength();
        if(len <= index)
            return _EOF;

        return str[len - index - 1];
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 返回字符串是否以指定的字符串结束
     * @warning 待比较的串必须比源串短
     */
    static bool EndWith(const ByteArray& src, const ByteArray& dst, bool ignoreCase = false)
    {
        size_t srclen = src.GetLength();
        size_t dstlen = dst.GetLength();
        if(srclen < dstlen)
            return false;

        for(size_t i = 1;i <= dstlen; ++i)
        {
            if(ignoreCase)
            {
                if(_get_lower(dst[dstlen - i]) != _get_lower(src[srclen - i]))
                    return false;
            }
            else
            {
                if(dst[dstlen - i] != src[srclen - i])
                    return false;
            }
        }

        return true;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 返回字符串是否以指定的字符串开始
     * @warning 待比较的串必须比源串短
     * @retval false 不匹配
     * @retval true 匹配
     */
    static bool StartWith(const ByteArray& src, const ByteArray& dst, bool ignoreCase = false)
    {
        size_t srclen = src.GetLength();
        size_t dstlen = dst.GetLength();

        if(srclen < dstlen)
            return false;

        for(size_t i = 0;i < dstlen; ++i)
        {
            if(ignoreCase)
            {
                if(_get_lower(dst[i]) != _get_lower(src[i]))
                    return false;
            }
            else
            {
                if(dst[i] != src[i])
                    return false;
            }
        }

        return true;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 将字符串转换为大写
     */
    static void ToUpper(const ByteArray& src, ByteBuilder& dst)
    {
        ByteBuilder::Malloc(dst, src.GetLength());
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            // 初始时在末尾分配额外的空间
            dst.Append(static_cast<byte>(_get_upper(src[i])));
        }
    }
    /// 将转换的结果保存到源字符串中
    static void ToUpper(ByteArray& src)
    {
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            // 初始时在末尾分配额外的空间
            src[i] = _get_upper(src[i]);
        }
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 将字符串转换为小写
     */
    static void ToLower(const ByteArray& src, ByteBuilder& dst)
    {
        ByteBuilder::Malloc(dst, src.GetLength());
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            // 初始时在末尾分配额外的空间
            dst.Append(static_cast<byte>(_get_lower(src[i])));
        }
    }
    /// 将转换的结果保存到源字符串中
    static void ToLower(ByteArray& src)
    {
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            // 初始时在末尾分配额外的空间
            src[i] = _get_lower(src[i]);
        }
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 字符替换
     *
     * 简单的字符替换 
     *
     * @return 替换的子串个数
     */
    static size_t Replace(ByteArray& src, char srcChar, char dstChar)
    {
        size_t spaceCount = 0;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] == srcChar)
            {
                ++spaceCount;
                src[i] = dstChar;
            }
        }

        return spaceCount;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 删除指定字符
     * @return 删除的字符个数
     */
    static size_t Remove(const ByteArray& src, ByteBuilder& dst, char srcChar)
    {
        size_t spaceCount = 0;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] == srcChar)
            {
                ++spaceCount;
                continue;
            }
            dst += src[i];
        }

        return spaceCount;
    }
    static size_t Remove(ByteBuilder& src, char srcChar)
    {
        size_t spaceCount = 0;
        for(size_t i = 0;i < src.GetLength(); ++i)
        {
            if(spaceCount > 0) src[i] = src[i + spaceCount];
            if(src[i] == srcChar)
            {
                ++spaceCount;
                src[i] = src[i + spaceCount];
            }
        }
        src.RemoveTail(spaceCount);
        return spaceCount;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 剔除字符串中的所有空格
     * @return 删除的字符个数
     */
    static size_t TrimAll(const ByteArray& src, ByteBuilder& dst)
    {
        return Remove(src, dst, ' ');
    }
    static size_t TrimAll(ByteBuilder& str)
    {
        return Remove(str, ' ');
    }
    //@}
    //---------------------------------------------------------
    //@{
    /**@name 
     * @brief 剔除字符串左边的空格
     */ 
    static size_t TrimLeft(ByteBuilder& src)
    {
        size_t index = IndexOfNot(src, ' ');
        if(index == SIZE_EOF)
            return 0;

        src.RemoveFront(index);
        return index + 1;
    }
    /// 剔除左边的空格
    static ByteArray TrimLeft(const ByteArray& src)
    {
        size_t index = IndexOfNot(src, ' ');
        if(index == SIZE_EOF)
            return src;
        return src.SubArray(index);
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name 
     * @brief 剔除字符串右边的空格
     */ 
    static size_t TrimRight(ByteBuilder& src)
    {
        size_t index = LastIndexOfNot(src, ' ');
        if(index == SIZE_EOF)
            return 0;

        size_t rmvlen = src.GetLength() - index - 1;
        src.RemoveTail(rmvlen);
        return rmvlen;
    }
    /// 剔除右边的空格
    static ByteArray TrimRight(const ByteArray& src)
    {
        size_t index = LastIndexOfNot(src, ' ');
        if(index == SIZE_EOF)
            return src;
        return src.SubArray(0, index + 1);
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name 
     * @brief 剔除字符串中前后的空格
     */ 
    static size_t Trim(ByteBuilder& src)
    {
        size_t rmvlen = TrimRight(src);
        rmvlen += TrimLeft(src);

        return rmvlen;
    }
    /// 剔除字符串中前后的空格
    static ByteArray Trim(const ByteArray& src)
    {
        return TrimLeft(TrimRight(src));
    }
    //@}
    //-----------------------------------------------------
};// class StringConvert
//---------------------------------------------------------
/**
 * @brief 设备指令转换 
 * 
 * 设备指令的一些转换操作
 */
class DevCommand
{
private:
    DevCommand(){}
    //-----------------------------------------------------
    /// 将Hex格式的字节转换为对应的数字表示
    static byte _trans_bit(byte bit)
    {
        byte tmp = 0x00;
        if(bit >= '0' && bit <= '9')
            tmp = static_cast<byte>(bit - '0');
        else if(bit >= 'A' && bit <= 'F')
            tmp = static_cast<byte>(bit - 'A' + 0x0A);
        else if(bit >= 'a' && bit <= 'f')
            tmp = static_cast<byte>(bit - 'a' + 0x0A);

        return tmp;
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    /// 将"\033 \x33 \X33" 转换为对应的字节数据
    static byte TransByte(const ByteArray& src)
    {
        byte bit = 0;
        if(src.GetLength() < 4)
            return bit;
        if(src[0] != '\\')
            return bit;
        // 8进制
        if(src[1] == '0')
        {
            bit = static_cast<byte>(src[2] - '0');
            bit <<= (HALF_BIT_OFFSET - 1);
            bit += static_cast<byte>(src[3] - '0');
        }
        // 16进制
        else if(_get_lower(src[1]) == 'x')
        {
            bit = _trans_bit(src[2]);
            bit <<= HALF_BIT_OFFSET;
            bit += _trans_bit(src[3]);
        }

        return bit;
    }
    /// 将Hex格式的字符串转换为byte,多余的将被截断
    static byte TransHex(const ByteArray& hex)
    {
        if(hex.GetLength() < 2)
            return 0;

        ByteBuilder tmp(2);
        ByteConvert::FromAscii(hex, tmp);
        tmp += static_cast<byte>(0x00);
        return tmp[0];
    }
    /**
     * @brief 解析配置格式的字符串  
     *
     * 字符串中的\x30 \030将按照16进制格式和8进制格式进行解析 
     */ 
    static size_t FromCfgArg(const ByteArray& src, ByteBuilder& dst)
    {
        size_t i = 0;
        size_t lastlen = dst.GetLength();

        for(i = 0;i < src.GetLength(); ++i)
        {
            if(src[i] == '\\')
            {
                dst += TransByte(src.SubArray(i, 4));
                i += 3;
            }
            else
            {
                dst += src[i];
            }
        }

        return dst.GetLength() - lastlen;
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief Hex码16进制的命令
     * 
     * 从大写的16进制字符串中解析(字符串中不带其他字符)
     */
    static size_t FromHex(const ByteArray& cmdHex, ByteBuilder& dst)
    {
        if(cmdHex.IsEmpty())
            return 0;

        // 所有字符都转换后为原来长度的一半
        return ByteConvert::FromAscii(cmdHex, dst);
    }
    static ByteBuilder FromHex(const ByteArray& cmdHex)
    {
        // 长度为0将忽略
        ByteBuilder cmd;
        FromHex(cmdHex, cmd);

        return cmd;
    }
    //@}
    //-----------------------------------------------------
    /**
     * @brief 字符串转16进制的命令
     * 
     * 支持从字符串中转换,字符串中可待空格,字母支持大小写
     */
    static ByteBuilder FromAscii(const ByteArray& cmdAscii)
    {
        // 长度为0将忽略
        ByteBuilder cmd;
        FromAscii(cmdAscii, cmd);

        return cmd;
    }
    static size_t FromAscii(const ByteArray& cmdAscii, ByteBuilder& dst)
    {
        if(cmdAscii.IsEmpty())
            return 0;
        ByteBuilder tmp;
        ByteBuilder::Malloc(tmp, cmdAscii.GetLength());
        StringConvert::TrimAll(cmdAscii, tmp);
        // 所有字符都转换后为原来长度的一半
        return ByteConvert::FromAscii(tmp, dst);
    }
    //@}
    //-----------------------------------------------------
};// DevCommand
//---------------------------------------------------------
} // namespace base
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CONVERT_H_
//=========================================================
