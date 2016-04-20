//========================================================= 
/**@file TlvEditor.h 
 * @brief IC卡TLV格式标签编辑器 
 * 
 * @date 2013-03-25   21:22:12 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TLVEDITOR_H_
#define _LIBZHOUYB_TLVEDITOR_H_
//--------------------------------------------------------- 
#include "TlvElement.h"
#include "../../../../container/property_tree.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace base {
//--------------------------------------------------------- 
/// IC卡标签结点 
struct TlvNode
{
    /// 标签头标识
    ushort header;
    /// 标签数据长度 
    size_t len;
    /// 标签数据(实际上只有最底层的标签才保存数据)  
    ByteBuilder data;
};
//--------------------------------------------------------- 
/**
 * @brief IC卡TAG标签编辑器 
 * 提供对标签的插入,删除操作 
 */ 
class TlvEditor
{
public:
    //----------------------------------------------------- 
    typedef TlvNode tag_type;
    typedef property_tree<TlvNode> tree_type;
    typedef property_tree<TlvNode>::iterator iterator;
    
    /// 结点插入模式 
    enum InsertMode
    {
        /// 作为跟标签 
        InsertRoot = 0,
        /// 作为父标签 
        InsertParent,
        /// 作为前面一个标签 
        InsertFront,
        /// 作为后续一个标签 
        InsertBack,
        /// 作为子标签 
        InsertChild
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// TLV结构树
    property_tree<TlvNode> _tree;
    //----------------------------------------------------- 
    /// 标签迭代回调函数 
    static bool ElementsPreviewCallBack(iterator itr, pointer parg)
    {
        list<iterator>* plist = reinterpret_cast<list<iterator>*>(parg);
        plist->push_back(itr);

        return true;
    }
    //----------------------------------------------------- 
    /// 解析单独的标签返回标签所占的长度,返回IcTag  
    static size_t _TlvFromBytes(TlvNode& tag, const ByteArray& src, bool needData = false)
    {
        size_t headerlen = TlvConvert::TransHeader(src, tag.header);

        // 数据头错误
        if(headerlen <= TlvHeader::ERROR_TAG_HEADER)
            return TlvHeader::ERROR_TAG_HEADER;

        size_t len = TlvConvert::TransLength(src.SubArray(headerlen), tag.len);
        size_t taglen = headerlen + len;
        // 标签没有数据
        if(len > TlvHeader::ERROR_TAG_HEADER)
        {
            taglen += tag.len;
            // 需要数据域且数据域长度正确
            if(taglen <= src.GetLength())
            {
                if(needData)
                {
                    tag.data = src.SubArray(headerlen + len, tag.len);
                }
                return taglen;
            }
        }

        return TlvHeader::ERROR_TAG_HEADER;
    }
    /// 返回一个iterator是否能够包含子结点 
    inline bool _canHasChildren(iterator itr)
    {
        return (itr == Root() || TlvConvert::IsStructHeader(itr->header));
    }
    /// 递归的在数据中解析标签(返回是否创建了子标签) 
    bool _CreateChildren(iterator root, const ByteArray& src)
    {
       // 插入该标签作为最后一个子标签 
       iterator itrNew = _tree.insert(root, TlvNode(), property_tree<TlvNode>::insert_child);
       size_t taglen = _TlvFromBytes((*itrNew), src);
       // 标签不合法 
       if(taglen == TlvHeader::ERROR_TAG_HEADER)
       {
           _tree.erase(itrNew);
           return false;
       }

        bool needData = true;
        size_t headerlen = TlvConvert::GetHeaderByteLen((*itrNew).header);
        headerlen += TlvConvert::GetLengthByteLen((*itrNew).len);

        ByteArray subsrc = src.SubArray(headerlen, taglen - headerlen);
        // 如果该标签为结构标签则继续在该标签中查找子标签 
        if(TlvConvert::IsStructHeader((*itrNew).header))
        {
            // 继续在数据域中查找子标签(创建失败则给结构标签附加数据) 
            needData = !(_CreateChildren(itrNew, subsrc));
        }
        // 本身就是数据标签或者为没有子标签的结构标签 
        if(needData)
        {
            // 解析结构标签时没有子标签,把该标签当数据标签处理(附加上数据)
            (*itrNew).data = subsrc;
        }
        // 查找兄弟标签 
        _CreateChildren(root, src.SubArray(taglen));

        return true;
    }
    // 获取标签新值和旧值所占用的字节长度差  
    int _get_offset_len(iterator itr, ushort header, size_t datalen)
    {
        int len = 0;
        // 跟结点不保存header和len,不需要计算 
        if(itr != Root())
        {
            // 获取新的header和旧的header之间所占字节的长度差 
            if(header != itr->header)
            {
                len += TlvConvert::GetHeaderByteLen(header) - 
                    TlvConvert::GetHeaderByteLen(itr->header);
            }
            // 获取新数据和旧数据所占用的字节长度差 
            len += (int)(TlvConvert::GetLengthByteLen(datalen) - 
                TlvConvert::GetHeaderByteLen(static_cast<ushort>(itr->len)));

            len += datalen - itr->len;
        }
        return len;
    }
    /// 更新父结点的标签长度 
    void _update_len(iterator itr, size_t lenOffset)
    {
        if(lenOffset == 0)
            return ;
        while(itr.parent() != itr)
        {
            itr = itr.parent();
            itr->len += lenOffset;
        }
    }
    /// 更新父结点的标签长度 
    void _erase_len(iterator itr, size_t lenOffset)
    {
        if(lenOffset == 0)
            return;
        while(itr.parent() != itr)
        {
            itr = itr.parent();
            itr->len -= lenOffset;
        }
    }
    /// 组装标签 
    size_t _pack_tag(iterator root, ByteBuilder& data, bool dataOnly = false)
    {
        size_t len = 0;
        if(!dataOnly)
        {
            len += TlvConvert::ToHeaderBytes(root->header, data);
            len += TlvConvert::ToLengthBytes(root->len, data);
        }
        // 结点为结构标签切有子结点 
        bool hasChildren = _canHasChildren(root);
        // 作为父结点不保存数据,有子结点才添加,否则为无效的结点 
        if(hasChildren && !property_tree_node_helper::is_bottommost(root.node))
        {
            iterator itr = root;
            itr.down();
            // 输出兄弟结点 
            do
            {
                len += _pack_tag(itr, data, false);
            } while(itr.next());
        }
        else
        {
            // 根标签不保存数据 
            if(root != Root())
            {
                data += root->data;
                len += root->data.GetLength();
            }
        }
        return len;
    }
    /// 处理结构标签(data为该标签的所有数据,header为结点标签新的head)
    void _trans_tag_data(iterator itr, const ByteArray& data)
    {
        bool needData = true;
        bool hasChildren = _canHasChildren(itr);

        Clear(itr);
        itr->data.Clear();
        if(hasChildren && !data.IsEmpty())
        {
            needData = !(_CreateChildren(itr, data)); 
        }
        /* 如果 head为结构标签但是解析失败,则把结构标签当数据标签处理 */
        if(needData)
        {
            itr->data = data;
        }
        itr->len = data.GetLength();
    }
    /// 查找第一个子标签 
    iterator _find_first(iterator root, ushort head)
    {
        if(head == TlvHeader::ERROR_TAG_HEADER || 
            (head == root->header && root != Root()))
        {
            return root;
        }
        iterator result = Root();
        // 在子标签中查找 
        if(_canHasChildren(root))
        {
            iterator tmp = root;
            if(tmp.down())
            {
                do
                {
                    result = _find_first(tmp, head);
                    if(result != Root())
                        break;
                }while(tmp.next());
            }
        }
        return result;
    }
    //----------------------------------------------------- 
public:
    /// 标签根结点 
    iterator Root()
    {
        return _tree.root();
    }
    /// 解析标签 
    bool Parse(const ByteArray& tagBuff)
    {
        bool bCreate = _CreateChildren(_tree.root(), tagBuff);
        return bCreate;
    }
    /// 插入标签 
    iterator Insert(iterator itr, TlvHeader head, const ByteArray& data, 
        InsertMode mode = InsertChild)
    {
        iterator insert = Root();
        iterator tmp;
        ushort nheader = head;
        size_t offsetlen = 0;
        size_t subtaglen = 0;
        switch(mode)
        {
        // 当作为父标签和根标签插入时,head必须为结构标签,数据域data忽略
        case InsertRoot:
            if(itr == Root())
            {
                if(IsEmpty())
                    return Insert(itr, head, data, InsertChild);
                else
                    itr.down();
            }
            if(!TlvConvert::IsStructHeader(nheader))
                break;
            // 获取当前标签相同层最左边的标签 
            while(itr.prev())
                ;
            // 获取当前标签同层标签的所有长度 
            tmp = itr;
            do
            {
                subtaglen += TlvConvert::GetHeaderByteLen(tmp->header);
                subtaglen += TlvConvert::GetLengthByteLen(tmp->len);
                subtaglen += tmp->len;
            } while(tmp.next());
            // 在当前结点的下方插入一个新的结点以替换值 insert_down
            insert = _tree.insert(itr, tag_type(), tree_type::insert_up);
            insert->header = nheader;
            insert->len = subtaglen;
            // 作为父标签一定没有数据域 
            insert->data.Clear();

            offsetlen = TlvConvert::GetHeaderByteLen(nheader);
            offsetlen += TlvConvert::GetLengthByteLen(subtaglen);

            _update_len(insert, offsetlen);
            break;
        // 当作为父标签和根标签插入时,head必须为结构标签,数据域data忽略
        case InsertParent:
            if(itr == Root())
            {
                if(IsEmpty())
                    insert = Insert(itr, head, data, InsertChild);
                else
                    insert = Insert(itr, head, data, InsertRoot);

                return insert;
            }
            if(TlvConvert::IsStructHeader(nheader))
            {
                // 获取当前标签的数据长度 
                subtaglen = itr->len;
                subtaglen += TlvConvert::GetHeaderByteLen(itr->header);
                subtaglen += TlvConvert::GetLengthByteLen(itr->len);
                // 在当前结点的下方插入一个新的结点以替换值 insert_down
                insert = _tree.insert(itr, tag_type(), tree_type::insert_down);
                insert->header = itr->header;
                insert->len = itr->len;
                insert->data = itr->data;
                
                itr->header = nheader;
                itr->len = subtaglen;
                // 作为父标签一定没有数据域 
                itr->data.Clear();

                offsetlen = TlvConvert::GetHeaderByteLen(nheader);
                offsetlen += TlvConvert::GetLengthByteLen(subtaglen);
                
                _update_len(itr, offsetlen);
            }
            break;
        // 当作为后续的标签插入时,parent必须为结构标签(同时解析数据域内包含的子标签)  
        case InsertBack:
            if(_canHasChildren(itr.parent()))
            {
                insert = _tree.insert(itr, tag_type(), tree_type::insert_back);
                insert->header = nheader;
                _trans_tag_data(insert, data);

                offsetlen = TlvConvert::GetTagLength(insert->header, insert->len) + data.GetLength();
                _update_len(insert, offsetlen);
            }
            break;
        // 当作为前面的标签插入时,parent必须为结构标签(同时解析数据域内包含的子标签)  
        case InsertFront:
            if(_canHasChildren(itr.parent()))
            {
                insert = _tree.insert(itr, tag_type(), tree_type::insert_front);
                insert->header = nheader;
                _trans_tag_data(insert, data);

                offsetlen = TlvConvert::GetTagLength(insert->header, insert->len) + data.GetLength();
                _update_len(insert, offsetlen);
            }
            break;
        // 当作为子标签插入时,itr必须为结构标签(同时解析数据域内包含的子标签) 
        case InsertChild:
            if(_canHasChildren(itr))
            {
                insert = _tree.insert(itr, tag_type(), tree_type::insert_child);
                insert->header = nheader;
                _trans_tag_data(insert, data);

                offsetlen = TlvConvert::GetTagLength(insert->header, insert->len) + data.GetLength();
                _update_len(insert, offsetlen);
            }
            break;
        default:
            break;
        }

        return insert;
    }
    /// 插入完整的整个标签,返回插入的标签,失败则返回Root() 
    iterator Insert(iterator itr, const ByteArray& tag, InsertMode mode = InsertChild)
    {
        size_t taglen = TlvHeader::ERROR_TAG_HEADER;
        tag_type _tag;
        taglen = _TlvFromBytes(_tag, tag, true);
        if(taglen != TlvHeader::ERROR_TAG_HEADER)
            return Insert(itr, _tag.header, _tag.data, mode);
        
        return Root();
    }
    /// 在子结点中获取第一个找到的标签(没有找到则返回Root,head为空标签表示获取第一个标签) 
    iterator Element(iterator root, TlvHeader head = TlvHeader::ERROR_TAG_HEADER)
    {
        return _find_first(root, head);
    }
    /// 在子结点中获取所有值为head的子标签(子结点优先,然后为孙结点,如果head为空标签,则迭代所有子标签)  
    size_t Elements(iterator root, TlvHeader head, list<iterator>& _list)
    {
        list<iterator> tmplist;
        _tree.front_preview(root, ElementsPreviewCallBack, &tmplist);

        size_t count = 0;
        list<iterator>::iterator tmpItr;
        for(tmpItr = tmplist.begin();tmpItr != tmplist.end(); ++tmpItr)
        {
            if(head == TlvHeader::ERROR_TAG_HEADER || head == (*tmpItr)->header)
            {
                _list.push_back(*tmpItr);
                ++count;
            }
        }
        return count;
    }
    /// 获取标签的数据(如果是结构标签则重新组装以生成) 
    void GetValue(iterator itr, ByteBuilder& data)
    {
        // 获取根标签的数据 
        _pack_tag(itr, data, true);
    }
    /// 获取标签的数据长度(主要用于获取Root标签的长度,其他标签可以直接itr->len获得) 
    size_t GetLength(iterator itr)
    {
        size_t len = 0;
        // 空标签(没有子结点)
        if(itr.down())
        {
            while(itr.next())
            {
                len += itr->len;
            }
        }
        return len;
    }
    /// 获取标签的全部数据 
    size_t GetTLV(iterator itr, ByteBuilder& tagData)
    {
        return _pack_tag(itr, tagData, itr == Root());
    }
    /// 更新数据结点标签的数据 
    void SetTLV(iterator itr, TlvHeader header, const ByteArray& tagData)
    {
        ushort nheader = header;
        int len = _get_offset_len(itr, nheader, tagData.GetLength());
        itr->header = nheader;
        // 更新数据域 
        _trans_tag_data(itr, tagData);
        // 更新父标签长度值 
        _update_len(itr, len);
    }
    /// 更新数据结点标签的数据(完整的标签),返回设置的标签长度  
    size_t SetTLV(iterator itr, const ByteArray& tag)
    {
        size_t taglen = TlvHeader::ERROR_TAG_HEADER;
        tag_type _tag;
        taglen = _TlvFromBytes(_tag, tag, true);
        if(taglen != TlvHeader::ERROR_TAG_HEADER)
        {
            SetTLV(itr, _tag.header, _tag.data);
        }
        
        return taglen;
    }
    /// 返回一个标签是否是带数据的标签 
    bool HasValue(iterator itr)
    {
        return property_tree_node_helper::is_bottommost(itr.node);
    }
    /// 删除标签 
    void Erase(iterator itr)
    {
        int lenOffset = 0;
        lenOffset += TlvConvert::GetHeaderByteLen(itr->header);
        lenOffset += TlvConvert::GetLengthByteLen(itr->len);
        lenOffset += itr->len;
        iterator tmp = itr;
        _erase_len(itr, lenOffset);
        _tree.erase(itr);
        
    }
    /// 清空所有标签 
    void Clear()
    {
        _tree.clear();
    }
    /// 清空一个指定的标签
    void Clear(iterator itr)
    {
        _tree.clear(itr.node);
    }
    /// 返回标签是否为空 
    bool IsEmpty()
    {
        return _tree.empty();
    }
    /// 返回标签的子标签数 
    size_t GetCount(iterator itr)
    {
        return _tree.count(itr);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace base
} // namespace pboc 
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TLVEDITOR_H_
//========================================================= 
