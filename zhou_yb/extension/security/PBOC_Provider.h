//========================================================= 
/**@file PBOC_Provider.h 
 * @brief PBOC IC卡相关密钥计算方法 
 * 
 * @date 2014-03-16   14:54:11 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_PROVIDER_H_
#define _LIBZHOUYB_PBOC_PROVIDER_H_
//--------------------------------------------------------- 
#include "DES_Provider.h"

#include "../../device/iccard/pboc/base/TlvElement.h"
using zhou_yb::device::iccard::pboc::base::TlvHeader;
using zhou_yb::device::iccard::pboc::base::TlvConvert;
using zhou_yb::device::iccard::pboc::base::TlvElement;
using namespace zhou_yb::device::iccard::pboc::v2_0;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// PBOC相关密钥分散,MAC计算等密钥相关功能函数 
class PBOC_Provider
{
private:
    //----------------------------------------------------- 
    PBOC_Provider() {}
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief Encrypt DES密钥相关数据加密
     * 
     * @param [in] key_8_16_24 8/16/24字节长度的密钥 
     * @param [in] keyLen 密钥长度 
     * @param [in] data_8X 待加密的数据 
     * @param [in] data_8X_Len 待加密的数据长度 
     * @param [in] dst_8X 加密后的数据 
     * 
     * @return size_t 加密的数据长度  
     */
    static size_t Encrypt(const ByteArray& key_8_16_24, const ByteArray& data_8X, ByteBuilder& dst_8X)
    {
        size_t count = 0;
        ByteBuilder tmp1(DES_BLOCK_SIZE);
        ByteBuilder tmp2(DES_BLOCK_SIZE);
        switch(key_8_16_24.GetLength())
        {
        case DES_BLOCK_SIZE:
            count = DES_Provider::ECB_Encrypt(key_8_16_24, data_8X, dst_8X);
            break;
        case 2*DES_BLOCK_SIZE:
            count = DES_Provider::ECB_Encrypt(key_8_16_24, data_8X, tmp1);
            DES_Provider::ECB_Decrypt(key_8_16_24.SubArray(DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmp1, tmp2);
            DES_Provider::ECB_Encrypt(key_8_16_24, tmp2, dst_8X);
            break;
        case 3*DES_BLOCK_SIZE:
            count = DES_Provider::ECB_Encrypt(key_8_16_24, data_8X, tmp1);
            DES_Provider::ECB_Decrypt(key_8_16_24.SubArray(DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmp1, tmp2);
            DES_Provider::ECB_Encrypt(key_8_16_24.SubArray(2 * DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmp2, dst_8X);
            break;
        default:
            return count;
        }
        return count;
    }
    /**
     * @brief Decrypt DES密钥相关数据解密
     *
     * @param [in] key_8_16_24 8/16/24字节长度的密钥
     * @param [in] keyLen 密钥长度
     * @param [in] data_8X 待解密的数据
     * @param [in] data8X_Len 待解密的数据长度
     * @param [in] dst_8X 解密后的数据
     *
     * @return size_t 解密的数据长度
     */
    static size_t Decrypt(const ByteArray& key_8_16_24, const ByteArray& data_8X, ByteBuilder& dst_8X)
    {
        size_t count = 0;
        ByteBuilder tmp1(DES_BLOCK_SIZE);
        ByteBuilder tmp2(DES_BLOCK_SIZE);
        switch(key_8_16_24.GetLength())
        {
        case DES_BLOCK_SIZE:
            count = DES_Provider::ECB_Decrypt(key_8_16_24, data_8X, dst_8X);
            break;
        case 2 * DES_BLOCK_SIZE:
            count = DES_Provider::ECB_Decrypt(key_8_16_24, data_8X, tmp1);
            DES_Provider::ECB_Encrypt(key_8_16_24.SubArray(DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmp1, tmp2);
            DES_Provider::ECB_Decrypt(key_8_16_24, tmp2, dst_8X);
            break;
        case 3 * DES_BLOCK_SIZE:
            count = DES_Provider::ECB_Decrypt(key_8_16_24, data_8X, tmp1);
            DES_Provider::ECB_Encrypt(key_8_16_24.SubArray(DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmp1, tmp2);
            DES_Provider::ECB_Decrypt(key_8_16_24.SubArray(2 * DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmp2, dst_8X);
            break;
        default:
            return count;
        }
        return count;
    }
    /**
     * @brief 获取卡片分散因子 
     * @param [in] tag_5A_5F34 包含5A,5F34标签的数据源,没有5F34则以00替代  
     * @param [out] factor_8 获取到的8字节分散因子 
     * @return 返回是否包含5A,5F34标签 
     */ 
    static bool DispersionFactor(TlvElement& tag_5A_5F34, ByteBuilder& factor_8)
    {
        ByteBuilder tmp(32);
        ByteBuilder tagAscii(64);

        TlvElement subElement = tag_5A_5F34.Select(0x5A);
        if(subElement.IsEmpty())
            return false;
        
        subElement.GetValue(tmp);
        ByteConvert::ToAscii(tmp, tagAscii);
        if(StringConvert::LastCharAt(tagAscii, 1) == static_cast<byte>('F'))
            tagAscii.RemoveTail();

        subElement = tag_5A_5F34.Select(0x5F34);
        if(!subElement.IsEmpty())
        {
            tmp.Clear();
            tag_5A_5F34.GetValue(tmp);
            ByteConvert::ToAscii(tmp, tagAscii);
        }
        else
        {
            tagAscii += "00";
        }
        // 卡号+序列号 不足16字符前补'0',超过取后16个数字 
        ByteConvert::Fill(tagAscii, 16, false, static_cast<byte>('0'));
        ByteConvert::FromAscii(tagAscii, factor_8);

        return true;
    }
    /**
     * @brief 密钥分散获取过程密钥(如果需要的是单倍长度的密钥,直接使用subKey_16的前8字节即可) 
     * @param [in] key_16 待分散的16字节主密钥 
     * @param [in] factor_8 8字节分散因子 
     * @param [out] subKey_16 16字节分散好的子密钥 
     */ 
    static bool SessionKey(const ByteArray& key_16, const ByteArray& factor_8, ByteBuilder& subKey_16)
    {
        if(key_16.GetLength() < 2 * DES_BLOCK_SIZE || factor_8.GetLength() < DES_BLOCK_SIZE)
            return false;

        ByteBuilder subKey(DES_BLOCK_SIZE);
        ByteBuilder tmp(DES_BLOCK_SIZE);
        // 左半部分 
        ByteArray subfactor = factor_8.SubArray(0, DES_BLOCK_SIZE);
        Encrypt(key_16, subfactor, subKey);
        subKey_16 += subKey;

        // 右半部分(取反) 
        subKey.Clear();
        tmp.Append(subfactor);
        ByteConvert::Not(tmp);
        Encrypt(key_16, tmp, subKey);
        subKey_16 += subKey;

        return true;
    }
    /**
     * @brief ATC生成过程密钥 
     * @param [in] key_16 16字节待分散的MAC_UDK 
     * @param [in] tag_9F36 带9F36标签的数据源  
     * @param [out] sessionKey_16 生成的过程密钥 
     */ 
    static bool ATC_SessionKey(const ByteArray& key_16, TlvElement& tag_9F36, ByteBuilder& sessionKey_16)
    {
        TlvElement subElement = tag_9F36.Select(0x9F36);
        if(subElement.IsEmpty())
            return false;

        ByteBuilder atc(2);
        subElement.GetValue(atc);
        if(atc.GetLength() != 2)
            return false;

        return ATC_SessionKey(key_16, atc, sessionKey_16);
    }
    /**
     * @brief ATC生成过程密钥 
     * @param [in] key_16 16字节待分散的MAC_UDK 
     * @param [in] atc 2字节交易计数器ATC(9F36) 
     * @param [out] sessionKey_16 生成的过程密钥 
     */ 
    static bool ATC_SessionKey(const ByteArray& key_16, const ByteArray& atc, ByteBuilder& sessionKey_16)
    {
        if(key_16.GetLength() != 2*DES_BLOCK_SIZE)
            return false;

        ByteBuilder tag9F36(2);
        // 前补6个字节0到8字节 
        tag9F36.Append(static_cast<byte>(0x00), 6);
        // 带9F36的标签结构 
        if(atc.GetLength() > 2)
        {
            TlvElement tagElement = TlvElement::Parse(atc);
            if(tagElement.IsEmpty())
                return false;
            return ATC_SessionKey(key_16, tagElement, sessionKey_16);
        }
        else if(atc.GetLength() == 2)
        {
            tag9F36 += atc;
        }
        else 
        {
            return false;
        }
        // 左半部分 
        SessionKey(key_16, tag9F36, sessionKey_16);
        sessionKey_16.RemoveTail(DES_BLOCK_SIZE);
        // 右半部分 ATC取反,前补6个0x00
        tag9F36[6] = ~tag9F36[6];
        tag9F36[7] = ~tag9F36[7];
        SessionKey(key_16, tag9F36, sessionKey_16);
        sessionKey_16.RemoveTail(DES_BLOCK_SIZE);

        return true;
    }
    /**
     * @brief 计算ARQC的值 
     * @param [in] key_16 16字节密钥 
     * @param [in] tagAmtData 交易相关数据包含 "9F0206 9F0303 9F1A02 9505 5F2A02 9A03 9C01 9F3704 8202 9F3602 9F1013(CVR)"
     * @param [out] arqc 计算好的ARQC 
     * @param [out] pArqcData [default:NULL] 计算ARQC的数据 
     */ 
    static bool ARQC(const ByteArray& key_16, TlvElement& tagAmtData, ByteBuilder& arqc, ByteBuilder* pArqcData = NULL)
    {
        ByteBuilder pdol(16);
        ByteBuilder dol(32);

        DevCommand::FromAscii("9F0206 9F0306 9F1A02 9505 5F2A02 9A03 9C01 9F3704 8202 9F3602", pdol);
        PBOC_Library::PackPDOL(pdol, tagAmtData, dol);

        // 9F10 CVR单独处理
        TlvElement subElement = tagAmtData.Select(0x9F10);
        if(subElement.IsEmpty())
            return false;

        pdol.Clear();
        subElement.GetValue(pdol);
        if(pdol.GetLength() < 7)
            return false;
        // CVR为9F10第三到第7字节 
        dol += pdol.SubArray(3, 7);

        if(pArqcData != NULL)
        {
            pArqcData->Append(dol);
        }

        return PBOC_Provider::MAC(key_16, "", dol, arqc);
    }
    /**
     * @brief 计算ARPC的值  
     * @param [in] sessionKey_16 8字节过程密钥 
     * @param [in] arqc_8 8字节ARQC密文 
     * @param [in] authorizedCode_2 2字节授权响应码 
     * @param [out] arpc_8 计算出的8字节ARPC密文数据 
     */ 
    static bool ARPC(const ByteArray& sessionKey_16, const ByteArray& arqc_8, const ByteArray& authorizedCode_2, ByteBuilder& arpc_8)
    {
        if(sessionKey_16.GetLength() < 2*DES_BLOCK_SIZE || arqc_8.GetLength() < DES_BLOCK_SIZE || authorizedCode_2.GetLength() < 2)
            return false;

        ByteBuilder tmpInput(DES_BLOCK_SIZE);
        
        tmpInput = authorizedCode_2.SubArray(0, 2);
        ByteConvert::Fill(tmpInput, DES_BLOCK_SIZE, true, 0x00);
        ByteConvert::Xor(arqc_8, tmpInput);

        Encrypt(sessionKey_16.SubArray(0, 2 * DES_BLOCK_SIZE), tmpInput, arpc_8);

        return true;
    }
    /**
     * @brief 计算ARPC的值  
     * @param [in] sessionKey_16 8字节过程密钥 
     * @param [in] tagARQC 带9F26,8A数据的ARQC标签数据源  
     * @param [out] arpc_8 计算出的8字节ARPC密文数据 
     */ 
    static bool ARPC(const ByteArray& sessionKey_16, TlvElement& tagARQC, ByteBuilder& arpc_8)
    {
        ByteBuilder arqc(DES_BLOCK_SIZE);
        ByteBuilder authorizedCode(2);

        TlvElement subElement = tagARQC.Select(0x9F26);
        if(subElement.IsEmpty())
            return false;
        subElement.GetValue(arqc);

        if(arqc.GetLength() > DES_BLOCK_SIZE)
        {
            StringConvert::Right(arqc, 2, authorizedCode);
        }
        else
        {
            subElement = tagARQC.Select(0x8A);
            if(subElement.IsEmpty())
                return false;
            subElement.GetValue(authorizedCode);
        }

        ByteConvert::Fill(arqc, DES_BLOCK_SIZE, true);

        return ARPC(sessionKey_16, arqc, authorizedCode, arpc_8);
    }
    /**
     * @brief 计算MAC值(不需要补80,函数会自动补齐) 
     * @param [in] key_8_16 计算MAC的单倍或双倍长度密钥 
     * @param [in] iv 初始向量,长度不足8字节往后补0x00
     * @param [in] input_X 计算MAC的输入数据(不限制长度,计算时自动补80) 
     * @param [out] mac_8 计算出来的8字节MAC(实际使用时只需要前4字节)
     */ 
    static bool MAC(const ByteArray& key_8_16, const ByteArray& iv, ByteBuilder& input_X, ByteBuilder& mac_8)
    {
        if(key_8_16.GetLength() != DES_BLOCK_SIZE && key_8_16.GetLength() < 2*DES_BLOCK_SIZE)
            return false;

        size_t appendLen = DES_BLOCK_SIZE-(input_X.GetLength()%DES_BLOCK_SIZE);
        input_X += static_cast<byte>(0x80);
        input_X.Append(static_cast<byte>(0x00), appendLen - 1);

        // 初始化向量为8字节0x00
        ByteBuilder tmpInput(DES_BLOCK_SIZE);
        ByteBuilder tmpVector(DES_BLOCK_SIZE);

        tmpVector.Append(iv.SubArray(0, _min(iv.GetLength(), DES_BLOCK_SIZE)));
        ByteConvert::Fill(tmpVector, 8, true);
    
        for(size_t i = 0;i < input_X.GetLength()/DES_BLOCK_SIZE; ++i)
        {
            tmpInput.Clear();
            for(int j = 0;j < DES_BLOCK_SIZE; ++j)
                tmpInput += static_cast<byte>(input_X[DES_BLOCK_SIZE*i + j] ^ tmpVector[j]);
            tmpVector.Clear();
            DES_Provider::ECB_Encrypt(key_8_16, tmpInput, tmpVector);
        }
        input_X.RemoveTail(appendLen);
    
        if(key_8_16.GetLength() == 2*DES_BLOCK_SIZE)
        {
            tmpInput.Clear();
            DES_Provider::ECB_Decrypt(key_8_16.SubArray(DES_BLOCK_SIZE, DES_BLOCK_SIZE), tmpVector, tmpInput);
            tmpVector.Clear();
            DES_Provider::ECB_Encrypt(key_8_16, tmpInput, tmpVector);
        }
    
        mac_8 += tmpVector;
        return true;
    }
    /**
     * @brief 生成修改标签的脚本(完整的脚本,带MAC) 
     * @param [in] key_16 计算MAC的过程密钥 
     * @param [in] header 需要修改的标签 
     * @param [in] input_X 写卡的数据源(需要包含:9F26x8 9F36x2 header标识的数据) 
     * @param [in] script_X 组好的脚本指令 
     * @param [in] maclen [default:4] 需要的MAC长度字节,默认为4字节 
     */ 
    static bool Script04DA(const ByteArray& key_16, const TlvHeader& header, const ByteArray& input_X, ByteBuilder& script_X, size_t maclen = 4)
    {
        TlvElement root = TlvElement::Parse(input_X);
        if(root.IsEmpty())
            return false;

        TlvElement tagElement = root.Select(header);
        if(tagElement.IsEmpty())
            return false;
        // 计算MAC的输入数据 
        ByteBuilder macInput(16);
        DevCommand::FromAscii("04 DA", macInput);
        TlvConvert::ToHeaderBytes(header, macInput);
        
        byte len = 0;
        macInput += static_cast<byte>(len);
        
        tagElement = root.Select(0x9F36);
        if(tagElement.IsEmpty())
            return false;
        tagElement.GetValue(macInput);
        len += _itobyte(tagElement.GetLength());

        tagElement = root.Select(0x9F26);
        if(tagElement.IsEmpty())
            return false;
        tagElement.GetValue(macInput);
        len += _itobyte(tagElement.GetLength());

        tagElement = root.Select(header);
        if(tagElement.IsEmpty())
            return false;

        tagElement.GetValue(macInput);
        len += _itobyte(tagElement.GetLength());

        // 计算MAC 
        macInput[ICCardLibrary::LC_INDEX] = len;

        ByteBuilder mac(DES_BLOCK_SIZE);
        if(!MAC(key_16, "", macInput, mac))
            return false;
        // 所要求的MAC长度错误 
        if(!StringConvert::Left(mac, maclen))
            return false;

        DevCommand::FromAscii("04 DA", script_X);
        TlvConvert::ToHeaderBytes(header, script_X);
        tagElement = root.Select(header);
        tagElement.GetValue(script_X);

        script_X += mac;

        return true;
    }
    /**
     * @brief 生成修改记录的脚本 
     * @param [in] key_16 计算MAC的过程密钥 
     * @param [in] sfi 记录所在的SFI
     * @param [in] recodeNo 记录所在的记录号 
     * @param [in] recode 整条记录信息,还包括ARQC和ATC数据源 70+9F26+9F36  
     * @param [out] script_X 生成的脚本指令 
     * @param [in] maclen [default:4] 需要的MAC长度字节,默认为4字节 
     */ 
    static bool Script04DC(const ByteArray& key_16, byte sfi, byte recodeNo, const ByteArray& recode, ByteBuilder& script_X, size_t maclen = 4)
    {
        // 计算MAC的输入数据 
        ByteBuilder macInput(16);
        DevCommand::FromAscii("04 DC", macInput);

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace security
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_PROVIDER_H_
//========================================================= 