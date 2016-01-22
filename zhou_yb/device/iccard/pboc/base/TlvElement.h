//========================================================= 
/**@file TlvElement.h 
 * @brief PBOC中标签的解析器 
 * 
 * PBOC 中定义的标签容器
 * 
 * @date 2014-10-17   17:18:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../../include/Base.h"
#include "../../../../include/Container.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace base {
//--------------------------------------------------------- 
/// 标签转换器 
class TlvConvert
{
protected:
    TlvConvert() {}
    //-----------------------------------------------------
    /* 标签头部解析 */
    /// 判断字节后5位是否为全1
    static inline bool _is_double_tag(byte b)
    {
        return (b & 0x1F) == 0x1F;
    }
    /// 判断第一位是否为1
    static inline bool _is_multi_len(byte b)
    {
        return (b & 0x80) == 0x80;
    }
    /// 获取双字节长度标签的表示长度,取后7位 
    static inline ushort _get_high_len(byte b)
    {
        return (b&0x7F);
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    /// 错误标签头标识 
    static const ushort ERROR_TAG_HEADER /* 0x00 */;
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 解析头部信息
     * @return 标签头部占用的字节数
     * @retval ERROR_TAG_HEADER 错误的标签头格式
     */
    static size_t TransHeader(const ByteArray& src, ushort& name)
    {
        if(src.IsEmpty())
            return ERROR_TAG_HEADER;

        /* 解析标签头 */
        size_t index = 0;
        name = 0;
        if(_is_double_tag(src[0]))
        {
            // 多字节标签头长度 > 2
            if(src.GetLength() < 2)
                return ERROR_TAG_HEADER;//错误的标签头
            name += src[index];
            name <<= BIT_OFFSET;
            // 双字节标签头的后一个标签值

            ++index;
        }
        name += src[index];

        return (name != ERROR_TAG_HEADER)?(index + 1):ERROR_TAG_HEADER;
    }
    inline static size_t TransHeader(const char* tagAscii, ushort& name)
    {
        return TransHeader(DevCommand::FromAscii(tagAscii), name);
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 解析长度信息
     * @return 标签长度域所占用的字节数
     * @retval ERROR_TAG_HEADER 错误的标签头格式
     */
    static size_t TransLength(const ByteArray& src, size_t& len)
    {
        if(src.IsEmpty())
            return ERROR_TAG_HEADER;
        /* 解析标签长度 */
        size_t index = 0;
        len = static_cast<size_t>(src[0]);
        // 多字节长度 
        if(_is_multi_len(src[0]))
        {
            // 多字节标签长度字节数 
            size_t _len = _get_high_len(src[0]);
            // _len + 1为标签长度域的长度 
            if((_len + 1) > src.GetLength())
                return ERROR_TAG_HEADER;

            len = static_cast<size_t>(src[++index]);// src[1]
            for(size_t i = 1;i < _len; ++i)
            {
                len <<= BIT_OFFSET;
                len += src[++index];
            }
        }
        // 返回标签长度域所占用的字节数
        return (index + 1);
    }
    inline static size_t TransLength(const char* tagAscii, size_t& len)
    {
        return TransLength(DevCommand::FromAscii(tagAscii), len);
    }
    //@}
    //-----------------------------------------------------
    /// 将标签头转化为字符串
    static size_t ToHeaderBytes(ushort name, ByteBuilder& tag)
    {
        size_t len = 0;
        byte* pTag = reinterpret_cast<byte*>(&name);
        // 多字节标签
        for(ushort i = sizeof(name);i > 0; --i)
        {
            if(pTag[i - 1] > 0)
            {
                tag += pTag[i - 1];
                ++len;
            }
        }
        return len;
    }
    /// 将标签长度转化为字符串 
    static size_t ToLengthBytes(size_t len, ByteBuilder& tag)
    {
        size_t alen = 0;
        // 单字节长度
        if(len <= 0x7F)
        {
            tag += _itobyte(len);
            ++alen;
            return alen;
        }

        byte* pTagLen = reinterpret_cast<byte*>(&len);
        // 先预留一个长度标签位置
        tag += static_cast<byte>(0x00);
        size_t taglenindex = tag.GetLength() - 1;

        for(short i = sizeof(len);i > 0; --i)
        {
            if(pTagLen[i - 1] == 0x00)
                continue;
            tag += pTagLen[i - 1];
            ++alen;
        }
        // 设置长度位的值
        tag[taglenindex] = static_cast<byte>(0x80 | _itobyte(tag.GetLength() - taglenindex - 1));

        return alen;
    }
    /// 将标签头转换为Ascii码格式字符串 
    static void ToHeaderAscii(ushort name, ByteBuilder& tagAscii)
    {
        ByteBuilder tmp(6);
        ToHeaderBytes(name, tmp);

        ByteConvert::ToAscii(tmp, tagAscii);
    }
    /// 将标签头转换为Ascii码格式字符串 
    static const char* ToHeaderAscii(ushort name)
    {
        static ByteBuilder headerAscii(8);
        headerAscii.Clear();

        ToHeaderAscii(name, headerAscii);

        return headerAscii.GetString();
    }
    /// 生成标签返回标签的总长度 
    static size_t MakeTLV(ushort header, const ByteArray& data, ByteBuilder& buff)
    {
        size_t len = ToHeaderBytes(header, buff);
        len += ToLengthBytes(data.GetLength(), buff);
        buff.Append(data);

        len += data.GetLength();

        return len;
    }
    //-----------------------------------------------------
    /// 返回该标签头部是否是结构标签
    static inline bool IsStructHeader(ushort name)
    {
        ushort tmp = name;
        // 双字节标签
        if(tmp >= 0xFF)
            tmp >>= BIT_OFFSET;
        return (tmp & 0x20) == 0x20;
    }
    //-----------------------------------------------------
    /// 返回标签头所占字节的个数
    static size_t GetHeaderByteLen(ushort name)
    {
        /* 标签部分 */
        if(name == ERROR_TAG_HEADER)
            return 0;
        // 头部至少占一个字节
        size_t size = 1;
        // 双字节标签
        if((name>>BIT_OFFSET) > 0)
            ++size;

        return size;
    }
    /// 返回标签长度所占字节的个数
    static size_t GetLengthByteLen(size_t len)
    {
        /* 长度部分 */
        size_t size = 0;
        size_t tmp = len;
        do 
        {
            ++size;
            tmp >>= BIT_OFFSET;
        } while (tmp > 0);
        // 如果大于BIT:0111 1111 (127) 则还有一个标识长度的字节 
        if(len > 0x7F)
            ++size;

        return size;
    }
    /// 返回整个标签头所占字节的个数
    static size_t GetTagLength(ushort name, size_t len)
    {
        size_t size = GetHeaderByteLen(name);
        size += GetLengthByteLen(len);

        return size;
    }
    //-----------------------------------------------------
};
//---------------------------------------------------------
/// 标签头信息，提供对标签头的一些基本操作 
class TlvHeader
{
protected:
    ushort _name;
public:
    //-----------------------------------------------------
    TlvHeader(const char* tagAscii = NULL) : _name(ERROR_TAG_HEADER)
    {
        TlvConvert::TransHeader(tagAscii, _name);
    }
    TlvHeader(ushort name) : _name(name) {}
    TlvHeader(const TlvHeader& other) : _name(other._name) {}
    //-----------------------------------------------------
    /// 错误标签头标识 
    static const ushort ERROR_TAG_HEADER;
    //-----------------------------------------------------
    inline operator ushort() const 
    {
        return _name;
    }
    //-----------------------------------------------------
    inline bool operator == (const TlvHeader& other) const
    {
        return (*this) == other._name;
    }
    inline bool operator == (const char* tagAscii) const
    {
        TlvHeader tag(tagAscii);
        ByteBuilder tmp(64);
        DevCommand::FromAscii(tagAscii, tmp);
        if(tmp.GetLength() != TlvConvert::GetHeaderByteLen(tag))
            return false;

        return (*this) == tag._name;
    }
    inline bool operator == (ushort name) const
    {
        return _name == name;
    }
    inline bool operator != (const TlvHeader& other) const
    {
        return !(this->operator ==(other));
    }
    inline bool operator != (const char* tagAscii) const
    {
        return !(this->operator ==(tagAscii));
    }
    inline bool operator != (ushort name) const
    {
        return !(this->operator ==(name));
    }
    //-----------------------------------------------------
    inline TlvHeader& operator = (ushort name)
    {
        _name = name;
        return (*this);
    }
    inline TlvHeader& operator = (const TlvHeader& other)
    {
        _name = other._name;
        return (*this);
    }
    inline TlvHeader& operator = (const char* tagAscii)
    {
        TlvConvert::TransHeader(tagAscii, _name);
        return (*this);
    }
    //----------------------------------------------------- 
};
//---------------------------------------------------------
/// TLV标签对象 
class TlvElement
{
public:
    //----------------------------------------------------- 
    /// 标签解析模式 
    enum TlvMode
    {
        /// 完整格式
        Normal,
        /// 只包含标签头
        HeaderOnly,
        /// 不包含数据域 
        NonValue
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 标签头 
    TlvHeader _header;
    /// 标签数据长度
    size_t _len;
    /// 标签数据指针 
    const byte* _position;
    /// 父标签 
    TlvElement* _parent;
    /// 子标签 
    shared_obj<list<TlvElement*> > _children;
    /// 当前位置 
    list<TlvElement*>::iterator _itrCurrent;
    /// 上一个选择的标签
    TlvHeader _lastSelect;
    //----------------------------------------------------- 
    /// 初始化 
    void _init()
    {
        _header = TlvHeader::ERROR_TAG_HEADER;
        _len = 0;
        _position = NULL;
        _itrCurrent = _children.obj().begin();
        _parent = this;
        _lastSelect = _header;
    }
    /// 拷贝
    void _Copy(const TlvElement& obj)
    {
        _header = obj._header;
        _position = obj._position;
        _children = obj._children;
        _parent = this;
        _len = obj._len;
        _itrCurrent = _children.obj().begin();
        _lastSelect = _header;
    }
    /// 解析标签 
    static size_t _TlvFromBytes(TlvElement& tag, const ByteArray& src, TlvMode mode)
    {
        ushort name = TlvHeader::ERROR_TAG_HEADER;
        size_t headerlen = TlvConvert::TransHeader(src, name);
        tag._header = name;

        // 数据头错误
        if(headerlen <= TlvHeader::ERROR_TAG_HEADER)
            return TlvHeader::ERROR_TAG_HEADER;
        // 只有标签头,不需要解析其余部分
        if(mode == HeaderOnly)
            return headerlen;

        size_t len = TlvConvert::TransLength(src.SubArray(headerlen), tag._len);
        size_t taglen = headerlen + len;
        // 标签没有数据
        if(len > TlvHeader::ERROR_TAG_HEADER)
        {
            // 不需要数据域
            if(mode == NonValue)
                return taglen;

            taglen += tag._len;
            // 需要数据域且数据域长度正确
            if(taglen <= src.GetLength())
            {
                // 设置数据
                tag._position = const_cast<byte*>(src.GetBuffer(headerlen + len));

                return taglen;
            }
        }

        tag._header = TlvHeader::ERROR_TAG_HEADER;
        tag._len = 0;
        tag._position = NULL;

        return TlvHeader::ERROR_TAG_HEADER;
    }
    /// 递归的在数据中创建标签树 
    static size_t _CreateChildren(TlvElement& root, TlvMode mode)
    {
        size_t count = 0;
        const TlvHeader& header = root._header;
        if(header != TlvHeader::ERROR_TAG_HEADER &&
            !TlvConvert::IsStructHeader(header))
        {
            return count;
        }

        size_t taglen = 0;
        size_t size = TlvHeader::ERROR_TAG_HEADER;
        /* 在数据域里面查找子标签 */
        // 标签合法（且数据域不为空）
        TlvElement tag;
        ByteArray subArray(root._position, root._len);
        do
        {
            if(subArray.IsEmpty())
                break;

            size = _TlvFromBytes(tag, subArray, mode);
            if(size > TlvHeader::ERROR_TAG_HEADER)
            {
                TlvElement* pChild = simple_alloc<TlvElement>::allocate();
                simple_alloc<TlvElement>::construct(pChild);

                ++count;
                taglen += size;

                pChild->_header = tag._header;
                pChild->_position = tag._position;
                pChild->_len = tag._len;
                pChild->_parent = &root;
                
                TlvElement* pElement = pChild;
                while(pElement != pElement->_parent)
                {
                    pElement->_parent->_children.obj().push_back(pChild);
                    pElement = pElement->_parent;
                }
                pChild->_itrCurrent = pChild->_parent->_children.obj().begin();
                // 子孙标签 
                count += _CreateChildren(*pChild, mode);

                subArray = subArray.SubArray(size);
            }
        } while(size > TlvHeader::ERROR_TAG_HEADER);

        return count;
    }
    /// 解析数据
    static size_t _Parse(TlvElement& root, const ByteArray& tag, TlvMode mode)
    {
        root._position = tag.GetBuffer();
        root._len = tag.GetLength();

        size_t count = _CreateChildren(root, mode);
        root._itrCurrent = root._children.obj().begin();
        root._lastSelect = root._header;

        return count;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    TlvElement()
    {
        _init();
    }
    TlvElement(const TlvElement& obj)
    {
        _Copy(obj);
    }
    virtual ~TlvElement()
    {
        Clear();
    }
    //----------------------------------------------------- 
    /// 返回一个标签的父标签
    static inline TlvElement Parent(const TlvElement& tlvElement)
    {
        return *(tlvElement._parent);
    }
    /// 返回一个标签的根标签
    static inline TlvElement Root(const TlvElement& tlvElement)
    {
        const TlvElement* parent = &tlvElement;
        while(parent != parent->_parent)
            parent = parent->_parent;

        return (*parent);
    }
    /// 获取根标签的解析模式 
    static TlvMode GetTlvMode(const TlvElement& root)
    {
        // 不是根标签
        if(root.IsEmpty())
            return Normal;
        if(root._len < 1)
            return HeaderOnly;
        if(root._position == NULL)
            return NonValue;
        return Normal;
    }
    /// 解析标签,返回根标签 
    static TlvElement Parse(const ByteArray& tag, TlvMode mode = Normal)
    {
        TlvElement root;
        _Parse(root, tag, mode);
        return root;
    }
    /// 解析标签
    static bool Parse(TlvElement& root, const ByteArray& tag)
    {
        TlvMode mode = GetTlvMode(root);
        _Parse(root, tag, mode);
        return true;
    }
    //----------------------------------------------------- 
    /// 选择下一个标签，如果和上次相同则是下一个标签，否则为第一个
    TlvElement Select(const TlvHeader& header)
    {
        // 空标签
        if(header == _header)
            return TlvElement();

        list<TlvElement*>::iterator itr = _itrCurrent;
        // 选择的标签和上一次标签相同,但已经没有后续标签 
        if(_lastSelect == header)
        {
            ++itr;
        }
        else
        {
            // 标签不同,需要重头开始选择 
            itr = _children.obj().begin();
            _lastSelect = header;
        }
        for(;itr != _children.obj().end(); ++itr)
        {
            if((*itr)->GetHeader() == header)
                break;
        }
        // 没有后续标签 
        if(itr == _children.obj().end())
            return TlvElement();
        _itrCurrent = itr;
        return *(*_itrCurrent);
    }
    /// 迭代选择后续的标签
    TlvElement SelectAfter(const TlvHeader& header)
    {
        // 空标签
        if(header == _header)
            return TlvElement();

        list<TlvElement*>::iterator itr = _itrCurrent;
        // 没有选择过任何标签 
        if(_lastSelect == _header)
        {
            itr = _children.obj().begin();
        }
        else
        {
            ++itr;
        }
        _lastSelect = header;
        for(;itr != _children.obj().end(); ++itr)
        {
            if((*itr)->GetHeader() == header)
                break;
        }
        // 没有后续标签 
        if(itr == _children.obj().end())
            return TlvElement();
        _itrCurrent = itr;
        return *(*_itrCurrent);
    }
    /// 定位到第一个标签
    inline void SelectRoot()
    {
        _itrCurrent = _children.obj().begin();
        _lastSelect = _header;
    }
    /// 移动标签到下一个结点
    TlvElement MoveNext()
    {
        if(_itrCurrent == _children.obj().end())
            return TlvElement();
        list<TlvElement*>::iterator itr = _itrCurrent;
        ++_itrCurrent;
        return *(*itr);
    }
    //----------------------------------------------------- 
    /// 拷贝构造函数
    TlvElement& operator = (const TlvElement& obj)
    {
        if(&obj != this)
        {
            _Copy(obj);
        }
        return (*this);
    }
    /// 返回标签头 
    inline TlvHeader GetHeader() const
    {
        return _header;
    }
    /// 返回数据域长度
    inline size_t GetLength() const
    {
        return _len;
    }
    /// 返回数据 
    inline size_t GetValue(ByteBuilder& data) const
    {
        ByteArray buff(_position, _len);
        if(buff.IsEmpty())
            return 0;

        data.Append(buff);
        return _len;
    }
    /// 设置值
    inline size_t SetValue(const byte* data)
    {
        // 不允许设置数据 
        if(_position == NULL)
            return 0;

        if(data == NULL)
            memset(const_cast<byte*>(_position), 0, _len);
        else
            memcpy(const_cast<byte*>(_position), data, _len);
        return _len;
    }
    /// 返回TLV数据 
    size_t GetTLV(ByteBuilder& data) const
    {
        size_t len = 0;
        len += TlvConvert::ToHeaderBytes(_header, data);
        len += TlvConvert::ToLengthBytes(_len, data);
        data.Append(ByteArray(_position, _len));

        len += _len;
        return len;
    }
    /// 返回子标签数
    inline size_t GetCount() const
    {
        return _children.obj().size();
    }
    /// 返回TLV对象是否为空 
    inline bool IsEmpty() const
    {
        return _header == TlvHeader::ERROR_TAG_HEADER && GetCount() < 1;
    }
    /// 清空
    void Clear()
    {
        /// 析构根标签 
        if(_parent == this && _children.ref_count() < 2)
        {
            for(_itrCurrent = _children.obj().begin();_itrCurrent != _children.obj().end(); ++_itrCurrent)
            {
                simple_alloc<TlvElement>::destroy(*_itrCurrent);
                simple_alloc<TlvElement>::deallocate(*_itrCurrent);
            }
            _children.obj().clear();
        }
        _children.reset();
        _init();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace base
} // namespace pboc 
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//=========================================================
