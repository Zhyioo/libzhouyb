//========================================================= 
/**@file pboc_app.h 
 * @brief IC卡客户端上层应用逻辑实现
 *
 * 将IC卡的标签数据转换为ABCD这样的标签数据
 *
 * @date 2013-06-23   17:02:48 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_APP_H_
#define _LIBZHOUYB_PBOC_APP_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/Device.h"

#include <vector>
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace pboc {
//--------------------------------------------------------- 
#define TABLE_EOF 0x00
#define TABLE_L(i) (2*i)
#define TABLE_R(i) (2*i+1)
//--------------------------------------------------------- 
/// PBOC应用转换表 
class PBOC_TransTable
{
protected:
    PBOC_TransTable() {}
public:
    /// 默认的获取信息转换对照表
    static const ushort INFORMATION[];
    /// ARQC转换对照表 
    static const ushort AMOUNT[];
    /// 取记录转换对照表
    static const ushort DETAIL[];
};
//--------------------------------------------------------- 
/// TLV标签转换器 
struct ITlvConverter
{
    /**
     * @brief 将标签转换为数据 
     *
     * 一些默认的数据格式可能无法符合要求,如果提供了自定义的需求函数则根据函数转换后的数据进行拼装
     * 标签flag的格式和所提供的转换对照表一致(IC卡标签和外部的字符标签)
     * 
     * @param [in] tag 需要转换的标签 
     * @param [in] data 需要转换的数据源
     * @param [out] dst 转换后的数据
     * 
     * @return size_t 转换后数据的长度(组标签时的长度) 
     */
    virtual size_t Parse(ushort tag, const ByteArray& data, ByteBuilder& dst) = 0;
};
//--------------------------------------------------------- 
/**
 * @brief PBOC应用辅助类 
 */ 
struct PBOC_AppHelper
{
protected:
    PBOC_AppHelper() {}
public:
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief 表格的一些转换操作  
     */ 
    /// 获取表格的大小 
    static size_t getTableSize(const ushort table[])
    {
        size_t len = 0;
        if(NULL == table)
            return len;

        while(table[2*len] != TABLE_EOF && table[2*len+1] != TABLE_EOF)
            ++len;
        return len;
    }
    //----------------------------------------------------- 
    /**
     * @brief 表格的查找操作
     * @param [in] src 需要查找的标签
     * @param [in] table 查找的表格
     * @param [in] tablesize 表格大小 
     * @param [in] isReverse [default:false] 是否反向查找[A,B]则B查找A
     * @retval ushort
     * @return 
     */
    static ushort findInTable(ushort src, const ushort table[], size_t tablesize, bool isReverse = false)
    {
        size_t s = 0;
        size_t d = 0;
        for(size_t i = 0;i < tablesize; ++i)
        {
            if(!isReverse)
            {
                s = TABLE_L(i);
                d = TABLE_R(i);
            }
            else
            {
                s = TABLE_R(i);
                d = TABLE_L(i);
            }

            if(table[s] == src)
                return table[d];
        }
        return TABLE_EOF;
    }
    //----------------------------------------------------- 
    /**
     * @brief 将标签按照表格进行转换 
     * @param [in] srcTag 转换前的数据 
     * @param [in] table 转换的对照表 
     * @param [out] dstTag 转换后的标签数据(一系列标签头)
     */
    static size_t getTagHeader(const ByteArray& srcTag, const ushort table[], ByteBuilder& dstTag)
    {
        size_t tablesize = getTableSize(table);
        size_t transCount = 0;
        if(srcTag.IsEmpty() || tablesize < 1)
            return transCount;

        ushort tmp = TABLE_EOF;
        for(size_t i = 0;i < srcTag.GetLength(); ++i)
        {
            tmp = findInTable(static_cast<ushort>(srcTag[i]), table, tablesize);
            if(tmp == TABLE_EOF)
                continue;
            TlvConvert::ToHeaderBytes(tmp, dstTag);
            ++transCount;
        }

        return transCount;
    }
    //----------------------------------------------------- 
    /**
     * @brief 确保标签在列表中,如果没有则加上,返回是否追加了数据 
     */
    static bool assertTagHeader(ByteBuilder& dstTag, ushort srcTag)
    {
        TlvElement root = TlvElement::Parse(dstTag, TlvElement::HeaderOnly);
        TlvElement tagElement = root.Select(srcTag);
        if(tagElement.IsEmpty())
        {
            TlvConvert::ToHeaderBytes(srcTag, dstTag);
            return true;
        }
        return false;
    }
    /**
     * @brief 确保ASCII码格式的标签在列表中,如果没有则加上,返回是否追加了数据
     */
    static bool assertTagHeaderAscii(ByteBuilder& dstTag, const ByteArray& sTag)
    {
        TlvElement tagElement = TlvElement::Parse(dstTag, TlvElement::HeaderOnly);

        ByteBuilder srcTag(16);
        DevCommand::FromAscii(sTag, srcTag);

        TlvElement srcElement = TlvElement::Parse(srcTag, TlvElement::HeaderOnly);
        bool isAppend = false;
        TlvElement subElement = srcElement.MoveNext();
        while(!subElement.IsEmpty())
        {
            ushort header = subElement.GetHeader();
            subElement = tagElement.Select(header);
            if(subElement.IsEmpty())
            {
                TlvConvert::ToHeaderBytes(header, dstTag);
                isAppend = true;
            }

            subElement = srcElement.MoveNext();
        }
        return isAppend;
    }
    //----------------------------------------------------- 
    /**
     * @brief 获取标签的数据长度  
     * @param [in] src 字符串表示的长度 
     * @param [in] len 长度的字符数 
     */ 
    static size_t getLen(const char* src, size_t len)
    {
        char slen[32] = {0};
        memcpy(slen, src, len);

        return ArgConvert::FromString<size_t>(slen);
    }
    //----------------------------------------------------- 
    /**
     * @brief 获取指定的标签数据 
     * @param [in] info 数据
     * @param [in] lenbyte 长度字符的个数 
     * @param [in] tag 需要查找的标签 
     * @param [out] taglen 该标签的数据长度 
     * @return 标签在数据中的位置 
     */ 
    static const char* getTag(const ByteArray& info, size_t lenbyte, char tag, size_t* taglen)
    {
        const char* ptr = info.GetString();
        const char* infoEOF = ptr + info.GetLength() - 1;
        size_t len = 0;
    
        while(ptr <= infoEOF)
        {
            len = getLen(ptr + 1, lenbyte);
        
            if(tag == *ptr)
            {    
                if(taglen != NULL)
                {
                    *taglen = len;
                }
            
                return ptr;
            }
            ptr += 1;
            ptr += lenbyte;
            ptr += len;
        }
        return NULL;
    }
    //----------------------------------------------------- 
    /**
     * @brief 转换数据格式
     * 
     * @param [in] src src 转换的数据源(IC卡中的原始数据格式) 
     * @param [in] dst 转换后的数据 
     * @param [in] lenbyte 长度位的标识字符串长度
     * @param [in] table 转换对照表 
     * @param [in] tlvConverter 转换的数据格式解析回调函数 
     * @param [in] fillEmpty [default:false] 是否填充没有找到的标签 
     * 
     * @return size_t 转换的标签数  
     */
    static size_t transFromTLV(const ByteArray& src, ByteBuilder& dst, 
        size_t lenbyte, const ushort table[],
        ITlvConverter& tlvConverter, bool fillEmpty = false)
    {
        size_t transCount = 0;
        size_t tablesize = getTableSize(table);
        if(tablesize < 1)
            return transCount;

        TlvElement root = TlvElement::Parse(src);
        if(root.IsEmpty())
            return transCount;

        size_t cvtLen = 0;
        size_t offset = 0;

        ByteBuilder tmp(64);
        ByteBuilder sLenFormat(32);
        sLenFormat += "%0";
        sLenFormat.Format("%d", lenbyte);
        sLenFormat += "d";

        ushort header = TlvHeader::ERROR_TAG_HEADER;
        TlvElement tagElement;
        for(size_t i = 0;i < tablesize; ++i)
        {
            header = table[TABLE_R(i)];
            tmp.Clear();
            dst += static_cast<byte>(table[TABLE_L(i)]);
            // 预先占住长度位 
            offset = dst.GetLength();
            dst.Append(static_cast<byte>('0'), lenbyte);
            tagElement = root.Select(header);
            if(!tagElement.IsEmpty())
            {
                tagElement.GetValue(tmp);
                cvtLen = tlvConverter.Parse(header, tmp, dst);
                
                tmp.Clear();
                tmp.Format(sLenFormat.GetString(), cvtLen);
                memcpy(const_cast<byte*>(dst.GetBuffer()) + offset, tmp.GetBuffer(), lenbyte);

                ++transCount;
            }
            else
            {
                // 不填充的话截掉标签位和长度位 
                if(!fillEmpty)
                {
                    dst.RemoveTail(lenbyte + 1);
                }
            }
        }
        
        return transCount;
    }
    //----------------------------------------------------- 
    /**
     * @brief 将外部的数据转换为IC卡中的数据格式 
     * @param [in] src 源数据 
     * @param [in] lenbyte 长度字节的长度 
     * @param [out] dst 转换后的数据 
     * @param [in] table 转换的标签对照表 
     */ 
    static bool transToTLV(const ByteArray& src, size_t lenbyte, 
        ByteBuilder& dst, const ushort table[])
    {
        ushort flag = TABLE_EOF;
        ushort tag = TABLE_EOF;
        size_t len = 0;
        size_t srclen = src.GetLength();
        const char* ptr = src.GetString();
        size_t tablesize = getTableSize(table);
        ByteBuilder tmpBuffer(16);
        ByteBuilder transBuffer(16);

        for(size_t i = 0;i < srclen;)
        {
            // 标识位
            flag = static_cast<ushort>(src[i++]);
            tag = findInTable(flag, table, tablesize);

            // 长度存在 
            ASSERT_Func(srclen > i + lenbyte);
            len = getLen(ptr + i, lenbyte);
            i += lenbyte;
            ASSERT_Func(srclen >= i + len);
            i += len;
            if(TABLE_EOF == tag)
                continue;
            tmpBuffer.Clear();
            // 先将数据取出来
            if(len % 2 != 0)
                tmpBuffer += "0";
            tmpBuffer.Append(src.SubArray(i - len, len));

            // 组包数据
            DOLDataType type = PBOC_v2_0_TagMap::GetType(tag);
            size_t tmpLength = PBOC_v2_0_TagMap::GetLength(tag);
            // 计算填充的长度
            if(tmpLength == 0)
            {
                // tmpBuffer.GetLength() 一定为偶数
                tmpLength = (type == DOL_ans ? len : (tmpBuffer.GetLength() / 2));
            }

            // 需要将ASCII码转换为BCD码
            if(type != DOL_ans)
            {
                transBuffer = tmpBuffer;
                tmpBuffer.Clear();
                ByteConvert::FromAscii(transBuffer, tmpBuffer);
            }
            TlvConvert::ToHeaderBytes(tag, dst);
            TlvConvert::ToLengthBytes(tmpLength, dst);
            PBOC_Library::PackDolData(dst, tmpBuffer, tmpLength, type);
        }
        return true;
    }
    //----------------------------------------------------- 
    /**
     * @brief 合并格式 
     * @param [in] format 标签的顺序和长度 
     * @param [in] src 对应格式的数据 
     * @param [in] dst 转换后的数据 
     * @return 解析的标签数目 
     */ 
    static size_t packFormatData(const ByteArray& format, const ByteArray& src, ByteBuilder& dst)
    {
        size_t transCount = 0;
        TlvElement root = TlvElement::Parse(format, TlvElement::NonValue);
        if(root.IsEmpty())
            return transCount;

        size_t formatlen = 0;
        size_t currentlen = 0;
        size_t lastlen = dst.GetLength();
        ushort header = TlvHeader::ERROR_TAG_HEADER;

        ByteArray tmp;
        TlvElement tagElement = root.MoveNext();
        while(!tagElement.IsEmpty())
        {
            currentlen = tagElement.GetLength();
            header = tagElement.GetHeader();
            tmp = src.SubArray(formatlen, currentlen);

            formatlen += currentlen;
            if(formatlen > src.GetLength())
            {
                dst.RemoveTail(dst.GetLength() - lastlen);
                return 0;
            }

            TlvConvert::MakeTLV(header, tmp, dst);

            ++transCount;

            tagElement = root.MoveNext();
        }
        
        return transCount;
    }
    //----------------------------------------------------- 
    // 打印N个字符 
    static void writeChar(const char* pStr, size_t n, LoggerAdapter& logAdapter)
    {
        for(size_t i = 0;i < n; ++i)
        {
            logAdapter<<pStr[i];
        }
    }
    //----------------------------------------------------- 
    /**
     * @brief 按照默认的信息标签表输出客户信息 
     */ 
    static void PrintInformation(const ByteArray& info, LoggerAdapter& logAdapter)
    {
        const char* ptr = info.GetString();
        size_t len = 0;
        bool isMoney = false;
        const char* infoEOF = ptr + info.GetLength() - 1;
    
        while(ptr <= infoEOF)
        {
            switch(*ptr)
            {
            case 'A':
                logAdapter<<"卡号: <";
                break;
            case 'B':
                logAdapter<<"姓名: <";
                break;
            case 'C':
                logAdapter<<"证件类型: <";
                break;
            case 'D':
                logAdapter<<"证件号: <";
                break;
            case 'E':
                logAdapter<<"二磁道数据: <";
                break;
            case 'F':
                logAdapter<<"一磁道数据: <";
                break;
            case 'G':
                logAdapter<<"电子现金余额: <";
                isMoney = true;
                break;
            case 'H':
                logAdapter<<"余额上限: <";
                isMoney = true;
                break;
            case 'I':
                logAdapter<<"失效日期: <";
                break;
            case 'J':
                logAdapter<<"PAN 序列号: <";
                break;
            case 'K':
                logAdapter<<"单笔交易限额: <";
                isMoney = true;
                break;
            case 'L':
                logAdapter<<"电子现金重置阈值: <";
                isMoney = true;
                break;
            case 'M':
                logAdapter<<"连续脱机交易限制数(国际-国家): <";
                break;
            case 'N':
                logAdapter<<"连续脱机交易限制数(国际-货币): <";
                break;
            case 'O':
                logAdapter<<"累计脱机交易金额限制数: <";
                isMoney = true;
                break;
            case 'P':
                logAdapter<<"累计脱机交易金额限制数(双货币): <";
                isMoney = true;
                break;
            case 'Q':
                logAdapter<<"累计脱机交易金额上限: <";
                isMoney = true;
                break;
            case 'R':
                logAdapter<<"货币转换因子: <";
                break;
            case 'S':
                logAdapter<<"连续脱机交易下限: <";
                isMoney = true;
                break;
            case 'T':
                logAdapter<<"连续脱机交易上限: <";
                isMoney = true;
                break;
            case 'U':
                logAdapter<<"卡片当前应用状态码: <";
                break;
            }
            ptr += 1;
            len = getLen(ptr, 3);
            ptr += 3;
        
            if(isMoney)
            {
                size_t money = getLen(ptr, len);
                ByteBuilder tmpBuff(8);
                tmpBuff.Format("%.2f元", money/100.0);
            
                logAdapter<<tmpBuff.GetString();
            
                isMoney = false;
            }
            else
            {
                writeChar(ptr, len, logAdapter);
            }
            ptr += len;
        
            logAdapter.WriteLine(">");
        }
    }
    //----------------------------------------------------- 
    /**
     * @brief 按照默认的日志标签输出交易明细 
     */ 
    static void PrintDealDetail(const ByteArray& detail, LoggerAdapter& logAdapter)
    {
        const char* ptr = detail.GetString();
        size_t len = 0;
        bool isMoney = false;
        int count = 0;
        const char* detailEOF = ptr + detail.GetLength() - 1;

        size_t money = 0;
        ByteBuilder tmpBuff(8);

        while(ptr <= detailEOF)
        {
            switch(*ptr)
            {
            case 'P':
                logAdapter<<"\n记录:<"<<++count<<">"<<endl;
                logAdapter<<"授权金额: <";
                isMoney = true;
                break;
            case 'Q':
                logAdapter<<"其他金额: <";
                isMoney = true;
                break;
            case 'R':
                logAdapter<<"交易货币代码: <";
                break;
            case 'S':
                logAdapter<<"交易日期: <";
                break;
            case 'T':
                logAdapter<<"交易类型: <";
                break;
            case 'U':
                logAdapter<<"交易时间: <";
                break;
            case 'V':
                logAdapter<<"终端国家代码: <";
                break;
            case 'W':
                logAdapter<<"商户名称: <";
                break;
            case 'X':
                logAdapter<<"交易计数器: <";
                break;
            }
            ptr += 1;
            len = getLen(ptr, 3);
            ptr += 3;
        
            if(isMoney)
            {
                money = getLen(ptr, len);

                tmpBuff.Clear();
                tmpBuff.Format("%.2f元", money/100.0);
            
                logAdapter<<tmpBuff.GetString();
            
                isMoney = false;
            }
            else
            {
                writeChar(ptr, len, logAdapter);
            }
            ptr += len;
        
            logAdapter.WriteLine(">");
        }
        logAdapter<<"记录条数:<"<<count<<">\n";
    }
    //----------------------------------------------------- 
    //@}
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 默认的转换 
class PbocTlvConverter : public ITlvConverter
{
public:
    /// 对于ans格式标签数据的字符集转换函数 
    typedef size_t(*fpTlvAnsConvert)(const char* gbk, size_t gbkLen, ByteBuilder& dst);
protected:
    fpTlvAnsConvert _tlvAnsConverter;
public:
    PbocTlvConverter(fpTlvAnsConvert tlvAnsConvert = NULL)
    {
        _tlvAnsConverter = tlvAnsConvert;
    }
    /// 转换数据 
    virtual size_t Parse(ushort tag, const ByteArray& data, ByteBuilder& dst)
    {
        size_t len = 0;
        size_t cvtLen = 0;
        ByteBuilder tmp(32);
        switch(tag)
        {
            // 卡号 
        case 0x5A:
            // 二磁道数据 
        case 0x57:
            ByteConvert::ToAscii(data, tmp);
            /* 剔除末尾补位的F */
            if(tmp.GetLength() > 0)
            {
                while(_get_upper(tmp[tmp.GetLength() - 1]) == 'F')
                    tmp.RemoveTail();
            }
            /* 替换二磁道数据中的D为=号 */
            if(0x57 == tag)
            {
                for(size_t i = 0;i < tmp.GetLength(); ++i)
                {
                    if(_get_upper(tmp[i]) == 'D')
                    {
                        tmp[i] = '=';
                        break;
                    }
                }
            }
            dst += tmp;
            cvtLen = tmp.GetLength();
            break;
            // 货币代码 
        case 0x9F1A:
        case 0x5F2A:
            ByteConvert::ToAscii(data, tmp);
            tmp.RemoveFront();
            dst += tmp;
            cvtLen = tmp.GetLength();
            break;
        default:
            if(PBOC_v2_0_TagMap::GetType(tag) == DOL_ans)
            {
                len = strlen(data.GetString());
                // GBK编码的字符串 
                if(_tlvAnsConverter != NULL)
                {
                    cvtLen = _tlvAnsConverter(data.GetString(), len, dst);
                }
                else
                {
                    dst.Append(data.SubArray(0, len));
                    cvtLen = len;
                }
            }
            else
            {
                cvtLen = ByteConvert::ToAscii(data, dst);
            }
            break;
        }

        return cvtLen;
    }
};
//--------------------------------------------------------- 
} // namespace pboc 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_APP_H_
//========================================================= 
