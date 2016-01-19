//========================================================= 
/**@file PBOC_v2_0_TagMap.cpp 
 * @brief PBOC2.0 IC卡数据标签表 
 * 
 * @date 2014-08-15   22:21:22 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../device/iccard/pboc/v2_0/PBOC_v2_0_TagMap.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace v2_0 {
//--------------------------------------------------------- 
/// 标签值、长度、数据类型表
const HeaderType _pboc_v2_0_tag_table[] = 
{
    /* 第六部分36个标签 P72 */
    // 收单行标识
    HeaderType( 0x9F01,  6, DOL_n ),
    // 附加终端性能
    HeaderType( 0x9F40,  5, DOL_b ),
    // 授权金额(二进制)
    HeaderType( 0x81,    4, DOL_b ),
    // 授权金额(数值型)
    HeaderType( 0x9F02,  6, DOL_n ),
    // 其它金额(二进制)
    HeaderType( 0x9F04,  4, DOL_b ),
    // 其他金额(数值形)
    HeaderType( 0x9F03,  6, DOL_n ),
    // 参考货币金额
    HeaderType( 0x9F3A,  4, DOL_b ),
    // 应用标识(AID)
    HeaderType( 0x9F06,  0, DOL_b ),
    // 应用版本号
    HeaderType( 0x9F09,  2, DOL_b ),
    // 授权响应代码
    HeaderType( 0x8A,    2, DOL_an),
    // 持卡人验证方法（CVM）结果
    HeaderType( 0x9F34,  3, DOL_b ),
    // 认证中心公钥索引
    HeaderType( 0x9F22,  1, DOL_b ),
    // 命令模版
    HeaderType( 0x83,    0, DOL_b ),
    // 接口设备（IFD）序列号
    HeaderType( 0x9F1E,  8, DOL_an),
    // 商户分类码
    HeaderType( 0x9F15,  2, DOL_n ),
    // 商户标识
    HeaderType( 0x9F16, 15, DOL_ans),
    // 销售点（POS）输入方式
    HeaderType( 0x9F39,  1, DOL_n ),
    // 终端性能
    HeaderType( 0x9F33,  3, DOL_b ),
    // 终端国家代码
    HeaderType( 0x9F1A,  2, DOL_n ),
    // 终端最低限额
    HeaderType( 0x9F1B,  4, DOL_b ),
    // 终端标识
    HeaderType( 0x9F1C,  8, DOL_an),
    // 终端类型
    HeaderType( 0x9F35,  1, DOL_n ),
    // 终端验证结果
    HeaderType( 0x95,    5, DOL_b ),
    // 交易证书（TC）哈希值
    HeaderType( 0x98,   20, DOL_b ),
    // 交易货币代码
    HeaderType( 0x5F2A,  2, DOL_n ),
    // 交易货币指数
    HeaderType( 0x5F36,  1, DOL_n ),
    // 交易日期
    HeaderType( 0x9A,    3, DOL_n ),
    // 交易PIN 数据
    HeaderType( 0x99,    0, DOL_var),
    // 交易参考货币代码
    HeaderType( 0x9F3C,  2, DOL_n ),
    // 交易参考货币指数
    HeaderType( 0x9F3D,  1, DOL_n ),
    // 交易序列计数器
    HeaderType( 0x9F41,  0, DOL_n ),
    // 交易状态信息
    HeaderType( 0x9B,    2, DOL_b ),
    // 交易时间
    HeaderType( 0x9F21,  3, DOL_n ),
    // 交易类型
    HeaderType( 0x9C,    1, DOL_n ),
    // 随机数
    HeaderType( 0x9F37,  4, DOL_b ),
    // 账户类型
    HeaderType( 0x5F57,  1, DOL_n ),

    /* 第五部分标签 P96 */
    // 应用密文（AC）
    HeaderType( 0x9F26 , 8, DOL_b ),
    // 应用货币代码
    HeaderType( 0x9F42 , 0, DOL_n ),
    // 应用货币代码
    HeaderType( 0x9F51 , 0, DOL_n ),
    // 应用货币指数
    HeaderType( 0x9F44 , 0, DOL_n ),
    // 应用缺省行为（ADA）
    HeaderType( 0x9F52 , 0, DOL_b ),
    // 应用自定义数据
    HeaderType( 0x9F05 , 0, DOL_b ),
    // 应用生效日期
    HeaderType( 0x5F25 , 0, DOL_n ),
    // 应用失效日期
    HeaderType( 0x5F24 , 0, DOL_n ),
    // 应用文件定位器（AFL）
    HeaderType( 0x94 , 0, DOL_b ),
    // 应用标识符（AID）
    HeaderType( 0x4F , 0, DOL_var ),
    // 应用交互特征（AIP）
    HeaderType( 0x82 , 0, DOL_b ),
    // 应用标签
    HeaderType( 0x50 , 0, DOL_a),
    // 应用首选名称
    HeaderType( 0x9F12 , 0, DOL_a ),
    // 应用PAN
    HeaderType( 0x5A , 0, DOL_cn),
    // 应用PAN 序列号
    HeaderType( 0x5F34 , 0, DOL_n ),
    // 应用优先指示器
    HeaderType( 0x87 , 0, DOL_b ),
    // 应用交易计数器（ATC）
    HeaderType( 0x9F36 , 0, DOL_b ),
    // 应用用途控制（AUC）
    HeaderType( 0x9F07 , 0, DOL_b ),
    // 应用版本号
    HeaderType( 0x9F08 , 0 ,DOL_b ),
    // 卡片风险管理数据对象列表1（CDOL1）
    HeaderType( 0x8C , 0, DOL_b ),
    // 卡片风险管理数据对象列表2（CDOL2）
    HeaderType( 0x8D , 0, DOL_b ),
    // 持卡人姓名
    HeaderType( 0x5F20 , 0, DOL_ans ),
    // 持卡人姓名扩展
    HeaderType( 0x9F0B , 0, DOL_a ),
    // 持卡人证件号
    HeaderType( 0x9F61 , 0, DOL_ans ),
    // 持卡人证件类型
    HeaderType( 0x9F62 , 0, DOL_cn ),
    // 持卡人验证方法（CVM）列表
    HeaderType( 0x8E , 0, DOL_b ),
    // CA 公钥索引
    HeaderType( 0x8F , 0, DOL_b ),
    // 连续脱机交易限制数（国际-货币）
    HeaderType( 0x9F53 , 0, DOL_a ),
    // 连续脱机交易限制数（国际-国家）
    HeaderType( 0x9F72 , 0, DOL_b ),
    // 密文信息数据（CID）
    HeaderType( 0x9F27 , 0, DOL_b ),
    // 累计脱机交易金额限制数
    HeaderType( 0x9F54 ,0, DOL_n ),
    // 累计脱机交易金额上限
    HeaderType( 0x9F5C , 0, DOL_n ),
    // 累计脱机交易金额限制数（双货币）
    HeaderType( 0x9F75 , 0, DOL_n ),
    // 货币转换因子
    HeaderType( 0x9F73 , 0, DOL_n ),
    // 数据认证码
    HeaderType( 0x9F45 , 0, DOL_b ),
    // 专用（DF）文件名称
    HeaderType( 0x84 ),
    // 目录定义文件
    HeaderType( 0x5D , 0, DOL_cn ),
    // 目录自定义模板
    HeaderType( 0x73 , 0, DOL_struct),
    // 动态数据认证数据对象列表（DDOL）
    HeaderType( 0x9F49 , 0, DOL_b ),
    // 文件控制信息（FCI）发卡行自定义数据
    HeaderType( 0xBF0C , 0, DOL_struct),
    // FCI 专有模板
    HeaderType( 0xA5 , 0, DOL_struct),
    // FCI 模板
    HeaderType( 0x6F , 0, DOL_struct),
    // ICC 动态数
    HeaderType( 0x9F4C , 0, DOL_b ),
    // 公钥证书
    HeaderType( 0x9F47 , 0, DOL_b ),
    // 公钥模数
    HeaderType( 0x9F46 , 0, DOL_b ),
    // 公钥余数
    HeaderType( 0x9F48 , 0, DOL_b ),
    // 发卡行行为代码-缺省
    HeaderType( 0x9F0D , 0, DOL_b ),
    // 发卡行行为代码-拒绝
    HeaderType( 0x9F0E , 0, DOL_b ),
    // 发卡行行为代码-联机
    HeaderType( 0x9F0F , 0, DOL_b ),
    // 发卡行应用数据
    HeaderType( 0x9F10 , 0, DOL_b ),
    // 发卡行认证数据
    HeaderType( 0x91 , 0, DOL_b ),
    // 发卡行认证指示位
    HeaderType( 0x9F56 , 0, DOL_b ),
    // 发卡行代码表索引
    HeaderType( 0x9F11 , 0, DOL_n ),
    // 发卡行国家代码
    HeaderType( 0x5F28 , 0, DOL_n ),
    // 发卡行国家代码
    HeaderType( 0x9F57 , 0, DOL_n ),
    // 发卡行公钥证书
    HeaderType( 0x90 , 0, DOL_b ),
    // 发卡行公钥模数
    HeaderType( 0x9F32 , 0, DOL_b ),
    // 发卡行公钥余数
    HeaderType( 0x92 , 0, DOL_b ),
    // 发卡行脚本模板2
    HeaderType( 0x72 ),
    // 发卡行URL 100页为9F50,89页为5F50
    HeaderType( 0x9F50 , 0, DOL_ans ),
    // 发卡行URL2
    HeaderType( 0x9F5A , 0, DOL_a ),
    // 首选语言
    HeaderType( 0x5F2D , 0, DOL_ans ),
    // 上次联机ATC 寄存器
    HeaderType( 0x9F13 , 0, DOL_b ),
    // 日志入口
    HeaderType( 0x9F4D , 0, DOL_b ),
    // 日志格式
    HeaderType( 0x9F4F , 0, DOL_b ),
    // 连续脱机交易下限
    HeaderType( 0x9F14 , 0, DOL_b ),
    // 连续脱机交易下限
    HeaderType( 0x9F58 , 0, DOL_b ),
    // PIN 尝试计数器
    HeaderType( 0x9F17 , 0, DOL_b ),
    // 处理选项数据对
    HeaderType( 0x9F38 , 0, DOL_b ),
    // 响应报文模板格式1
    HeaderType( 0x80 , 0, DOL_b ),
    // 第2 应用货币代码
    HeaderType( 0x9F76 , 0, DOL_n ),
    // 服务码
    HeaderType( 0x5F30 , 0, DOL_n ),
    // 短文件标识符（SFI）
    HeaderType( 0x88 , 0, DOL_b ),
    // 签名的动态应用数据
    HeaderType( 0x9F4B , 0, DOL_b ),
    // 签名的静态应用数据
    HeaderType( 0x93 , 0, DOL_b ),
    // 静态数据认证标签列表
    HeaderType( 0x9F4A , 0, DOL_b ),
    // 磁条1 自定义数据
    HeaderType( 0x9F1F , 0, DOL_ans ),
    // 磁条2 等效数据
    HeaderType( 0x57 , 0, DOL_b ),
    // 交易证书数据对象列表（TDOL）
    HeaderType( 0x97 , 0, DOL_b ),
    // 连续脱机交易上限
    HeaderType( 0x9F23 , 0, DOL_b ),
    // 连续脱机交易上限
    HeaderType( 0x9F59 , 0, DOL_b ),
    // 卡产品标识信息
    HeaderType( 0x9F63 ),

    /* 其他接口之外的标签 */
    // 应用模板
    HeaderType( 0x61 , 0, DOL_struct),
    // 目录数据名称
    HeaderType( 0x9D , 0, DOL_b ),
    // 发卡行脚本命令
    HeaderType( 0x86 , 0, DOL_b ),
    // 电子现金余额
    HeaderType( 0x9F79, 6, DOL_n),
    // 电子现金余额上限
    HeaderType( 0x9F77, 6, DOL_n),
    // 电子现金发卡行授权码
    HeaderType( 0x9F74, 6, DOL_a),
    // 电子现金单笔交易限额
    HeaderType( 0x9F78, 6, DOL_n),
    // 电子现金重置阈值
    HeaderType( 0x9F6D, 6, DOL_n),
    // 电子现金终端支持指示器
    HeaderType( 0x9F7A, 1, DOL_b),
    // 电子现金终端交易限额
    HeaderType( 0x9F7B, 6, DOL_n),
    // 发卡行脚本
    HeaderType( 0x71 , 0, DOL_struct),
    HeaderType( 0x72 , 0, DOL_struct),
    // 响应报文模板格式2
    HeaderType( 0x77 , 0, DOL_struct),
    // FID
    HeaderType( 0x51 , 0, DOL_b ),
    // C1_Moneta
    HeaderType( 0xC1 ),
    // C2_Moneta
    HeaderType( 0xC2 ),
    // 交易货币指数
    HeaderType( 0x5F36 , 0, DOL_n ),
    // 发卡行脚本标识
    HeaderType( 0x9F18 , 0, DOL_b ),
    // 终端风险管理数据
    HeaderType( 0x9F1D , 0, DOL_b ),
    // 磁条2 自定义数据
    HeaderType( 0x9F20 , 0, DOL_cn ),
    // IC卡PIN加密公钥证书
    HeaderType( 0x9F2D , 0, DOL_b ),
    // IC卡PIN加密公钥指数
    HeaderType( 0x9F2E , 0, DOL_b ),
    // IC卡PIN加密公钥余项
    HeaderType( 0x9F2F , 0, DOL_b ),
    // 应用参考货币
    HeaderType( 0x9F3B , 0, DOL_n ),
    // 应用参考货币指数
    HeaderType( 0x9F43 , 0, DOL_n ),
    // GEO_GRAPHIC_INDICATOR_VISA
    HeaderType( 0x9F55 , 0, DOL_b ),
    // TRM_ISR
    HeaderType( 0x9F5B , 0, DOL_b ),
    // CPLC_HISTORY_FID_VISA
    HeaderType( 0x9F7F , 0, DOL_b ),
    // TRM_SCRIPT
    HeaderType( 0x17 , 0, DOL_b ),
    // TRM_CAPK
    HeaderType( 0x18 , 0, DOL_b ),
    // TRM_PDOL_DATA
    HeaderType( 0x19 , 0, DOL_b ),
    // TRM_CDOL1DATA
    HeaderType( 0x1A , 0, DOL_b ),
    // TRM_CDOL2DATA
    HeaderType( 0x1B , 0, DOL_b ),
    // EOT4
    HeaderType( 0x04 ),
    // 记录标签头
    HeaderType( 0x70 , 0, DOL_struct),
    // 授权码
    HeaderType( 0x89 , 0, DOL_an ),
    // 商户名称
    HeaderType( 0x9F4E, 20, DOL_ans ),
    // 可用脱机消费金额
    HeaderType( 0x9F5D, 0, DOL_n ),
    // 终端交易属性
    HeaderType( 0x9F66, 4, DOL_b ),
    // 卡片附加处理
    HeaderType( 0x9F68, 0, DOL_b ),
    // 卡片持卡人验证方法限制
    HeaderType( 0x9F6B, 0, DOL_n ),
    // 卡片交易属性
    HeaderType( 0x9F6C, 0, DOL_b ),
};
/// _pboc_v2_0_tag_table 数组的大小 
const size_t _pboc_v2_0_tag_table_len = SizeOfArray(_pboc_v2_0_tag_table);
//---------------------------------------------------------
/// 可以通过GET DATA命令获取到数据的标签表
const ushort _pboc_v2_0_get_data_table[] = {
    // 应用货币代码1 0x9F42
    _GET_HEADER(ApplicationCurrencyCode_1),
    // 应用货币代码2 0x9F51
    _GET_HEADER(ApplicationCurrencyCode_2),
    // 应用缺省行为 0x9F52
    _GET_HEADER(ADA),
    // 连续脱机交易限制数（国际-货币）0x9F53
    _GET_HEADER(ContinuousOffLineTransactionLimitCurrency),
    // 连续脱机交易限制数（国际-国家)0x9F72
    _GET_HEADER(ContinuousOffLineTransactionLimitCountry),
    // 累计脱机交易金额限制数 0x9F54
    _GET_HEADER(CumulativeOffLineTransactionsGoldAmountLimit),
    // 累计脱机交易金额限制数（双货币）0x9F75
    _GET_HEADER(CumulativeOffLineTransactionsGoldAmountLimitDoubleCurrency),
    // 累计脱机交易金额上限 0x9F5C
    _GET_HEADER(CumulativeOffLineTransactionsGoldAmountMaximum),
    // 货币转换因子 0x9F73
    _GET_HEADER(CurrencyConversionFactor),
    // 发卡行认证指示位 0x9F56
    _GET_HEADER(IssuerAuthenticationIndicatorBit),
    // 发卡行国家代码 0x5F28
    _GET_HEADER(IssuerCountryCode_1),
    // 发卡行国家代码 0x9F57
    _GET_HEADER(IssuerCountryCode_2),
    // 连续脱机交易下限 0x9F14
    _GET_HEADER(ContinuousOfflineTransactionDownLimit_1),
    // 连续脱机交易下限 0x9F58
    _GET_HEADER(ContinuousOfflineTransactionDownLimit_2),
    // 连续脱机交易上限 0x9F23
    _GET_HEADER(ContinuousOfflineTransactionUpLimit_1),
    // 连续脱机交易上限 0x9F59
    _GET_HEADER(ContinuousOfflineTransactionUpLimit_2),
    // 第2应用货币代码 0x9F76
    _GET_HEADER(SecondGenerationApplicationCurrencyCode),
    /* 金融部分 */
    // 第五部分表格正下方 
    // ATC 交易计数器 0x9F36
    _GET_HEADER(ApplicationTransactionCounter),
    // 上次联机ATC 寄存器 0x9F13
    _GET_HEADER(ATCMailOnlineLatch),
    // PIN 尝试计数器 0x9F17
    _GET_HEADER(PIN_AttemptCounter),
    // 日志格式 0x9F4F
    _GET_HEADER(LogFormat),
    // 第十三部分 
    // 电子现金余额 0x9F79
    _GET_HEADER(EC_Balance),
    // 电子现金余额上限 0x9F77
    _GET_HEADER(EC_BalanceLimit),
    // 电子现金发卡行授权码 0x9F74
    _GET_HEADER(EC_IssuerAuthorizationCode),
    // 电子现金单笔交易限额 0x9F78
    _GET_HEADER(EC_SingleTransactionLimit),
    // 电子现金重置阈值 0x9F6D
    _GET_HEADER(EC_ResetThreshold)
};
/// _pboc_v2_0_get_data_table 数组的大小 
const size_t _pboc_v2_0_get_data_table_len = SizeOfArray(_pboc_v2_0_get_data_table);
//--------------------------------------------------------- 
/// 货币代码转换表
const CurrencyCodeNode  _pboc_v2_0_currencyCodeTable[] = {
    {0x04  , "AFA"}, //Afghanistan  Afghani
    {0x08  , "ALL"}, //Albania  Lek
    {0x012 , "DZD"}, //Algeria  Algerian Dinar
    {0x0840, "USD"}, //American Samoa   US Dollar
    {0x0724, "ESP"}, //Andorra  Spanish Peseta
    {0x0250, "FRF"}, // French Franc
    {0x020 , "ADP"}, // Andorran Peseta
    {0x024 , "AON"}, //Angola   New Kwanza
    {0x0982, "AOR"}, // Kwanza Reajustado
    {0x0951, "XCD"}, //Anguilla East Caribbean Dollar
    {0x0951, "XCD"}, //Antigua and Barbuda  East Caribbean Dollar
    {0x032 , "ARS"}, //Argentina    Argentine Peso
    {0x051 , "AMD"}, //Armenia  Armenian Dram
    {0x0533, "AWG"}, //Aruba    Aruban Guilder
    {0x036 , "AUD"}, //Australia    Australian Dollar
    {0x040 , "ATS"}, //Austria  Schilling
    {0x031 , "AZM"}, //Azerbaijan   Azerbaijanian Manat
    {0x044 , "BSD"}, //Bahamas  Bahamian Dollar
    {0x048 , "BHD"}, //Bahrain  Bahraini Dinar
    {0x050 , "BDT"}, //Bangladesh   Taka
    {0x052 , "BBD"}, //Barbados Barbados Dollar
    {0x0974, "BYR"}, //Belarus  Belarussian Ruble
    {0x056 , "BEF"}, //Belgium  Belgian Franc
    {0x084 , "BZD"}, //Belize   Belize Dollar
    {0x0952, "XOF"}, //Benin    CFA Franc BCEAO
    {0x060 , "BMD"}, //Bermuda  Bermudian Dollar
    {0x0356, "INR"}, //Bhutan   Indian Rupee
    {0x064 , "BTN"}, // Ngultrum
    {0x0977, "BAM"}, //Bosnia and Herzegovina   Convertible Marks
    {0x072 , "BWP"}, //Botswana Pula
    {0x0578, "NOK"}, //Bouvet Island    Norwegian Krone
    {0x0986, "BRL"}, //Brazil   Brazilian Real
    {0x0840, "USD"}, //British Indian Ocean Territory   US Dollar
    {0x096 , "BND"}, //Brunei Darussalam    Brunei Dollar
    {0x0100, "BGL"}, //Bulgaria Lev
    {0x0975, "BGN"}, // Bulgarian LEV
    {0x0952, "XOF"}, //Burkina Faso CFA Franc BCEAO
    {0x0108, "BIF"}, //Burundi  Burundi Franc
    {0x0116, "KHR"}, //Cambodia Riel
    {0x0950, "XAF"}, //Cameroon CFA Franc BEAC
    {0x0124, "CAD"}, //Canada   Canadian Dollar
    {0x0132, "CVE"}, //Cape Verde   Cape Verde Escudo
    {0x0136, "KYD"}, //Cayman Islands   Cayman Islands Dollar
    {0x0950, "XAF"}, //Central African Republic CFA Franc BEAC //Chad CFA Franc BEAC
    {0x0152, "CLP"}, //Chile    Chilean Peso
    {0x0990, "CLF"}, // Unidades de fomento
    {0x0156, "CNY"}, //China    Yuan Renminbi
    {0x036 , "AUD"}, //Christmas Island Australian Dollar //Cocos (Keeling) Islands  Australian Dollar
    {0x0170, "COP"}, //Colombia Colombian Peso
    {0x0174, "KMF"}, //Comoros  Comoro Franc
    {0x0950, "XAF"}, //Congo    CFA Franc BEAC
    {0x0976, "CDF"}, //Congo, The Democratic Republic Of    Franc Congolais
    {0x0554, "NZD"}, //Cook Islands New Zealand Dollar
    {0x0188, "CRC"}, //Costa Rica   Costa Rican Colon
    {0x0952, "XOF"}, //Cote D'ivoire    CFA Franc BCEAO
    {0x0191, "HRK"}, //Croatia  Kuna
    {0x0192, "CUP"}, //Cuba Cuban Peso
    {0x0196, "CYP"}, //Cyprus   Cyprus Pound
    {0x0203, "CZK"}, //Czech Republic   Czech Koruna
    {0x0208, "DKK"}, //Denmark  Danish Krone
    {0x0262, "DJF"}, //Djibouti Djibouti Franc
    {0x0951, "XCD"}, //Dominica East Caribbean Dollar
    {0x0214, "DOP"}, //Dominican Republic   Dominican Peso
    {0x0626, "TPE"}, //East Timor   Timor Escudo
    {0x0360, "IDR"}, // Rupiah
    {0x0218, "ECS"}, //Ecuador  Sucre
    {0x0983, "ECV"}, // Unidad de Valor Constante (UVC)
    {0x0818, "EGP"}, //Egypt    Egyptian Pound
    {0x0222, "SVC"}, //El Salvador  El Salvador Colon
    {0x0950, "XAF"}, //Equatorial Guinea    CFA Franc BEAC
    {0x0233, "EEK"}, //Estonia  Kroon
    {0x0232, "ERN"}, //Eritrea  Nakfa
    {0x0230, "ETB"}, //Ethiopia Ethiopian Birr
    {0x0208, "DKK"}, //Faeroe Islands   Danish Krone
    {0x0238, "FKP"}, //Falkland Islands (Malvinas)  Pound
    {0x0242, "FJD"}, //Fiji Fiji Dollar
    {0x0246, "FIM"}, //Finland  Markka
    {0x0250, "FRF"}, //France   French Franc
    {0x0250, "FRF"}, //French Guiana    French Franc
    {0x0953, "XPF"}, //French Polynesia CFP Franc
    {0x0250, "FRF"}, //French Southern Territories  French Franc
    {0x0950, "XAF"}, //Gabon    CFA Franc BEAC
    {0x0270, "GMD"}, //Gambia   Dalasi
    {0x0981, "GEL"}, //Georgia  Lari
    {0x0280, "DEM"}, //Germany  Deutsche Mark
    {0x0288, "GHC"}, //Ghana    Cedi
    {0x0292, "GIP"}, //Gibraltar    Gibraltar Pound
    {0x0300, "GRD"}, //Greece   Drachma
    {0x0208, "DKK"}, //Greenland    Danish Krone
    {0x0951, "XCD"}, //Grenada  East Caribbean Dollar
    {0x0250, "FRF"}, //Guadeloupe   French Franc
    {0x0840, "USD"}, //Guam US Dollar
    {0x0320, "GTQ"}, //Guatemala    Quetzal
    {0x0324, "GNF"}, //Guinea   Guinea Franc
    {0x0624, "GWP"}, //Guinea-Bissau    Guinea-Bissau Peso
    {0x0952, "XOF"}, // CFA Franc BCEAO
    {0x0328, "GYD"}, //Guyana   Guyana Dollar
    {0x0332, "HTG"}, //Haiti    Gourde
    {0x0840, "USD"}, // US Dollar
    {0x036 , "AUD"}, //Heard and Mcdonald Islands   Australian Dollar
    {0x0340, "HNL"}, //Honduras Lempira
    {0x0344, "HKD"}, //Hong Kong    Hong Kong Dollar
    {0x0348, "HUF"}, //Hungary  Forint
    {0x0352, "ISK"}, //Iceland  Iceland Krona
    {0x0356, "INR"}, //India    Indian Rupee
    {0x0360, "IDR"}, //Indonesia    Rupiah
    {0x0960, "XDR"}, //International Monetary Fund (Imf)    SDR
    {0x0364, "IRR"}, //Iran (Islamic Republic Of)   Iranian Rial
    {0x0368, "IQD"}, //Iraq Iraqi Dinar
    {0x0372, "IEP"}, //Ireland  Irish Pound
    {0x0376, "ILS"}, //Israel   New Israeli Sheqel
    {0x0380, "ITL"}, //Italy    Italian Lira
    {0x0388, "JMD"}, //Jamaica  Jamaican Dollar
    {0x0392, "JPY"}, //Japan    Yen
    {0x0400, "JOD"}, //Jordan   Jordanian Dinar
    {0x0398, "KZT"}, //Kazakhstan   Tenge
    {0x0404, "KES"}, //Kenya    Kenyan Shilling
    {0x036 , "AUD"}, //Kiribati Australian Dollar
    {0x0408, "KPW"}, //Korea, Democratic People's Republic Of   North Korean Won
    {0x0410, "KRW"}, //Korea, Republic Of   Won
    {0x0414, "KWD"}, //Kuwait   Kuwaiti Dinar
    {0x0417, "KGS"}, //Kyrgyzstan   Som
    {0x0418, "LAK"}, //Lao People's Democratic Republic Kip
    {0x0428, "LVL"}, //Latvia   Latvian Lats
    {0x0422, "LBP"}, //Lebanon  Lebanese Pound
    {0x0710, "ZAR"}, //Lesotho  Rand
    {0x0991, "ZAL"}, // (financial Rand)
    {0x0426, "LSL"}, // Loti
    {0x0430, "LRD"}, //Liberia  Liberian Dollar
    {0x0434, "LYD"}, //Libyan Arab Jamahiriya   Libyan Dinar
    {0x0756, "CHF"}, //Liechtenstein    Swiss Franc
    {0x0440, "LTL"}, //Lithuania    Lithuanian Litas
    {0x0442, "LUF"}, //Luxembourg   Luxembourg Franc
    {0x0446, "MOP"}, //Macau    Pataca
    {0x0807, "MKD"}, //Macedonia, The Former Yugoslav Republic Of   Denar
    {0x0450, "MGF"}, //Madagascar   Malagasy Franc
    {0x0454, "MWK"}, //Malawi   Kwacha
    {0x0458, "MYR"}, //Malaysia Malaysian Ringgit
    {0x0462, "MVR"}, //Maldives Rufiyaa
    {0x0952, "XOF"}, //Mali CFA Franc BCEAO
    {0x0470, "MTL"}, //Malta    Maltese Lira
    {0x0840, "USD"}, //Marshall Islands US Dollar
    {0x0250, "FRF"}, //Martinique   French Franc
    {0x0478, "MRO"}, //Mauritania   Ouguiya
    {0x0480, "MUR"}, //Mauritius    Mauritius Rupee
    {0x0484, "MXN"}, //Mexico   Mexican Peso
    {0x0979, "MXV"}, // Mexican Unidad de Inversion (UDI)
    {0x0840, "USD"}, //Micronesia   US Dollar
    {0x0498, "MDL"}, //Moldova, Republic Of Moldovan Leu
    {0x0250, "FRF"}, //Monaco   French Franc
    {0x0496, "MNT"}, //Mongolia Tugrik
    {0x0951, "XCD"}, //Montserrat   East Caribbean Dollar
    {0x0504, "MAD"}, //Morocco  Moroccan Dirham
    {0x0508, "MZM"}, //Mozambique   Metical
    {0x0104, "MMK"}, //Myanmar  Kyat
    {0x0710, "ZAR"}, //Namibia  Rand
    {0x0516, "NAD"}, // Namibia Dollar
    {0x036 , "AUD"}, //Nauru    Australian Dollar
    {0x0524, "NPR"}, //Nepal    Nepalese Rupee
    {0x0528, "NLG"}, //Netherlands  Netherlands Guilder
    {0x0532, "ANG"}, //Netherlands Antilles Antillian Guilder
    {0x0953, "XPF"}, //New Caledonia    CFP Franc
    {0x0554, "NZD"}, //New Zealand  New Zealand Dollar
    {0x0558, "NIO"}, //Nicaragua    Cordoba Oro
    {0x0952, "XOF"}, //Niger    CFA Franc BCEAO
    {0x0566, "NGN"}, //Nigeria  Naira
    {0x0554, "NZD"}, //Niue New Zealand Dollar
    {0x036 , "AUD"}, //Norfolk Island   Australian Dollar
    {0x0840, "USD"}, //Northern Mariana Islands US Dollar
    {0x0578, "NOK"}, //Norway   Norwegian Krone
    {0x0512, "OMR"}, //Oman Rial Omani
    {0x0586, "PKR"}, //Pakistan Pakistan Rupee
    {0x0840, "USD"}, //Palau    US Dollar
    {0x0590, "PAB"}, //Panama   Balboa
    {0x0840, "USD"}, // US Dollar
    {0x0598, "PGK"}, //Papua New Guinea Kina
    {0x0600, "PYG"}, //Paraguay Guarani
    {0x0604, "PEN"}, //Peru Nuevo Sol
    {0x0608, "PHP"}, //Philippines  Philippine Peso
    {0x0554, "NZD"}, //Pitcairn New Zealand Dollar
    {0x0985, "PLN"}, //Poland   Zloty
    {0x0620, "PTE"}, //Portugal Portuguese Escudo
    {0x0840, "USD"}, //Puerto Rico  US Dollar
    {0x0634, "QAR"}, //Qatar    Qatari Rial
    {0x0250, "FRF"}, //Reunion  French Franc
    {0x0642, "ROL"}, //Romania  Leu
    {0x0810, "RUR"}, //Russian Federation   Russian Ruble
    {0x0643, "RUB"}, // Russian Ruble
    {0x0646, "RWF"}, //Rwanda   Rwanda Franc
    {0x0654, "SHP"}, //St Helena    St Helena Pound
    {0x0951, "XCD"}, //St Kitts - Nevis East Caribbean Dollar
    {0x0951, "XCD"}, //Saint Lucia  East Caribbean Dollar
    {0x0250, "FRF"}, //St Pierre and Miquelon   French Franc
    {0x0951, "XCD"}, //Saint Vincent and The Grenadines East Caribbean Dollar
    {0x0882, "WST"}, //Samoa    Tala
    {0x0380, "ITL"}, //San Marino   Italian Lira
    {0x0678, "STD"}, //Sao Tome and Principe    Dobra
    {0x0682, "SAR"}, //Saudi Arabia Saudi Riyal
    {0x0952, "XOF"}, //Senegal  CFA Franc BCEAO
    {0x0690, "SCR"}, //Seychelles   Seychelles Rupee
    {0x0694, "SLL"}, //Sierra Leone Leone
    {0x0702, "SGD"}, //Singapore    Singapore Dollar
    {0x0703, "SKK"}, //Slovakia Slovak Koruna
    {0x0705, "SIT"}, //Slovenia Tolar
    {0x090 , "SBD"}, //Solomon Islands  Solomon Islands Dollar
    {0x0706, "SOS"}, //Somalia  Somali Shilling
    {0x0710, "ZAR"}, //South Africa Rand
    {0x0724, "ESP"}, //Spain    Spanish Peseta
    {0x0144, "LKR"}, //Sri Lanka    Sri Lanka Rupee
    {0x0736, "SDD"}, //Sudan    Sudanese Dinar
    {0x0740, "SRG"}, //Suriname Surinam Guilder
    {0x0578, "NOK"}, //Svalbard and Jan Mayen Islands   Norwegian Krone
    {0x0748, "SZL"}, //Swaziland    Lilangeni
    {0x0752, "SEK"}, //Sweden   Swedish Krona
    {0x0756, "CHF"}, //Switzerland  Swiss Franc
    {0x0760, "SYP"}, //Syrian Arab Republic Syrian Pound
    {0x0901, "TWD"}, //Taiwan, Province Of China    New Taiwan Dollar
    {0x0972, "TJS"}, //Tajikistan   Somoni
    {0x0762, "TJR"}, //Tajikistan (Old) Tajik Ruble (old)
    {0x0834, "TZS"}, //Tanzania, United Republic Of Tanzanian Shilling
    {0x0764, "THB"}, //Thailand Baht
    {0x0952, "XOF"}, //Togo CFA Franc BCEAO
    {0x0554, "NZD"}, //Tokelau  New Zealand Dollar
    {0x0776, "TOP"}, //Tonga    Pa'anga
    {0x0780, "TTD"}, //Trinidad and Tobago  Trinidad and Tobago Dollar
    {0x0788, "TND"}, //Tunisia  Tunisian Dinar
    {0x0792, "TRL"}, //Turkey   Turkish Lira
    {0x0795, "TMM"}, //Turkmenistan Manat
    {0x0840, "USD"}, //Turks and Caicos Islands US Dollar
    {0x036 , "AUD"}, //Tuvalu   Australian Dollar
    {0x0800, "UGX"}, //Uganda   Uganda Shilling
    {0x0980, "UAH"}, //Ukraine  Hryvnia
    {0x0784, "AED"}, //United Arab Emirates UAE Dirham
    {0x0826, "GBP"}, //United Kingdom   Pound Sterling
    {0x0840, "USD"}, //United States    US Dollar
    {0x0998, "USS"}, // (Same day)
    {0x0997, "USN"}, // (Next day)
    {0x0840, "USD"}, //United States Minor Outlaying Islands    US Dollar
    {0x0858, "UYU"}, //Uruguay  Peso Uruguayo
    {0x0860, "UZS"}, //Uzbekistan   Uzbekistan Sum
    {0x0548, "VUV"}, //Vanuatu  Vatu
    {0x0380, "ITL"}, //Vatican City State (Holy See)    Italian Lira
    {0x0862, "VEB"}, //Venezuela    Bolivar
    {0x0704, "VND"}, //Vietnam  Dong
    {0x0840, "USD"}, //Virgin Islands (British) US Dollar
    {0x0840, "USD"}, //Virgin Islands (U.S.)    US Dollar
    {0x0953, "XPF"}, //Wallis and Futuna Islands    CFP Franc
    {0x0504, "MAD"}, //Western Sahara   Moroccan Dirham
    {0x0886, "YER"}, //Yemen    Yemeni Rial
    {0x0891, "YUM"}, //Yugoslavia   New Dinar
    {0x0180, "ZRN"}, //Zaire    New Zaire
    {0x0894, "ZMK"}, //Zambia   Kwacha
    {0x0716, "ZWD"}, //Zimbabwe Zimbabwe Dollar
    {0x0978, "EUR"}, //Euro
    {0x0000, "???"}, //Unknown
};
/// _pboc_v2_0_currencyCodeTable 数组的大小 
const size_t _pboc_v2_0_currencyCodeTable_len = SizeOfArray(_pboc_v2_0_currencyCodeTable);
//--------------------------------------------------------- 
} // namespace v2_0 
} // namespace pboc
} // namesapce iccard 
} // namesapce device 
} // namespace zhou_yb
//========================================================= 