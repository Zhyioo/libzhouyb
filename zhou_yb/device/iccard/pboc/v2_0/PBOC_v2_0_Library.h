//========================================================= 
/**@file PBOC_v2_0_Library.h 
 * @brief PBOC IC卡相关数据结构定义及相关常规操作
 *
 * 提供关于标签的一些基本数据
 * 
 * @date 2014-10-17   17:35:16 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_V2_0_LIBRARY_H_
#define _LIBZHOUYB_PBOC_V2_0_LIBRARY_H_
//--------------------------------------------------------- 
#include "../base/TlvElement.h"

#include "PBOC_v2_0_TagMap.h"
using namespace zhou_yb::device::iccard::pboc::v2_0;
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
//---------------------------------------------------------
/**
 * @brief IC卡常用操作库
 *
 * 作为一个命名空间存在,这样可以使得后续不同版本的PBOC库能够扩展并使用 
 */
namespace PBOC_Library
{
    //-----------------------------------------------------
    /// GPO返回报文中AFL结构
    struct AFL
    {
        /// sfi [1,30]
        byte Sfi;
        /// 记录起始值
        byte Start;
        /// 记录结束值
        byte End;
        /// 用于验证的连续记录数
        byte Len;
    };
    //-----------------------------------------------------
    /// GPO返回报文数据
    struct GPO_Token
    {
        /// AIP标志 
        ByteBuilder AIP;
        /// AFL组 
        list<PBOC_Library::AFL> AFL_List;
    };
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 数据转换操作 
     */
    /**
     * @brief 将字符串标签转换为链表形式 
     * @param [in] tag "9F37 9F26"标签头组成的字节串  
     * @param [out] _list 获取到的标签列表 
     */ 
    static size_t MakeTagList(const ByteArray& taglist, list<TlvHeader>& _list)
    {
        size_t tagCount = 0;
        if(taglist.IsEmpty())
            return tagCount;

        TlvElement root = TlvElement::Parse(taglist, TlvElement::HeaderOnly);
        if(root.IsEmpty())
            return tagCount;

        TlvElement tagElement = root.MoveNext();
        while(!tagElement.IsEmpty())
        {
            _list.push_back(tagElement.GetHeader());
            ++tagCount;

            tagElement = root.MoveNext();
        }

        return tagCount;
    }
    /**
     * @brief 填充DOL域中的数据(只设置数据部分,其他部分不做修改)
     * @param [out] dst 存放数据的缓冲区 
     * @param [in] src dol数据的数据项 
     * @param [in] len 需要设置的长度 
     * @param [in] type 填充的数据类型 
     */ 
    static void PackDolData(ByteBuilder& dst, const ByteArray& src, 
        size_t len, DOLDataType type)
    {
        size_t tmplen = 0;
        // 数据长度太长则直接截断
        if(len <= src.GetLength())
        {
            // 如果是DOL_n则从左边开始截断数据
            if(type == DOL_n)
            {
                tmplen = src.GetLength() - len;
            }
            dst.Append(src.SubArray(tmplen, len));

            return;
        }
        tmplen = len - src.GetLength();
        // 进行补位
        switch(type)
        {
            // 数字,右对齐左补0x00
        case DOL_n:
            dst.Append(static_cast<byte>(0x00), tmplen);
            dst.Append(src);
            break;
            // 压缩数字,左对齐右补0xFF
        case DOL_cn:
            dst.Append(src);
            dst.Append(static_cast<byte>(0xFF), tmplen);
            break;
            // 结构标签,全部填0x00
        case DOL_struct:
            dst.Append(static_cast<byte>(0x00), len);
            break;
            // 其他类型,左对齐右补0x00
        case DOL_an:
        case DOL_ans:
        default:
            dst.Append(src);
            dst.Append(static_cast<byte>(0x00), tmplen);
            break;
        }
    }
    /**
     * @brief 构造PDOL数据(包括PDOL的数据域,返回添加的长度)
     * @param [in]  pdol pdol的标签头列表数据9F3603 9F7906
     * @param [in] srcElement 以解析过的用于组包pdol的完整数据
     * @param [out] dst 构造好的pdol数据输出缓冲区 
     * @return 返回组装后数据的全部长度 
     */ 
    static size_t PackPDOL(const ByteArray& pdol, TlvElement& srcElement, ByteBuilder& dst)
    {
        ByteBuilder tmpBuffer(16);
        TlvHeader header = TlvHeader::ERROR_TAG_HEADER;
        size_t len = 0;
        size_t totallen = 0;

        TlvElement root = TlvElement::Parse(pdol, TlvElement::NonValue);
        if(root.IsEmpty())
            return totallen;

        TlvElement tagElement = root.MoveNext();
        while(!tagElement.IsEmpty())
        {
            header = tagElement.GetHeader();
            len = tagElement.GetLength();

            // 非空，并且找到标签
            TlvElement subElement = srcElement.Select(header);
            if(!subElement.IsEmpty())
            {
                tmpBuffer.Clear();
                subElement.GetValue(tmpBuffer);
                PackDolData(dst, tmpBuffer, len, PBOC_v2_0_TagMap::GetType(header));
            }
            // 找不到标签直接填0x00
            else
            {
                dst.Append(static_cast<byte>(0x00), len);
            }
            totallen += len;

            tagElement = root.MoveNext();
        }

        return totallen;
    }
    /**
     * @brief 选择标签到指定的缓冲区中 
     * @param [in] taglist 需要选择的标签头列表 
     * @param [in] src 选择标签的数据源 
     * @param [out] buff 选择到的标签的输出缓冲区 
     * @param [out] pSelectList 已经选择到的标签 
     * @return 返回选择到的标签数目 
     */ 
    static size_t SelectTagToBuffer(const list<TlvHeader>& taglist, const ByteArray& src, 
        ByteBuilder& buff, list<TlvHeader>* pSelectList = NULL)
    {
        size_t count = 0;
        TlvElement root = TlvElement::Parse(src);
        if(root.IsEmpty())
            return count;

        list<TlvHeader>::const_iterator itr;
        for(itr = taglist.begin();itr != taglist.end(); ++itr)
        {
            TlvElement tagElement = root.Select(*itr);
            if(!tagElement.IsEmpty())
            {
                TlvConvert::ToHeaderBytes(*itr, buff);
                TlvConvert::ToLengthBytes(tagElement.GetLength(), buff);
                tagElement.GetValue(buff);
                ++count;

                // 记录选择到的标签 
                if(pSelectList != NULL)
                {
                    (*pSelectList).push_back(*itr);
                }
            }
        }
        return count;
    }
    /**
     * @brief 解析GPO命令返回的报文
     * @warning 解析后的AFL中已经将SFI转换为标准的格式(右移3位)
     * @param [in] gpoPkg 传入的GPO返回报文 
     * @param [out] token 解析出来的GPO结构 
     */
    static bool UnpackGPO_ReMessage(const ByteArray& gpoPkg, 
        PBOC_Library::GPO_Token& token)
    {
        size_t len = gpoPkg.GetLength();
        len %= 4;
        if(gpoPkg.IsEmpty() || len != 0 || gpoPkg[0] != 0x80)
            return false;

        // 复制AIP
        token.AIP.Clear();

        TlvElement root = TlvElement::Parse(gpoPkg);

        // 80模板 
        TlvElement tagElement = root.Select(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::ResponseMsgTemplateLatticeType_1));
        if(!tagElement.IsEmpty())
        {
            token.AIP.Append(gpoPkg[2]);
            token.AIP.Append(gpoPkg[3]);

            len = gpoPkg.GetLength() / 4;
            token.AFL_List.clear();
            for(size_t i = 1; i < len; ++i)
            {
                token.AFL_List.push_back();

                token.AFL_List.back().Sfi = static_cast<byte>(gpoPkg[4*i]>>3);
                token.AFL_List.back().Start = gpoPkg[4*i + 1];
                token.AFL_List.back().End = gpoPkg[4*i + 2];
                token.AFL_List.back().Len = gpoPkg[4*i + 3];
            }

            return true;
        }
        // 77模板 
        tagElement = root.Select(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::ResponseMsgTemplate2));
        if(!tagElement.IsEmpty())
        {
        }

        return false;
    }
    /**
     * @brief 将GPO数据转换为标签格式  
     */ 
    static size_t GPO_TokenToTag(const PBOC_Library::GPO_Token& token, ByteBuilder& tag)
    {
        size_t len = 0;
        // AIP 0x82
        len += TlvConvert::MakeTLV(
            PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::ApplicationInteractiveFeatures), token.AIP, tag);
        // AFL 0x94
        list<PBOC_Library::AFL>::const_iterator aflItr;
        for(aflItr = token.AFL_List.begin();aflItr != token.AFL_List.end(); ++aflItr)
        {
            len += TlvConvert::ToHeaderBytes(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::AFL), tag);
            // AFL的长度固定为4字节 
            len += TlvConvert::ToLengthBytes(4, tag);

            tag += aflItr->Sfi;
            tag += aflItr->Start;
            tag += aflItr->End;
            tag += aflItr->Len;

            len += 4;
        }

        return len;
    }
    /// 转换数据为ASCII码格式,返回转换后的数据长度  
    static size_t DolToString(const ByteArray& data, DOLDataType type, ByteBuilder& dataAscii)
    {
        size_t count = 0;
        if(data.IsEmpty())
            return count;
        switch(type)
        {
        case DOL_a:
        case DOL_an:
        case DOL_ans:
            count = strlen(data.GetString());
            dataAscii.Append(data.SubArray(0, count));
            break;
        case DOL_n:
        case DOL_cn:
            count = ByteConvert::ToAscii(data, dataAscii);
            while(count > 0 && dataAscii[dataAscii.GetLength() - 1] == 'F')
            {
                dataAscii.RemoveTail();
                --count;
            }
            break;
        default:
            count = ByteConvert::ToAscii(data, dataAscii);
            break;
        }

        return count;
    }
    //@}
    //-----------------------------------------------------
    /**
     * @brief 数据解析操作(从数据中提取指定的标签数据)
     * @param [in] src 要从中获取标签的数据源 
     * @param [in] header 需要获取的标签键值 biao
     * @param [out] pDst 获取出来的标签缓冲区(为NULL表示只判断是否包含标签不需要获取数据) 
     * @param [in] isFullTag [default:false] 是否获取完整的标签(标签头+长度+数据)
     */
    static bool GetTagValue(const ByteArray& src, 
        const TlvHeader& header, ByteBuilder* pDst, bool isFullTag = false)
    {
        TlvElement root = TlvElement::Parse(src);
        if(root.IsEmpty())
            return false;

        TlvElement tagElement = root.Select(header);
        bool contains = !tagElement.IsEmpty();
        if(contains && pDst != NULL)
        {
            if(isFullTag)
            {
                TlvConvert::ToHeaderBytes(header, *pDst);
                TlvConvert::ToLengthBytes(tagElement.GetLength(), *pDst);
            }
            tagElement.GetValue(*pDst);
        }

        return contains;
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 其他函数
     */
    /**
     * @brief 格式化DOL格式和数据以便输出
     * @date 2016-04-23 21:44
     * 
     * @param [in] format TL格式
     * @param [in] data 没有标签头的数据
     * @param [out] sFormat 分隔开的标签
     * @param [out] sData 分隔开的数据
     *
     *      9F7A01 9F0206       5F2A02
     * 8309 00     000000000000 0156
     * 
     * @return size_t 格式化的数据长度
     */
    static size_t FormatTLV(const ByteArray& format, const ByteArray& data, ByteBuilder& sFormat, ByteBuilder& sData)
    {
        TlvElement tagElement = TlvElement::Parse(format, TlvElement::NonValue);
        size_t len = 0;
        if(tagElement.IsEmpty())
            return len;

        // 计算格式需要的长度
        TlvElement subElement = tagElement.MoveNext();
        size_t tmplen = 0;
        size_t offset = 0;
        size_t datalen = 0;
        size_t headlen = 0;
        ByteBuilder tmp(8);
        while(!subElement.IsEmpty())
        {
            // 先输出数据
            tmplen = subElement.GetLength();
            datalen = ByteConvert::ToAscii(data.SubArray(offset, tmplen), sData);
            offset += tmplen;
            
            tmp.Clear();
            TlvConvert::ToHeaderBytes(subElement.GetHeader(), tmp);
            headlen = 2 * tmp.GetLength();
            ByteConvert::ToAscii(tmp, sFormat);
            sFormat += ':';
            ++headlen;
            tmp.Clear();
            TlvConvert::ToLengthBytes(subElement.GetLength(), tmp);
            headlen += 2 * tmp.GetLength();
            ByteConvert::ToAscii(tmp, sFormat);

            tmplen = _max(headlen, datalen) + 1;
            sFormat.Append(' ', tmplen - headlen);
            sData.Append(' ', tmplen - datalen);

            subElement = tagElement.MoveNext();
        }

        return len;
    }
    /**
     * @brief 格式化标签
     * @date 2016-04-23 22:18
     * 
     * @param [in] tag 需要格式化的标签头
     * @param [out] data 格式化后的数据
     * @param [in] splitChar [default:SPLIT_CHAR] 格式化的分隔符
     * 
     * @return size_t 格式化后的长度
     */
    static size_t FormatTAG(const ByteArray& tag, ByteBuilder& data, char splitChar = SPLIT_CHAR)
    {
        TlvElement tagElement = TlvElement::Parse(tag, TlvElement::HeaderOnly);
        TlvElement subElement = tagElement.MoveNext();
        ByteBuilder tmp(8);
        size_t len = 0;
        while(!subElement.IsEmpty())
        {
            tmp.Clear();
            TlvConvert::ToHeaderBytes(subElement.GetHeader(), tmp);
            len += ByteConvert::ToAscii(tmp, data);
            data += splitChar;
            ++len;

            subElement = tagElement.MoveNext();
        }
        return len;
    }
    /// 返回指定标签是否可以通过取数据命令获取 
    static bool IsGetDataTag(const TlvHeader& header)
    {
        size_t count = _pboc_v2_0_get_data_table_len;
        for(size_t i = 0;i < count; ++i)
        {
            if(header == _pboc_v2_0_get_data_table[i])
                return true;
        }
        return false;
    }
    /// 返回指定的SFI是否合法(是否在规范中的指定范围内) 
    static inline bool IsValidSFI(byte sfi)
    {
        // [1, 30]之间
        return (sfi > 0) && (sfi < 31);
    }
    //@}
    //-----------------------------------------------------
}
//---------------------------------------------------------
} // namespace pboc
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_V2_0_LIBRARY_H_
//=========================================================
