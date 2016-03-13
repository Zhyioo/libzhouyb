//========================================================= 
/**@file ByteBuilder.h 
 * @brief 字节缓冲区封装 
 * 
 * - 功能:
 *  - 数据区长度管理
 *  - 缓冲区的自动分配
 *  - 数据的追加
 *  - 简化的赋值操作
 *  - 数据Format的格式化 
 * .
 * 
 * - 特性:
 *  - 内部使用byte[] 管理
 *  - 兼容C方式字符串,类似于CString 
 *  - 末尾自动追加两字节0x00字符串结束标记,这样可以支持Unicode字符串
 *  - 提供对byte数组基本的追加和删除操作
 *  - 其他功能通过ByteConvert和StringConvert扩展 
 *  - 数据中可以包含0x00,长度为有效字节长度,但是以C字符串输出会被截断 
 * .
 * 
 * @date 2011-10-17   11:11:20 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../container/allocator.h"
using zhou_yb::container::simple_alloc;
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//---------------------------------------------------------
/// 只读的缓冲区 
class ByteArray
{
protected:
    //----------------------------------------------------- 
    /// 数据长度
    size_t _len;
    /// 缓冲区起始地址
    byte* _buf;
    //----------------------------------------------------- 
public:
    //-----------------------------------------------------
    ByteArray(const byte* ptr = NULL, size_t len = 0)
    {
        if(len > 0 && ptr != NULL)
        {
            _len = len;
            _buf = const_cast<byte*>(ptr);
        }
        else
        {
            _buf = NULL;
            _len = 0;
        }
    }
    ByteArray(const char* str, size_t len = 0)
    {
        // 长度<1时自动重新计算长度 
        if(str != NULL)
        {
            _len = len > 0 ? len : strlen(str);
            _buf = reinterpret_cast<byte*>(const_cast<char*>(str));
        }
        else
        {
            _buf = NULL;
            _len = 0;
        }
    }
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 取数据成员
     */
    /// 返回有效数据的长度 
    inline size_t GetLength() const { return _len; }
    /// 返回数据是否为空
    inline bool IsEmpty() const { return _len == 0; }
    /**
     * @brief 获取数据
     * 
     * @param [in] offset [default:0] 获取的数据偏移量,长度超长则返回NULL  
     * @warning 如果定义:ByteArray buf; 则buf的缓冲区指针为NULL,GetBuffer()使用前需要谨慎处理
     */
    inline const byte* GetBuffer(size_t offset = 0) const
    {
    	return _buf + offset;
    }
    /// 获取C方式的字符串,如果为空则返回""  
    inline const char* GetString() const
    {
        if(IsEmpty()) return "";
        return reinterpret_cast<const char*>(_buf);
    }
    /// 截取索引 index 长度为 len 的子串 
    inline ByteArray SubArray(size_t index, size_t len = SIZE_EOF) const
    {
        // 分隔的数据起始位置超长 
        if(index >= _len)
            return ByteArray();
        // 从index开始有效数据的实际长度 
        size_t slen = _len - index;
        // 超长的数据将被截掉 
        slen = _min(len, slen);
        return ByteArray(_buf + index, slen);
    }
    /// 拷贝到字节数组,返回实际拷贝的长度  
    inline size_t ToArray(void* pArray) const
    {
        if(pArray == NULL) return 0;
        if(_buf != NULL && _len > 0)
        {
            memcpy(pArray, _buf, _len);
        }
        reinterpret_cast<byte*>(pArray)[_len] = 0;
        return _len;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 比较操作
     */
    //----------------------------------------------------- 
    /// 比较字节数据是否匹配 
    bool IsEqual(const ByteArray& obj) const
    {
        // 数据为空 
        if(obj.IsEmpty())
            return 0 == _len;
        // 长度不同 
        if(obj.GetLength() != _len)
            return false;
        // 指向的是同一个数组 
        if(obj.GetBuffer() == _buf)
            return true;

        return memcmp(_buf, obj.GetBuffer(), _len) == 0;
    }
    inline bool operator ==(const ByteArray& obj) const { return IsEqual(obj); }
    inline bool operator ==(const char* cstr_buf) const { return IsEqual(ByteArray(cstr_buf)); }
    inline bool operator !=(const ByteArray& obj) const { return !(this->operator ==(obj)); }
    inline bool operator !=(const char* cstr_buf) const { return !(this->operator ==(cstr_buf)); }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 取值操作
     * @warning 需要控制index范围符合要求
     */
    inline byte& operator[](size_t index) { return _buf[index]; }
    inline const byte& operator[](size_t index) const { return _buf[index]; }
    //@}
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
/**
 * @brief 字节缓冲区
 *
 * - 简单的Byte数组封装
 * - 提供对字节数字的: 长度管理,追加操作,赋值操作,缓冲区管理
 * - 内存分配逻辑：缓冲区不够的话自动分配当前大小2倍的缓冲区
 * 意味着如果不手动调用Resize(int size),缓冲区会越来越大
 */
class ByteBuilder : public ByteArray
{
protected:
    //-----------------------------------------------------
    /// 缓冲区大小
    size_t _maxlen;
    //-----------------------------------------------------
private:
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 内部字符串操作(直接操作私有变量)
     */
    /// 上调内存大小为8的整块数 
    inline size_t _round_up(size_t n, size_t align)
    {
        return (((n)+align-1) & ~(align-1));
    }
    /// 重置缓冲区大小为指定长度(数据未拷贝) 
    inline byte* _create_buf(size_t n)
    {
        // 预留一个wchar_t 0的空间(这样存储Unicode的数据时也能正常计算出长度)
        byte* p = simple_alloc<byte>::allocate(n);
        if(p) memset(p, 0, sizeof(wchar_t));
        return p;
    }
    /// 校验是否能够容纳len长度的空间,不够则重新分配,内存分配失败则报错  
    bool _assert_size(size_t len)
    {
        size_t newlen = _len + len;
        // 缓冲区足够 
        if(_maxlen >= newlen)
            return true;
        size_t alloclen = 0;
        // 已经有缓冲区则分配2倍大小的空间 
        if(_buf != NULL)
        {
            // 已经有缓冲区存在 
            newlen *= 2;
        }

        alloclen = _round_up(newlen + sizeof(wchar_t), 8);
        byte* p = _create_buf(alloclen);
        if(p == NULL) return false;
        
        // 拷贝旧数据和释放旧空间 
        if(_buf != NULL)
        {
            memcpy(p, _buf, _len);
            _free_buf();
        }
        _buf = p;
        _maxlen = alloclen - sizeof(wchar_t);

        return true;
    }
    /// 字符串连接 
    void _concat_buf(const byte* buf, size_t len)
    {
        // 缓冲区足够,只需要拷贝数据即可 
        memcpy(_buf + _len, buf, len);
        _len += len;
        memset(_buf + _len, 0, sizeof(wchar_t));
    }
    /// 释放缓冲区 
    inline void _free_buf()
    {
        if(NULL == _buf) return;
        simple_alloc<byte>::deallocate(_buf, _maxlen + sizeof(wchar_t));
        _buf = NULL;
    }
    //----------------------------------------------------- 
    /// 追加格式化过的字符串
    size_t _append_format_s(const char* format, va_list args, bool auto_malloc = true)
    {
        /* format为空,不需要格式化 */
        if(_strlen(format) < 1)
            return 0;
        size_t len = 0;
        if(auto_malloc)
        {
            /* 估计所需要的空间 */
            len = _get_format_length(format, args);;
        }
        if(_assert_size(len))
        {
            // 实际使用的空间 
            len = vsprintf(reinterpret_cast<char*>(_buf + _len), format, args);
            _len += len;
            memset(_buf + _len, 0, sizeof(wchar_t));
        }

        return len;
    }
    //----------------------------------------------------- 
    /// 计算sprintf_s所需要的缓冲区长度(以需要的最大值计算)
    static size_t _get_format_length(const char* pszFormat, va_list args)
    {
        va_list argList;
        va_copy(argList, args);

        /* 开始估计需要的空间 */
        size_t nMaxLen = 0;
        for(const char *psz = pszFormat; *psz != '\0'; ++psz)
        {
            if(*psz != '%' || *(++psz) == '%')
            {
                ++nMaxLen;
                continue;
            }

            size_t nItemLen = 0;
            size_t nWidth = 0;

            for(;*psz != '\0'; ++psz)
            {
                if(*psz == '#')
                {
                    nMaxLen += 2;
                }
                else if(*psz == '*')
                {
                    nWidth = va_arg(argList, int);
                }
                else if(*psz == '-' ||
                    *psz == '+' ||
                    *psz == '0' ||
                    *psz == ' ')
                {
                    ;
                }
                else
                {
                    break;
                }
            }

            if(nWidth == 0)
            {
                nWidth = atoi(psz);
                for(;*psz != '\0' && _is_digital(*psz); ++psz)
                {
                    ;
                }
            }

            int nPrecision = 0;
            if(*psz == '.')
            {
                ++psz;

                if(*psz == '*')
                {
                    nPrecision = va_arg(argList, int);
                    ++psz;
                }
                else
                {
                    nPrecision = atoi(psz);
                    for(; *psz != '\0' && _is_digital(*psz); ++psz)
                    {
                        ;
                    }
                }
            }
            switch(*psz)
            {
            case 'h':
            case 'l':
            case 'F':
            case 'N':
            case 'L':
                ++psz;
                break;
            }
            switch(*psz)
            {
            case 'c':
            case 'C':
                nItemLen = 2;
                va_arg(argList, int);
                break;
            case 's':
            case 'S':
                const char *pstrNextArg = va_arg(argList, char*);
                if(NULL == pstrNextArg)
                {
                    nItemLen = 6;// "(null)"
                }
                else 
                {
                    nItemLen = _strlen(pstrNextArg);
                    nItemLen = _max(1, nItemLen);
                }
                break;
            }
            if(nItemLen != 0)
            {
                nItemLen = _max(nItemLen, nWidth);
                if(nPrecision != 0)
                {
                    nItemLen = _min(nItemLen, static_cast<size_t>(nPrecision));
                }
            }
            else
            {
                switch(*psz)
                {
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                case 'X':
                case 'o':
                    va_arg(argList, int);
                    nItemLen = 32;
                    nItemLen = _max(nItemLen, nWidth + nPrecision);
                    break;
                case 'e':
                case 'f':
                case 'g':
                case 'G':
                    va_arg(argList, double);
                    nItemLen = 128;
                    nItemLen = _max(nItemLen, nWidth + nPrecision);
                    break;
                case 'p':
                    va_arg(argList, void*);
                    nItemLen = 32;
                    nItemLen = _max(nItemLen, nWidth + nPrecision);
                    break;
                case 'n':
                    va_arg(argList, int*);
                    break;
                default:
                    ;
                }
            }
            nMaxLen += nItemLen;
        }

        return nMaxLen;
    }
    //@}
    //-----------------------------------------------------
public:
    //----------------------------------------------------- 
    /**
      * @brief 分配dst的空间保证剩余有效空间为len大小
      *
      * @attention 如果dst空闲空间大小不足len则进行分配,否则不进行任何操作
      *
      * @param[out] dst 缓冲区
      * @param[in] len 最小空间大小
      * @retval size_t
      * @return 分配前的空间大小
      **/
    static size_t Malloc(ByteBuilder& dst, size_t len)
    {
        size_t lastMaxLen = dst.GetSize();
        // 剩余空间
        size_t lastlen = lastMaxLen - dst.GetLength();

        if(lastlen < len)
        {
            len += dst.GetLength();
            dst.Resize(len);
        }

        return lastMaxLen;
    }
    //----------------------------------------------------- 
    ByteBuilder(const char* cstr_buf)
        : ByteArray(), _maxlen(0)
    {
        Append(cstr_buf);
    }
    explicit ByteBuilder(size_t maxlen = 0)
        : ByteArray(), _maxlen(0)
    {
        _assert_size(maxlen);
    }
    ByteBuilder(const ByteArray& other)
        : ByteArray(), _maxlen(0)
    {
        Append(other);
    }
    ByteBuilder(const ByteBuilder& other)
        : ByteArray(), _maxlen(0)
    {
        Append(other);
    }
    virtual ~ByteBuilder()
    {
        _free_buf();
    }
    //-----------------------------------------------------
    /// 获取指定格式的数据转换为字符串所需要的空间(估计)
    static size_t GetFormatLength(const char* format, ...)
    {
        if(_strlen(format) < 1)
            return 0;

        va_list args;
        va_start(args, format);

        size_t len = _get_format_length(format, args);

        va_end(args);
        return len;
    }
    //----------------------------------------------------- 
    /// 返回缓冲区大小
    inline size_t GetSize() const { return _maxlen; }
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 追加数据
     * - 单个字节 多次追加(byte n)
     * - C方式字符串(c_str)
     * - 多个字节数据(byte* len)
     * - 字节数组(ByteBuilder)
     */
    ByteBuilder& Append(byte val, size_t len = 1)
    {
        if(len > 0 && _assert_size(len))
        {
            memset(_buf + _len, val, len);
            _len += len;
            memset(_buf + _len, 0, sizeof(wchar_t));
        }

        return (*this);
    }
    inline ByteBuilder& Append(const char* cstr_buf)
    {
        return Append(ByteArray(cstr_buf));
    }
    inline ByteBuilder& Append(const ByteArray& other)
    {
        // 没有数据需要拷贝
        if(!other.IsEmpty() && _assert_size(other.GetLength()))
        {
            _concat_buf(other.GetBuffer(), other.GetLength());
        }
        return (*this);
    }
    //@}
    //-----------------------------------------------------
    /**
     * @brief 删除数据前端len长度的数据 
     * @param [in] len [default:1] 需要剔除的数据长度 
     * @retval size_t
     * @return 实际删除的字节数 
     * @warning 如果len长度大于实际数据长度则相当于Clear() 
     */
    size_t RemoveFront(size_t len = 1)
    {
        if(len < 1)
            return 0;
        // 要删除的长度大于总长度
        if(len >= _len)
        {
            len = 0;
            Clear();
        }
        else
        {
            // 分段拷贝 
            size_t datalen = _len - len;
            size_t copylen = _min(len, datalen);
            for(size_t i = 0;i < datalen; i += copylen)
            {
                memcpy(_buf + i, _buf + len + i, copylen);
            }
            // 有剩余的长度 
            size_t tmplen = datalen % copylen;
            if(tmplen > 0)
                memcpy(_buf + datalen - tmplen, _buf + _len - tmplen, tmplen);
            _len = datalen;
        }
        if(_buf != NULL)
            memset(_buf + _len, 0, sizeof(wchar_t));

        return len;
    }
    /**
     * @brief 删除末尾len长度的数据
     * @param [in] len [default:1] 需要删除的数据长度 
     * @retval size_t
     * @return 实际删除的字节数
     * @warning 函数只设置有效数据长度标志位,如果len大与实际长度则相当于Clear()
     */
    size_t RemoveTail(size_t len = 1)
    {
        if(len < 1)
            return 0;
        // 要删除的长度大于总长度
        if(len >= _len)
        {
            len = _len;
            Clear();
        }
        else
        {
            _len -= len;
        }
        if(_buf != NULL)
            memset(_buf + _len, 0, sizeof(wchar_t));

        return len;
    }
    /// 重置缓冲区大小为指定长度(长度过小则截断数据)
    /**
     * @brief 重置缓冲区大小为指定长度
     * @param [in] size 需要重置的缓冲区大小 
     * @warning 长度过小则截断数据 
     */
    void Resize(size_t size)
    {
        size_t alloclen = _round_up(size + sizeof(wchar_t), 8);
        byte* p = _create_buf(alloclen);
        if(NULL == p)
            return ;
        // 有数据才拷贝 
        if(!IsEmpty())
        {
            size_t len = _min(_len, size);
            memcpy(p, _buf, len);
            _len = len;
        }
        _free_buf();
        _buf = p;
        _maxlen = alloclen - sizeof(wchar_t);
    }
    //----------------------------------------------------- 
    /**
     * @brief 清空数据
     * @retval void
     * @return 
     * @warning 只重置长度,缓冲区仍然有旧数据,可以通过GetBuffer获取 
     */
    inline void Clear()
    {
        _len = 0;
    }
    //----------------------------------------------------- 
    /**
     * @brief 交换两个对象的数据
     * @param [in] other 
     * @retval void
     * @return 
     * @attention 交换_buf指针 
     */
    inline void Swap(ByteBuilder& other)
    {
        byte* ptr = other._buf;
        size_t len = other._len;
        size_t maxlen = other._maxlen;

        other._buf = _buf;
        other._len = _len;
        other._maxlen = _maxlen;

        _buf = ptr;
        _len = len;
        _maxlen = maxlen;
    }
    //-----------------------------------------------------
    //@{
    /**@name 
     * @brief 格式化字符串
     * @return 实际追加的字节数
     * @warning 格式化函数计算出的空间为所需的最大空间，如:%d算
     * 为43个字符长度，但是数据可能用不了那么多，如:%d 2
     */
    /// 追加，空间不够时自动分配
    size_t Format(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        // 自动分配空间
        size_t tmplen = _append_format_s(format, args);

        va_end(args);

        return tmplen;
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 赋值操作
     */
    inline ByteBuilder& operator =(const char* cstr_buf)
    {
        Clear();
        return Append(cstr_buf);
    }
    inline ByteBuilder& operator =(const ByteArray& other)
    {
        Clear();
        return Append(other);
    }
    inline ByteBuilder& operator =(const ByteBuilder& other)
    {
        Clear();
        return Append(other);
    }
    inline ByteBuilder& operator +=(const ByteArray& other)
    {
        return Append(other);
    }
    inline ByteBuilder& operator +=(const ByteBuilder& other)
    {
        return Append(other);
    }
    inline ByteBuilder& operator +=(const char* cstr_buf)
    {
        return Append(cstr_buf);
    }
    inline ByteBuilder& operator +=(byte val)
    {
        return Append(val);
    }
    //@}
    //-----------------------------------------------------
};// class ByteBuilder
//---------------------------------------------------------
} // namespace base
} // namespace zhou_yb
//=========================================================
