//========================================================= 
/**@file PBOC_v2_0_TagMap.h 
 * @brief IC卡应用数据表
 * 
 * @date 2014-10-17   17:41:00 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../../include/Base.h"

#include "../base/TlvElement.h"
using zhou_yb::device::iccard::pboc::base::TlvConvert;
using zhou_yb::device::iccard::pboc::base::TlvHeader;
using zhou_yb::device::iccard::pboc::base::TlvElement;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace v2_0 {
//--------------------------------------------------------- 
#ifdef _MSC_VER
#   define _GET_HEADER(index) (_pboc_v2_0_tag_table[PBOC_v2_0_TagMap::##index].header)
#else
#   define _GET_HEADER(index) (_pboc_v2_0_tag_table[PBOC_v2_0_TagMap::index].header)
#endif
//--------------------------------------------------------- 
/// 标签数据类型
enum DOLDataType
{
    /// 字母
    DOL_a = 0,
    /// 字母和数字
    DOL_an,
    /// 字母,数字,特殊符号
    DOL_ans,
    /// 二进制
    DOL_b,
    /// 压缩数字
    DOL_cn,
    /// 数字
    DOL_n,
    /// 可变长数据
    DOL_var,
    /// 结构标签
    DOL_struct,
    /// 未知类型 
    DOL_Unknown
};
//---------------------------------------------------------
/// 标签头信息
struct HeaderType
{
    ushort header;
    ushort len;
    DOLDataType type;

    HeaderType(ushort _header = 0, 
        ushort _len = 0, 
        DOLDataType _type = DOL_var)
        : header(_header), len(_len), type(_type) {}
};
//---------------------------------------------------------
/// 货币代码结构
struct CurrencyCodeNode
{
    /// 代码
    ushort CurrencyCode;
    /// 字符串描述
    char   CurrencyName[4];
};
//---------------------------------------------------------
/// 标签值、长度、数据类型表
extern const HeaderType _pboc_v2_0_tag_table[];
/// _pboc_v2_0_tag_table 数组大小 
extern const size_t _pboc_v2_0_tag_table_len;
//---------------------------------------------------------
/// 货币代码转换表
extern const CurrencyCodeNode _pboc_v2_0_currencyCodeTable[];
/// _pboc_v2_0_currencyCodeTable 数组大小 
extern const size_t _pboc_v2_0_currencyCodeTable_len;
//--------------------------------------------------------- 
/// 可以通过GET DATA命令获取的标签 
extern const ushort _pboc_v2_0_get_data_table[];
/// _pboc_v2_0_get_data_table 数组大小 
extern const size_t _pboc_v2_0_get_data_table_len;
//--------------------------------------------------------- 
/// PBOC2.0标签表
class PBOC_v2_0_TagMap
{
protected:
    PBOC_v2_0_TagMap() {}
public:
    //-----------------------------------------------------
    /// 标签索引枚举
    enum HeaderIndex 
    {
        /* 第六部分36个数据标签 P72 */
        /// 收单行标识 0x9F01
        AcquiringBank = 0,
        /// 附加终端性能 0x9F40
        AdditionalTerminalPerformance,
        /// 授权金额(二进制) 0x81
        AuthorizedAmount_B,
        /// 授权金额(数值型) 0x9F02
        AuthorizedAmount_N,
        /// 其它金额(二进制) 0x9F04
        OtherAmount_B,
        /// 其他金额(数值形) 0x9F03
        OtherAmount_N,
        /// 参考货币金额 0x9F3A
        ReferenceCurrency,
        /// 应用标识（AID） 0x9F06
        //ApplicationIdentifier,
        AID,
        /// 应用版本号 0x9F09
        AID_Version,
        /// 授权响应代码 0x8A
        AuthorizationResponseCode,
        /// 持卡人验证方法（CVM）结果 0x9F34
        CVM_Result,
        /// 认证中心公钥索引 0x9F22
        PublicKeyIndex,
        /// 命令模版 0x83
        CommandTemplate,
        /// 接口设备（IFD）序列号 0x9F1E
        IFD_Number,
        /// 商户分类码 0x9F15
        MerchantCategoryCode,
        /// 商户标识 0x9F16
        MerchantIdentification,
        /// 销售点（POS）输入方式 0x9F39
        PosInputMode,
        /// 终端性能 0x9F33
        TerminalPerformance,
        /// 终端国家代码 0x9F1A
        TerminalCountryCode,
        /// 终端最低限额 0x9F1B
        TerminalLimit,
        /// 终端标识 0x9F1C
        TerminalIdentification,
        /// 终端类型 0x9F35
        TerminalType,
        /// 终端验证结果 0x95
        TerminalVerificationResults,
        /// 交易证书（TC）哈希值 0x98
        //TransactionCertificateHash,
        TC_Hash,
        /// 交易货币代码 0x5F2A
        TransactionCurrencyCode,
        /// 交易货币指数 0x5F36
        TransactionCurrencyIndex,
        /// 交易日期 0x9A
        TransactionDate,
        /// 交易PIN 数据 0x99
        TransactionPIN_Data,
        /// 交易参考货币代码 0x9F3C
        TransactionRefCurrencyCenerationCode,
        /// 交易参考货币指数 0x9F3D
        TransactionRefCurrencyNum,
        /// 交易序列计数器 0x9F41
        TransactionSequenceCounter,
        /// 交易状态信息 0x9B
        TransactionStatusInfo,
        /// 交易时间 0x9F21
        TransactionTime,
        /// 交易类型 0x9C
        TransactionType,
        /// 随机数 0x9F37
        RandomNumber,
        /// 账户类型 0x5F57
        AccountType,

        /* 第五部分数据需求标签 P96 */
        /// 应用密文（AC）0x9F26
        ///ApplicationCiphertext,
        AC,
        /// 应用货币代码 0x9F42
        ApplicationCurrencyCode_1,
        /// 应用货币代码 0x9F51
        ApplicationCurrencyCode_2,
        /// 应用货币指数 0x9F44
        ApplicationMonetaryIndex,
        /// 应用缺省行为（ADA）0x9F52
        //ApplicationDefaultAction,
        ADA,
        /// 应用自定义数据 0x9F05
        ApplicationCustomData,
        /// 应用生效日期 0x5F25
        ApplicationEffectiveDate,
        // 应用失效日期 0x5F24
        ApplicationFailureDate,
        /// 应用文件定位器（AFL） 0x94
        //ApplicationFileLocator,
        AFL,
        /// 应用标识符（AID）0x4F
        //ApplicationIdentification,
        AID_Symbol,
        /// 应用交互特征（AIP）0x82
        ApplicationInteractiveFeatures,
        /// 应用标签 0x50
        ApplicationLabels,
        /// 应用首选名称 0x9F12
        ApplicationPreferredName,
        /// 应用PAN 0x5A
        ApplicationPAN,
        /// 应用PAN 序列号 0x5F34
        ApplicationPANSequenceNumber,
        /// 应用优先指示器 0x87
        //ApplicationPriorityIndicator,
        AID_Priority,
        /// 应用交易计数器（ATC） 0x9F36
        ApplicationTransactionCounter,
        /// 应用用途控制（AUC）0x9F07
        ApplicationUsageControl,
        /// 应用版本号 0x9F08
        ApplicationVersionNumber,
        /// 卡片风险管理数据对象列表1（CDOL1）0x8C
        CDOL1,
        /// 卡片风险管理数据对象列表2（CDOL2）0x8D
        CDOL2,
        /// 持卡人姓名 0x5F20
        CardholderName,
        /// 持卡人姓名扩展 0x9F0B
        CardholderExtendName,
        /// 持卡人证件号 0x9F61
        CardholderCertificateNumber,
        /// 持卡人证件类型 0x9F62
        CardholderCertificateType,
        /// 持卡人验证方法（CVM）列表 0x8E
        CVMList,
        /// CA 公钥索引 0x8F
        CAPublicKeyIndex,
        /// 连续脱机交易限制数（国际-货币）0x9F53
        ContinuousOffLineTransactionLimitCurrency,
        /// 连续脱机交易限制数（国际-国家) 0x9F72
        ContinuousOffLineTransactionLimitCountry,
        /// 密文信息数据（CID）0x9F27
        CID,
        /// 累计脱机交易金额限制数 0x9F54
        CumulativeOffLineTransactionsGoldAmountLimit,
        /// 累计脱机交易金额上限 0x9F5C
        CumulativeOffLineTransactionsGoldAmountMaximum,
        /// 累计脱机交易金额限制数（双货币）0x9F75
        CumulativeOffLineTransactionsGoldAmountLimitDoubleCurrency,
        /// 货币转换因子 0x9F73
        CurrencyConversionFactor,
        /// 数据认证码 0x9F45
        DataAuthenticationCode,
        /// 专用（DF）文件名称 0x84
        DF_Name,
        /// 目录定义文件名称 0x5D
        // D:Define
        DDF_Name,
        /// 目录自定义模板 0x73
        CatalogCustomTemplates,
        /// 动态数据认证数据对象列表（DDOL）0x9F49
        //DynamicDataAuthenticationDataObjectList,
        DDOL,
        /// 文件控制信息（FCI）发卡行自定义数据 0xBF0C
        FCI_DefineData,
        /// FCI 专有模板 0xA5
        FCI_ExclusiveTemplate,
        /// FCI 模板 0x6F
        FCI_Template,
        /// ICC 动态数 0x9F4C
        ICC_DynamicNumber,
        /// 公钥证书 0x9F47
        PublicKeyCertificate,
        /// 公钥模数 0x9F46
        PublicKeyModulus,
        // 公钥余数 0x9F48
        PublicKeyRemainder,
        /// 发卡行行为代码-缺省 0x9F0D
        IssuerActionCodeDefault,
        /// 发卡行行为代码-拒绝 0x9F0E
        IssuerActionCodeRefuse,
        /// 发卡行行为代码-联机 0x9F0F
        IssuerActionCodeOnline,
        /// 发卡行应用数据 0x9F10
        IssuerApplicationData,
        /// 发卡行认证数据 0x91
        IssuerAuthenticationData,
        /// 发卡行认证指示位 0x9F56
        IssuerAuthenticationIndicatorBit,
        /// 发卡行代码表索引 0x9F11
        IssuerCodeTableIndex,
        /// 发卡行国家代码 0x5F28
        IssuerCountryCode_1,
        /// 发卡行国家代码 0x9F57
        IssuerCountryCode_2,
        // 发卡行公钥证书 0x90
        IssuerPublicKeyCertificate,
        /// 发卡行公钥模数 0x9F32
        IssuerPublicKeyModulus,
        /// 发卡行公钥余数 0x92
        IssuerPublicKeyRemainder,
        /// 发卡行脚本模板2 0x72
        IssuerScriptTemplate2,
        /// 发卡行URL 0x9F50
        IssuerURL,
        /// 发卡行URL2 0x9F5A
        IssuerURL2,
        /// 首选语言 0x5F2D
        PreferredLanguage,
        /// 上次联机ATC 寄存器 0x9F13
        ATCMailOnlineLatch,
        /// 日志入口 0x9F4D
        LogEntrance,
        /// 日志格式 0x9F4F
        LogFormat,
        /// 连续脱机交易下限 0x9F14
        ContinuousOfflineTransactionDownLimit_1,
        /// 连续脱机交易下限 0x9F58
        ContinuousOfflineTransactionDownLimit_2,
        /// PIN 尝试计数器 0x9F17
        PIN_AttemptCounter,
        /// 处理选项数对 0x9F38
        //ProcessingOptionsDataOn,
        PDOL,
        /// 响应报文模板格式1 0x80
        ResponseMsgTemplateLatticeType_1,
        /// 第2应用货币代码 0x9F76
        SecondGenerationApplicationCurrencyCode,
        /// 服务码 0x5F30
        ServiceCode,
        /// 短文件标识符（SFI）0x88
        //ShortFileIdentifier,
        SFI,
        /// 签名的动态应用数据 0x9F4B
        SignatureDynamicApplicationData,
        /// 签名的静态应用数据 0x93
        SignatureStaticApplicationData,
        /// 静态数据认证标签列表 9F4A
        StaticDataAuthenticationMarkCheckList,
        /// 磁条1 自定义数据 0x9F1F
        Magnetic1CustomData,
        /// 磁条2 等效数据 0x57
        Magnetic2EquivalentData,
        /// 交易证书数据对象列表（TDOL）0x97
        //TransactionCertificateDataObjectList,
        TDOL,
        /// 连续脱机交易上限 0x9F23
        ContinuousOfflineTransactionUpLimit_1,
        /// 连续脱机交易上限 0x9F59
        ContinuousOfflineTransactionUpLimit_2,
        /// 卡产品标识信息 0x9F63
        CardProductIdentificationInformation,

        /* 其他接口之外的标签 */
        /// 应用模板 0x61
        ApplicationTemplate,
        /// 目录数据名称 0x9D
        DDF,
        /// 发卡行脚本命令 0x86
        IssuerScriptCommand,
        /// 电子现金余额 0x9F79
        EC_Balance,
        /// 电子现金余额上限 0x9F77
        EC_BalanceLimit,
        /// 电子现金发卡行授权码 0x9F74
        EC_IssuerAuthorizationCode,
        /// 电子现金单笔交易限额 0x9F78
        EC_SingleTransactionLimit,
        /// 电子现金重置阈值 0x9F6D
        EC_ResetThreshold,
        /// 电子现金终端支持指示器 0x9F7A
        EC_TerminalSupportIndicator,
        /// 电子现金终端交易限额 0x9F7B
        EC_TerminalTransactionLimit,
        /// 发卡行脚本 0x71
        IssuerScript1,
        /// 发卡行脚本 0x72
        IssuerScript2,
        /// 响应报文模板格式2 0x77
        ResponseMsgTemplate2,
        /// 0x51
        FID,
        /// C1_MONETA 0xC1
        C1_Moneta,
        /// 0xC2
        C2_Moneta,
        /// 交易货币指数 0x5F36
        TrmTrxnCurrencyExp,
        /// 发卡行脚本标识 0x9F18
        IssuerScriptID,
        /// 终端风险管理数据 0x9F1D
        TrmRiskManagentData,
        /// 磁条2自定义数据 0x9F20
        Magnetic2CustomData,
        /// IC卡PIN加密公钥证书 0x9F2D
        ICCardPIN_EPKC,
        /// IC卡PIN加密公钥指数 0x9F2E
        ICCardPIN_EPKE,
        /// IC卡PIN加密公钥余项 0x9F2F
        ICCardPIN_EPKR,
        /// 应用参考货币 0x9F3B
        AppRefCurrency,
        /// 应用参考货币指数 0x9F43
        AppRefCurrencyExp,
        /// 0x9F55
        GeoGraphicIndicatorVisa,
        /// TRM_ISR 0x9F5B
        TrmIsr,
        /// CPLC_HISTORY_FID_VISA 0x9F7F
        CplcHistoryFidVisa,
        /// TRM_SCRIPT 0x17
        TrmScript,
        /// TRM_CAPK 0x18
        TrmCapk,
        /// TRM_PDOL_DATA 0x19
        TrmPDOL_DATA,
        /// TRM_CDOL1DATA 0x1A
        TrmCDOL1_DATA,
        /// TRM_CDOL2DATA 0x1B
        TrmCDOL2_DATA,
        /// 0x04
        EOT4,
        /// 记录标签头 0x70
        Recode,
        /// 授权码 0x89
        AuxnCode,
        /// 商户名称 0x9F4E
        MerchantName,
        /// 可用脱机消费金额 0x9F5D
        AvailableOfflineConsumptionAmount,
        /// 终端交易属性 0x9F66
        EC_TerminalTransactionAttribute,
        /// 卡片附加处理 0x9F68
        CardAdditionalProcessing,
        /// 卡片持卡人验证方法限制 0x9F6B
        CardCardholderVerificationMethodLimit,
        /// 卡片交易属性 0x9F6C
        CardTransactionAttributes,
        /// 索引的最大项
        MaxHeaderIndex
    };
    //-----------------------------------------------------
    /// 获取指定索引处的标签头
    static TlvHeader GetHeader(HeaderIndex index)
    {
        if(index >= MaxHeaderIndex)
            return TlvHeader::ERROR_TAG_HEADER;
        return TlvHeader(_pboc_v2_0_tag_table[index].header);
    }
    /// 获取指定索引处的标签所占长度
    static size_t GetLength(const TlvHeader& header)
    {
        if(header == TlvHeader::ERROR_TAG_HEADER)
            return static_cast<size_t>(TlvHeader::ERROR_TAG_HEADER);

        size_t count = static_cast<size_t>(MaxHeaderIndex);
        for(size_t i = 0;i < count; ++i)
        {
            if(header == _pboc_v2_0_tag_table[i].header)
                return static_cast<size_t>(_pboc_v2_0_tag_table[i].len);
        }

        return TlvHeader::ERROR_TAG_HEADER;
    }
    /// 获取指定索引处的标签数据格式
    static DOLDataType GetType(const TlvHeader& header)
    {
        if(header == TlvHeader::ERROR_TAG_HEADER)
            return DOL_Unknown;

        size_t count = static_cast<size_t>(MaxHeaderIndex);
        for(size_t i = 0;i < count; ++i)
        {
            if(header == _pboc_v2_0_tag_table[i].header)
                return _pboc_v2_0_tag_table[i].type;
        }

        return DOL_Unknown;
    }
    //-----------------------------------------------------
};
//---------------------------------------------------------
} // namespace v2_0 
} // namespace pboc
} // namesapce iccard 
} // namesapce device 
} // namespace zhou_yb
//=========================================================