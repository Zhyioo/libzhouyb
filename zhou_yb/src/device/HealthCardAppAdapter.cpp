//========================================================= 
/**@file HealthCardAppAdapter.cpp 
 * @brief 居民健康卡应用客户端操作逻辑实现 
 * 
 * @date 2014-10-03   11:55:49 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../device/iccard/health_card/HealthCardAppAdapter.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace health_card {
//--------------------------------------------------------- 
/// 卡片密钥对照表 
//--------------------------------------------------------- 
    /// 转换对应的数据表名 
#define FID_DATA_TABLE(fid) _fid_data_##fid
    /// 数据表的长度 
#define FID_DATA_TABLE_LEN(fid) SizeOfArray(FID_DATA_TABLE(fid))
//@{
/**@name 
 * @brief 各文件数据对照表  
 */ 
/// MF.EF05
const FID_DATA_MSG FID_DATA_TABLE(EF05)[] = 
{
    {  1, "卡类别",       DOL_ans,   1 }, 
    {  2, "规范版本",     DOL_ans,   4 }, 
    {  3, "发卡机构名称", DOL_ans,  30 }, 
    {  4, "发卡机构代码", DOL_cn,   11 }, 
    {  5, "发卡机构证书", DOL_b,   180 }, 
    {  6, "发卡时间",     DOL_cn,    4 }, 
    {  8, "卡号",         DOL_ans,  18 }, 
    {  9, "安全码",       DOL_ans,   3 }, 
    { 10, "发卡序列号",   DOL_ans,  10 }, 
    { 57, "应用城市代码", DOL_cn,    3 }
};
/// MF.EF06
const FID_DATA_MSG FID_DATA_TABLE(EF06)[] = 
{
    { 11, "姓名",         DOL_ans,  30 }, 
    { 12, "性别",         DOL_b,     1 }, 
    { 13, "民族代码",     DOL_cn,    1 }, 
    { 14, "出生日期",     DOL_cn,    4 }, 
    { 15, "身份证号码",   DOL_ans,  18 }
};
/// MF.EF07
const FID_DATA_MSG FID_DATA_TABLE(EF07)[] = 
{
    { 0, "照片", DOL_b, 3074 }
};
/// MF.EF08
const FID_DATA_MSG FID_DATA_TABLE(EF08)[] = 
{
    {  7, "卡有效期",         DOL_cn,    4 }, 
    { 16, "本人电话1",        DOL_ans,  20 }, 
    { 17, "本人电话2",        DOL_ans,  20 }, 
    { 18, "医疗费用支付方式1",DOL_cn,    1 }, 
    { 19, "医疗费用支付方式2",DOL_cn,    1 },
    { 20, "医疗费用支付方式3",DOL_cn,    1 }
};
/// DF01.EF05
const FID_DATA_MSG FID_DATA_TABLE(DF01_EF05)[] = 
{
    { 21, "地址类别1", DOL_cn,    1 }, 
    { 22, "地址1",     DOL_ans, 100 }, 
    { 23, "地址类别2", DOL_cn,    1 }, 
    { 24, "地址2",     DOL_ans, 100 }
};
/// DF01.EF06
const FID_DATA_MSG FID_DATA_TABLE(DF01_EF06)[] = 
{
    { 25, "联系人姓名1", DOL_ans,    30 }, 
    { 26, "联系人关系1", DOL_cn,      1 }, 
    { 27, "联系人电话1", DOL_ans,    20 }, 
    { 28, "联系人姓名2", DOL_ans,    30 }, 
    { 29, "联系人关系2", DOL_cn,      1 }, 
    { 30, "联系人电话2", DOL_ans,    20 }, 
    { 31, "联系人姓名3", DOL_ans,    30 }, 
    { 32, "联系人关系3", DOL_cn,      1 }, 
    { 33, "联系人电话3", DOL_ans,    20 } 
};
/// DF01.EF07
const FID_DATA_MSG FID_DATA_TABLE(DF01_EF07)[] = 
{
    { 34, "文化程度代码", DOL_cn,    1 }, 
    { 35, "婚姻情况代码", DOL_cn,    1 }, 
    { 36, "职业代码",     DOL_ans,   3 }
};
/// DF01.EF08
const FID_DATA_MSG FID_DATA_TABLE(DF01_EF08)[] = 
{
    { 37, "证件类别",       DOL_cn,     1 }, 
    { 38, "证件号码",       DOL_ans,   18 }, 
    { 39, "健康档案编号",   DOL_ans,   17 }, 
    { 40, "新农合证(卡)号", DOL_ans,   18 }
};
/// DF02.EF05
const FID_DATA_MSG FID_DATA_TABLE(DF02_EF05)[] = 
{
    { 41, "ABO血型代码",     DOL_b,    1 },
    { 42, "RH血型代码",      DOL_cn,   1 },
    { 43, "哮喘标志",        DOL_b,    1 },
    { 44, "心脏病标志",      DOL_b,    1 },
    { 45, "心脑血管病标志",  DOL_b,    1 },
    { 46, "癫痫病标志",      DOL_b,    1 },
    { 47, "凝血紊乱标志",    DOL_b,    1 },
    { 48, "糖尿病标志",      DOL_b,    1 },
    { 49, "青光眼标志",      DOL_b,    1 },
    { 50, "透析标志",        DOL_b,    1 },
    { 51, "器官移植标志",    DOL_b,    1 },
    { 52, "器官缺失标志",    DOL_b,    1 },
    { 53, "可装卸的义肢标志",DOL_b,    1 },
    { 54, "心脏起搏器标志",  DOL_b,    1 },
    { 55, "其他医学警示名称",DOL_ans, 40 }
};
/// DF02.EF06
const FID_DATA_MSG FID_DATA_TABLE(DF02_EF06)[] = 
{
    { 56, "精神病标志", DOL_b, 1 }
};
/// DF02.EF07
const FID_DATA_MSG FID_DATA_TABLE(DF02_EF07)[] = 
{
    { 0, "过敏物质名称", DOL_ans,  20 },
    { 0, "过敏反应",     DOL_ans, 100 }
};
/// DF02.EF08
const FID_DATA_MSG FID_DATA_TABLE(DF02_EF08)[] = 
{
    { 0, "免疫接种名称", DOL_ans,  20 },
    { 0, "免疫接种时间", DOL_cn,   04 }
};
/// DF03.EF05
const FID_DATA_MSG FID_DATA_TABLE(DF03_EF05)[] = 
{
    { 0, "住院记录有效标志", DOL_b,  1 }
};
/// DF03.EF06
const FID_DATA_MSG FID_DATA_TABLE(DF03_EF06)[] = 
{
    { 0, "门诊记录有效标志", DOL_b,  1 }
};
/// DF03.EE00(EE01-EE03)
const FID_DATA_MSG FID_DATA_TABLE(DF03_EE00)[] = 
{
    { 0, "住院机构名称",                   DOL_ans, 70 },
    { 0, "住院机构组织机构代码",           DOL_ans, 10 },
    { 0, "入院日期",                       DOL_cn,   4 },
    { 0, "住院患者住院次数",               DOL_cn,   2 },
    { 0, "病案号",                         DOL_ans, 18 },
    { 0, "住院患者入院科室名称",           DOL_ans, 50 },
    { 0, "住院患者入院病情",               DOL_cn,   1 },
    { 0, "住院患者医院感染名称",           DOL_ans, 50 },
    { 0, "住院患者损伤和中毒外部原因",     DOL_ans,  7 },
    { 0, "住院患者血清学检查项目代码1",    DOL_cn,   1 },
    { 0, "住院患者血清学检查结果代码1",    DOL_cn,   1 },
    { 0, "疾病诊断名称1",                  DOL_ans, 50 },
    { 0, "疾病诊断代码1",                  DOL_ans,  7 },
    { 0, "确诊日期1",                      DOL_cn,   4 },
    { 0, "住院患者诊断符合情况-详细描述1", DOL_ans, 20 },
    { 0, "住院患者诊断符合情况-代码1",     DOL_cn,   1 },
    { 0, "住院患者疾病诊断类型-详细描述1", DOL_ans, 20 },
    { 0, "住院患者疾病诊断类型-代码1",     DOL_cn,   1 },
    { 0, "住院患者治疗结果代码1",          DOL_cn,   1 },
    { 0, "手术/操作-名称1",                DOL_ans, 80 },
    { 0, "手术/操作-代码1",                DOL_ans,  5 },
    { 0, "手术/操作-日期1",                DOL_cn,   4 },
    { 0, "麻醉-方法1",                     DOL_ans, 50 },
    { 0, "麻醉-方法代码1",                 DOL_cn,   1 },
    { 0, "手术切口愈合等级代码1",          DOL_cn,   1 },
    { 0, "住院患者血清学检查项目代码2",    DOL_cn,   1 },
    { 0, "住院患者血清学检查结果代码2",    DOL_cn,   1 },
    { 0, "疾病诊断名称2",                  DOL_ans, 50 },
    { 0, "疾病诊断代码2",                  DOL_ans,  7 },
    { 0, "确诊日期2",                      DOL_cn,   4 },
    { 0, "住院患者诊断符合情况-详细描述2", DOL_ans, 20 },
    { 0, "住院患者诊断符合情况-代码2",     DOL_cn,   1 },
    { 0, "住院患者疾病诊断类型-详细描述2", DOL_ans, 20 },
    { 0, "住院患者疾病诊断类型-代码2",     DOL_cn,   1 },
    { 0, "住院患者治疗结果代码2",          DOL_cn,   1 },
    { 0, "手术/操作-名称2",                DOL_ans, 80 },
    { 0, "手术/操作-代码2",                DOL_ans,  5 },
    { 0, "手术/操作-日期2",                DOL_cn,   4 },
    { 0, "麻醉-方法2",                     DOL_ans, 50 },
    { 0, "麻醉-方法代码2",                 DOL_cn,   1 },
    { 0, "手术切口愈合等级代码2",          DOL_cn,   1 },
    { 0, "住院患者血清学检查项目代码3",    DOL_cn,   1 },
    { 0, "住院患者血清学检查结果代码3",    DOL_cn,   1 },
    { 0, "疾病诊断名称3",                  DOL_ans, 50 },
    { 0, "疾病诊断代码3",                  DOL_ans,  7 },
    { 0, "确诊日期3",                      DOL_cn,   4 },
    { 0, "住院患者诊断符合情况-详细描述3", DOL_ans, 20 },
    { 0, "住院患者诊断符合情况-代码3",     DOL_cn,   1 },
    { 0, "住院患者疾病诊断类型-详细描述3", DOL_ans, 20 },
    { 0, "住院患者疾病诊断类型-代码3",     DOL_cn,   1 },
    { 0, "住院患者治疗结果代码3",          DOL_cn,   1 },
    { 0, "手术/操作-名称3",                DOL_ans, 80 },
    { 0, "手术/操作-代码3",                DOL_ans,  5 },
    { 0, "手术/操作-日期3",                DOL_cn,   4 },
    { 0, "麻醉-方法3",                     DOL_ans, 50 },
    { 0, "麻醉-方法代码3",                 DOL_cn,   1 },
    { 0, "手术切口愈合等级代码3",          DOL_cn,   1 },
    { 0, "住院期间输血品种代码1",          DOL_cn,   1 },
    { 0, "住院期间输血量1",                DOL_cn,   2 },
    { 0, "住院患者输血量计量单位1",        DOL_ans, 10 },
    { 0, "住院期间输血品种代码2",          DOL_cn,   1 },
    { 0, "住院期间输血量2",                DOL_cn,   2 },
    { 0, "住院患者输血量计量单位2",        DOL_ans, 10 },
    { 0, "住院期间输血品种代码3",          DOL_cn,   1 },
    { 0, "住院期间输血量3",                DOL_cn,   2 },
    { 0, "住院患者输血量计量单位3",        DOL_ans, 10 },
    { 0, "住院期间输血品种代码4",          DOL_cn,   1 },
    { 0, "住院期间输血量4",                DOL_cn,   2 },
    { 0, "住院患者输血量计量单位4",        DOL_ans, 10 },
    { 0, "住院患者抢救次数",               DOL_cn,   2 },
    { 0, "住院患者抢救成功次数",           DOL_cn,   2 },
    { 0, "出院日期",                       DOL_cn,   4 },
    { 0, "住院患者出院科室名称",           DOL_ans, 50 },
    { 0, "住院患者住院天数",               DOL_cn,   3 },
    { 0, "住院患者尸检标志",               DOL_b,    1 },
    { 0, "住院患者随诊标志",               DOL_b,    1 },
    { 0, "住院费用-医疗付款方式代码",      DOL_cn,   1 },
    { 0, "住院费用-分类1",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码1",             DOL_ans,  1 },
    { 0, "住院费用-金额1",                 DOL_cn,   5 },
    { 0, "住院费用-分类2",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码2",             DOL_ans,  1 },
    { 0, "住院费用-金额2",                 DOL_cn,   5 },
    { 0, "住院费用-分类3",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码3",             DOL_ans,  1 },
    { 0, "住院费用-金额3",                 DOL_cn,   5 },
    { 0, "住院费用-分类4",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码4",             DOL_ans,  1 },
    { 0, "住院费用-金额4",                 DOL_cn,   5 },
    { 0, "住院费用-分类5",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码5",             DOL_ans,  1 },
    { 0, "住院费用-金额5",                 DOL_cn,   5 },
    { 0, "住院费用-分类6",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码6",             DOL_ans,  1 },
    { 0, "住院费用-金额6",                 DOL_cn,   5 },
    { 0, "住院费用-分类7",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码7",             DOL_ans,  1 },
    { 0, "住院费用-金额7",                 DOL_cn,   5 },
    { 0, "住院费用-分类8",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码8",             DOL_ans,  1 },
    { 0, "住院费用-金额8",                 DOL_cn,   5 },
    { 0, "住院费用-分类9",                 DOL_ans, 20 },
    { 0, "住院费用-分类代码9",             DOL_ans,  1 },
    { 0, "住院费用-金额9",                 DOL_cn,   5 },
    { 0, "住院费用-分类10",                DOL_ans, 20 },
    { 0, "住院费用-分类代码10",            DOL_ans,  1 },
    { 0, "住院费用-金额10",                DOL_cn,   5 },
    { 0, "住院费用-分类11",                DOL_ans, 20 },
    { 0, "住院费用-分类代码11",            DOL_ans,  1 },
    { 0, "住院费用-金额11",                DOL_cn,   5 },
    { 0, "住院费用-分类12",                DOL_ans, 20 },
    { 0, "住院费用-分类代码12",            DOL_ans,  1 },
    { 0, "住院费用-金额12",                DOL_cn,   5 },
    { 0, "住院费用-分类13",                DOL_ans, 20 },
    { 0, "住院费用-分类代码13",            DOL_ans,  1 },
    { 0, "住院费用-金额13",                DOL_cn,   5 },
    { 0, "住院费用-分类14",                DOL_ans, 20 },
    { 0, "住院费用-分类代码14",            DOL_ans,  1 },
    { 0, "住院费用-金额14",                DOL_cn,   5 },
    { 0, "住院费用-分类15",                DOL_ans, 20 },
    { 0, "住院费用-分类代码15",            DOL_ans,  1 },
    { 0, "住院费用-金额15",                DOL_cn,   5 },
    { 0, "住院费用-分类16",                DOL_ans, 20 },
    { 0, "住院费用-分类代码16",            DOL_ans,  1 },
    { 0, "住院费用-金额16",                DOL_cn,   5 },
    { 0, "住院费用-分类17",                DOL_ans, 20 },
    { 0, "住院费用-分类代码17",            DOL_ans,  1 },
    { 0, "住院费用-金额17",                DOL_cn,   5 },
    { 0, "住院费用-分类18",                DOL_ans, 20 },
    { 0, "住院费用-分类代码18",            DOL_ans,  1 },
    { 0, "住院费用-金额18",                DOL_cn,   5 },
    { 0, "住院费用-分类19",                DOL_ans, 20 },
    { 0, "住院费用-分类代码19",            DOL_ans,  1 },
    { 0, "住院费用-金额19",                DOL_cn,   5 },
    { 0, "住院费用-分类20",                DOL_ans, 20 },
    { 0, "住院费用-分类代码20",            DOL_ans,  1 },
    { 0, "住院费用-金额20",                DOL_cn,   5 },
    { 0, "住院总费用",                     DOL_cn,   5 },
    { 0, "床位费",                         DOL_cn,   5 },
    { 0, "住院护理费",                     DOL_cn,   5 },
    { 0, "住院西药费",                     DOL_cn,   5 },
    { 0, "住院中药费",                     DOL_cn,   5 },
    { 0, "住院化验费",                     DOL_cn,   5 },
    { 0, "住院诊疗费",                     DOL_cn,   5 },
    { 0, "住院手术费",                     DOL_cn,   5 },
    { 0, "住院检查费",                     DOL_cn,   5 },
    { 0, "其他住院费用",                   DOL_cn,   5 },
    { 0, "交易信息签名",                   DOL_b,   64 },
    { 0, "SAM卡证书",                      DOL_b,  190 }
};
/// DF03.ED00(ED01-ED05)
const FID_DATA_MSG FID_DATA_TABLE(DF03_ED00)[] = 
{
    { 0, "就诊机构名称",                   DOL_ans, 70 },
    { 0, "就诊机构组织机构代码",           DOL_ans, 10 },
    { 0, "就诊日期时间",                   DOL_cn,   7 },
    { 0, "门诊号",                         DOL_ans, 18 },
    { 0, "就医科室名称",                   DOL_ans, 50 },
    { 0, "医疗付款方式",                   DOL_cn,   1 },
    { 0, "症状名称1",                      DOL_ans, 50 },
    { 0, "症状代码1",                      DOL_ans,  5 },
    { 0, "诊断日期1",                      DOL_cn,   4 },
    { 0, "门诊诊断名称1",                  DOL_ans, 50 },
    { 0, "门诊诊断代码1",                  DOL_ans,  7 },
    { 0, "发病日期时间1",                  DOL_cn,   7 },
    { 0, "症状持续时间1",                  DOL_cn,   2 },
    { 0, "症状名称2",                      DOL_ans, 50 },
    { 0, "症状代码2",                      DOL_ans,  5 },
    { 0, "诊断日期2",                      DOL_cn,   4 },
    { 0, "门诊诊断名称2",                  DOL_ans, 50 },
    { 0, "门诊诊断代码2",                  DOL_ans,  7 },
    { 0, "发病日期时间2",                  DOL_cn,   7 },
    { 0, "症状持续时间2",                  DOL_cn,   2 },
    { 0, "症状名称3",                      DOL_ans, 50 },
    { 0, "症状代码3",                      DOL_ans,  5 },
    { 0, "诊断日期3",                      DOL_cn,   4 },
    { 0, "门诊诊断名称3",                  DOL_ans, 50 },
    { 0, "门诊诊断代码3",                  DOL_ans,  7 },
    { 0, "发病日期时间3",                  DOL_cn,   7 },
    { 0, "症状持续时间3",                  DOL_cn,   2 },
    { 0, "症状名称4",                      DOL_ans, 50 },
    { 0, "症状代码4",                      DOL_ans,  5 },
    { 0, "诊断日期4",                      DOL_cn,   4 },
    { 0, "门诊诊断名称4",                  DOL_ans, 50 },
    { 0, "门诊诊断代码4",                  DOL_ans,  7 },
    { 0, "发病日期时间4",                  DOL_cn,   7 },
    { 0, "症状持续时间4",                  DOL_cn,   2 },
    { 0, "症状名称5",                      DOL_ans, 50 },
    { 0, "症状代码5",                      DOL_ans,  5 },
    { 0, "诊断日期5",                      DOL_cn,   4 },
    { 0, "门诊诊断名称5",                  DOL_ans, 50 },
    { 0, "门诊诊断代码5",                  DOL_ans,  7 },
    { 0, "发病日期时间5",                  DOL_cn,   7 },
    { 0, "症状持续时间5",                  DOL_cn,   2 },
    { 0, "检查/检验项目名称1",             DOL_ans, 80 },
    { 0, "检查/检验结果代码1",             DOL_cn,   1 },
    { 0, "检查/检验定量结果1",             DOL_cn,   5 },
    { 0, "检查/检验计量单位1",             DOL_ans, 20 },
    { 0, "检查/检验项目代码1",             DOL_ans, 20 },
    { 0, "检查/检验项目名称2",             DOL_ans, 80 },
    { 0, "检查/检验结果代码2",             DOL_cn,   1 },
    { 0, "检查/检验定量结果2",             DOL_cn,   5 },
    { 0, "检查/检验计量单位2",             DOL_ans, 20 },
    { 0, "检查/检验项目代码2",             DOL_ans, 20 },
    { 0, "检查/检验项目名称3",             DOL_ans, 80 },
    { 0, "检查/检验结果代码3",             DOL_cn,   1 },
    { 0, "检查/检验定量结果3",             DOL_cn,   5 },
    { 0, "检查/检验计量单位3",             DOL_ans, 20 },
    { 0, "检查/检验项目代码3",             DOL_ans, 20 },
    { 0, "检查/检验项目名称4",             DOL_ans, 80 },
    { 0, "检查/检验结果代码4",             DOL_cn,   1 },
    { 0, "检查/检验定量结果4",             DOL_cn,   5 },
    { 0, "检查/检验计量单位4",             DOL_ans, 20 },
    { 0, "检查/检验项目代码4",             DOL_ans, 20 },
    { 0, "检查/检验项目名称5",             DOL_ans, 80 },
    { 0, "检查/检验结果代码5",             DOL_cn,   1 },
    { 0, "检查/检验定量结果5",             DOL_cn,   5 },
    { 0, "检查/检验计量单位5",             DOL_ans, 20 },
    { 0, "检查/检验项目代码5",             DOL_ans, 20 },
    { 0, "检查/检验项目名称6",             DOL_ans, 80 },
    { 0, "检查/检验结果代码6",             DOL_cn,   1 },
    { 0, "检查/检验定量结果6",             DOL_cn,   5 },
    { 0, "检查/检验计量单位6",             DOL_ans, 20 },
    { 0, "检查/检验项目代码6",             DOL_ans, 20 },
    { 0, "检查/检验项目名称7",             DOL_ans, 80 },
    { 0, "检查/检验结果代码7",             DOL_cn,   1 },
    { 0, "检查/检验定量结果7",             DOL_cn,   5 },
    { 0, "检查/检验计量单位7",             DOL_ans, 20 },
    { 0, "检查/检验项目代码7",             DOL_ans, 20 },
    { 0, "检查/检验项目名称8",             DOL_ans, 80 },
    { 0, "检查/检验结果代码8",             DOL_cn,   1 },
    { 0, "检查/检验定量结果8",             DOL_cn,   5 },
    { 0, "检查/检验计量单位8",             DOL_ans, 20 },
    { 0, "检查/检验项目代码8",             DOL_ans, 20 },
    { 0, "检查/检验项目名称9",             DOL_ans, 80 },
    { 0, "检查/检验结果代码9",             DOL_cn,   1 },
    { 0, "检查/检验定量结果9",             DOL_cn,   5 },
    { 0, "检查/检验计量单位9",             DOL_ans, 20 },
    { 0, "检查/检验项目代码9",             DOL_ans, 20 },
    { 0, "检查/检验项目名称10",            DOL_ans, 80 },
    { 0, "检查/检验结果代码10",            DOL_cn,   1 },
    { 0, "检查/检验定量结果10",            DOL_cn,   5 },
    { 0, "检查/检验计量单位10",            DOL_ans, 20 },
    { 0, "检查/检验项目代码10",            DOL_ans, 20 },
    { 0, "药物名称1",                      DOL_ans, 50 },
    { 0, "药物剂型代码1",                  DOL_cn,   1 },
    { 0, "用药天数1",                      DOL_cn,   3 },
    { 0, "药物使用频率1",                  DOL_ans, 20 },
    { 0, "药物使用剂量单位1",              DOL_ans,  6 },
    { 0, "药物使用次剂量1",                DOL_cn,   3 },
    { 0, "药物使用总剂量1",                DOL_cn,   6 },
    { 0, "药物使用途径代码1",              DOL_cn,   2 },
    { 0, "药物名称2",                      DOL_ans, 50 },
    { 0, "药物剂型代码2",                  DOL_cn,   1 },
    { 0, "用药天数2",                      DOL_cn,   3 },
    { 0, "药物使用频率2",                  DOL_ans, 20 },
    { 0, "药物使用剂量单位2",              DOL_ans,  6 },
    { 0, "药物使用次剂量2",                DOL_cn,   3 },
    { 0, "药物使用总剂量2",                DOL_cn,   6 },
    { 0, "药物使用途径代码2",              DOL_cn,   2 },
    { 0, "药物名称3",                      DOL_ans, 50 },
    { 0, "药物剂型代码3",                  DOL_cn,   1 },
    { 0, "用药天数3",                      DOL_cn,   3 },
    { 0, "药物使用频率3",                  DOL_ans, 20 },
    { 0, "药物使用剂量单位3",              DOL_ans,  6 },
    { 0, "药物使用次剂量3",                DOL_cn,   3 },
    { 0, "药物使用总剂量3",                DOL_cn,   6 },
    { 0, "药物使用途径代码3",              DOL_cn,   2 },
    { 0, "药物名称4",                      DOL_ans, 50 },
    { 0, "药物剂型代码4",                  DOL_cn,   1 },
    { 0, "用药天数4",                      DOL_cn,   3 },
    { 0, "药物使用频率4",                  DOL_ans, 20 },
    { 0, "药物使用剂量单位4",              DOL_ans,  6 },
    { 0, "药物使用次剂量4",                DOL_cn,   3 },
    { 0, "药物使用总剂量4",                DOL_cn,   6 },
    { 0, "药物使用途径代码4",              DOL_cn,   2 },
    { 0, "药物名称5",                      DOL_ans, 50 },
    { 0, "药物剂型代码5",                  DOL_cn,   1 },
    { 0, "用药天数5",                      DOL_cn,   3 },
    { 0, "药物使用频率5",                  DOL_ans, 20 },
    { 0, "药物使用剂量单位5",              DOL_ans,  6 },
    { 0, "药物使用次剂量5",                DOL_cn,   3 },
    { 0, "药物使用总剂量5",                DOL_cn,   6 },
    { 0, "药物使用途径代码5",              DOL_cn,   2 },
    { 0, "手术/操作名称1",                 DOL_ans, 80 },
    { 0, "手术/操作代码1",                 DOL_ans,  5 },
    { 0, "手术/操作日期1",                 DOL_cn,   4 },
    { 0, "手术/操作名称2",                 DOL_ans, 80 },
    { 0, "手术/操作代码2",                 DOL_ans,  5 },
    { 0, "手术/操作日期2",                 DOL_cn,   4 },
    { 0, "手术/操作名称3",                 DOL_ans, 80 },
    { 0, "手术/操作代码3",                 DOL_ans,  5 },
    { 0, "手术/操作日期3",                 DOL_cn,   4 },
    { 0, "门诊费用分类名称1",              DOL_ans, 20 },
    { 0, "门诊费用分类代码1",              DOL_cn,   1 },
    { 0, "门诊费用金额1",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称2",              DOL_ans, 20 },
    { 0, "门诊费用分类代码2",              DOL_cn,   1 },
    { 0, "门诊费用金额2",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称3",              DOL_ans, 20 },
    { 0, "门诊费用分类代码3",              DOL_cn,   1 },
    { 0, "门诊费用金额3",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称4",              DOL_ans, 20 },
    { 0, "门诊费用分类代码4",              DOL_cn,   1 },
    { 0, "门诊费用金额4",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称5",              DOL_ans, 20 },
    { 0, "门诊费用分类代码5",              DOL_cn,   1 },
    { 0, "门诊费用金额5",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称6",              DOL_ans, 20 },
    { 0, "门诊费用分类代码6",              DOL_cn,   1 },
    { 0, "门诊费用金额6",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称7",              DOL_ans, 20 },
    { 0, "门诊费用分类代码7",              DOL_cn,   1 },
    { 0, "门诊费用金额7",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称8",              DOL_ans, 20 },
    { 0, "门诊费用分类代码8",              DOL_cn,   1 },
    { 0, "门诊费用金额8",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称9",              DOL_ans, 20 },
    { 0, "门诊费用分类代码9",              DOL_cn,   1 },
    { 0, "门诊费用金额9",                  DOL_cn,   4 },
    { 0, "门诊费用分类名称10",             DOL_ans, 20 },
    { 0, "门诊费用分类代码10",             DOL_cn,   1 },
    { 0, "门诊费用金额10",                 DOL_cn,   4 },
    { 0, "交易信息签名",                   DOL_b,   64 },
    { 0, "SAM卡证书",                      DOL_b,  190 }
};
//@}
//--------------------------------------------------------- 
/// 用户卡数据表 
FID_MSG FID_INFO[24] = {
    { "EF05",      FID_Recode, 1, FID_DATA_TABLE(EF05),      FID_DATA_TABLE_LEN(EF05) },
    { "EF06",      FID_Recode, 1, FID_DATA_TABLE(EF06),      FID_DATA_TABLE_LEN(EF06) },
    { "EF07",      FID_Bianry, 1, FID_DATA_TABLE(EF07),      FID_DATA_TABLE_LEN(EF07) },
    { "EF08",      FID_Recode, 1, FID_DATA_TABLE(EF08),      FID_DATA_TABLE_LEN(EF08) },
    { "DF01|EF05", FID_Recode, 1, FID_DATA_TABLE(DF01_EF05), FID_DATA_TABLE_LEN(DF01_EF05) },
    { "DF01|EF06", FID_Recode, 1, FID_DATA_TABLE(DF01_EF06), FID_DATA_TABLE_LEN(DF01_EF06) },
    { "DF01|EF07", FID_Recode, 1, FID_DATA_TABLE(DF01_EF07), FID_DATA_TABLE_LEN(DF01_EF07) },
    { "DF01|EF08", FID_Recode, 1, FID_DATA_TABLE(DF01_EF08), FID_DATA_TABLE_LEN(DF01_EF08) },
    { "DF02|EF05", FID_Recode, 1, FID_DATA_TABLE(DF02_EF05), FID_DATA_TABLE_LEN(DF02_EF05) },
    { "DF02|EF06", FID_Recode, 1, FID_DATA_TABLE(DF02_EF06), FID_DATA_TABLE_LEN(DF02_EF06) },
    { "DF02|EF07", FID_Circle, 3, FID_DATA_TABLE(DF02_EF07), FID_DATA_TABLE_LEN(DF02_EF07) },
    { "DF02|EF08", FID_Circle, 3, FID_DATA_TABLE(DF02_EF08), FID_DATA_TABLE_LEN(DF02_EF08) },
    { "DF03|EF05", FID_Circle, 3, FID_DATA_TABLE(DF03_EF05), FID_DATA_TABLE_LEN(DF03_EF05) },
    { "DF03|EF06", FID_Circle, 5, FID_DATA_TABLE(DF03_EF06), FID_DATA_TABLE_LEN(DF03_EF06) },
    { "DF03|EE01", FID_Bianry, 1, FID_DATA_TABLE(DF03_EE00), FID_DATA_TABLE_LEN(DF03_EE00) },
    { "DF03|EE02", FID_Bianry, 1, FID_DATA_TABLE(DF03_EE00), FID_DATA_TABLE_LEN(DF03_EE00) },
    { "DF03|EE03", FID_Bianry, 1, FID_DATA_TABLE(DF03_EE00), FID_DATA_TABLE_LEN(DF03_EE00) },
    { "DF03|ED01", FID_Bianry, 1, FID_DATA_TABLE(DF03_ED00), FID_DATA_TABLE_LEN(DF03_ED00) },
    { "DF03|ED02", FID_Bianry, 1, FID_DATA_TABLE(DF03_ED00), FID_DATA_TABLE_LEN(DF03_ED00) },
    { "DF03|ED03", FID_Bianry, 1, FID_DATA_TABLE(DF03_ED00), FID_DATA_TABLE_LEN(DF03_ED00) },
    { "", 0, 0, NULL, 0 }
};
//---------------------------------------------------------  
/**
 * @brief 用户卡密钥对照表
 *
 * 密钥为:""表示不受限制,"NULL"表示不支持
 */
FID_KEY_MSG FID_KEY_MAP[] = {
    // FID         读密钥      写密钥      擦除密钥 
    { "EF05",      "",         "NULL",     "NULL" },
    { "EF06",      "RK1_DDF1", "NULL",     "NULL" },
    { "EF07",      "RK1_DDF1", "UK1_DDF1", "NULL" },
    { "EF08",      "RK1_DDF1", "UK1_DDF1", "NULL" },
    { "DF01|EF05", "RK1_DF01", "UK1_DF01", "NULL" },
    { "DF01|EF06", "RK1_DF01", "UK1_DF01", "NULL" },
    { "DF01|EF07", "RK1_DF01", "UK1_DF01", "NULL" },
    { "DF01|EF08", "RK1_DF01", "UK1_DF01", "NULL" },
    { "DF02|EF05", "RK1_DF02", "UK1_DF02", "NULL" },
    { "DF02|EF06", "RK1_DF02", "UK2_DF02", "NULL" },
    { "DF02|EF07", "RK1_DF02", "UK3_DF02", "NULL" },
    { "DF02|EF08", "RK1_DF02", "UK3_DF02", "NULL" },
    { "DF03|EF05", "RK1_DF03", "UK1_DF03", "UK2_DF03" },
    { "DF03|EF06", "RK1_DF03", "UK1_DF03", "UK2_DF03" },
    { "DF03|EE01", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|EE02", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|EE03", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|ED01", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|ED02", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|ED03", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|ED04", "RK1_DF03", "UK1_DF03", "NULL" },
    { "DF03|ED05", "RK1_DF03", "UK1_DF03", "NULL" }
};
/// 用户卡密钥对照表长度 
size_t FID_KEY_MAP_LENGTH = SizeOfArray(FID_KEY_MAP);
/// SAM卡密钥表 
KEY_MSG SAM_KEY_MAP[] = {
    { "IRK_DDF1", 0x04, 0x47, 0x01 },
    { "STK_DDF1", 0x02, 0x28, 0x02 },
    { "STK_DF01", 0x82, 0x28, 0x03 },
    { "STK_DF02", 0x82, 0x48, 0x04 },
    { "STK_DF03", 0x82, 0x48, 0x05 },
    { "LK_DF01",  0x83, 0x27, 0x07 },
    { "LK_DF02",  0x83, 0x27, 0x08 },
    { "LK_DF03",  0x83, 0x27, 0x09 },
    { "UK1_DDF1", 0x05, 0x27, 0x0A },
    { "UK1_DF01", 0x85, 0x27, 0x0B },
    { "UK1_DF02", 0x85, 0x47, 0x0C },
    { "UK2_DF02", 0x86, 0x47, 0x0D },
    { "UK3_DF02", 0x87, 0x47, 0x0E },
    { "UK1_DF03", 0x85, 0x47, 0x0F },
    { "UK2_DF03", 0x86, 0x27, 0x10 },
    { "RK1_DDF1", 0x04, 0x47, 0x11 },
    { "RK1_DF01", 0x84, 0x47, 0x12 },
    { "RK1_DF02", 0x84, 0x47, 0x13 },
    { "RK1_DF03", 0x84, 0x47, 0x14 },
    { "BK_MF",    0x03, 0x27, 0x15 },
    { "STM_MF",   0x02, 0x28, 0x16 }
};
/// SAM卡密钥表长度 
size_t SAM_KEY_MAP_LENGTH = SizeOfArray(SAM_KEY_MAP);
//--------------------------------------------------------- 
} // namespace health_card
} // namespace iccard
} // namespace device
} // namespace zhou_yb
//========================================================= 
