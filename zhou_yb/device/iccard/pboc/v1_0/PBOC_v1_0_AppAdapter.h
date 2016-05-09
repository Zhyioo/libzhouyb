//========================================================= 
/**@file PBOC_v1_0_AppAdapter.h 
 * @brief PBOC1.0应用流程适配器 
 * 
 * @date 2014-08-15   21:40:18 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_V1_0_APPADAPTER_H_
#define _LIBZHOUYB_PBOC_V1_0_APPADAPTER_H_
//--------------------------------------------------------- 
#include "../../base/ICCardAppAdapter.h"

#include "PBOC_v1_0_CMD.h"

#include "PBOC_v1_0_Library.h"
using namespace zhou_yb::device::iccard::pboc::PBOC_Library;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace v1_0 {
//--------------------------------------------------------- 
/**
 * @brief PBOC1.0应用基本流程  
 */ 
class PBOC_v1_0_AppAdapter : public ICCardAppAdapter
{
protected:
    //----------------------------------------------------- 
    /// 初始化 
    inline void _init()
    {
        KeyIndex = 0;
        AmountCash = 0;
        TerminalID.Append(static_cast<byte>(0x00), 6);
        AmountDate.Append(static_cast<byte>(0x00), 4);
        AmountTime.Append(static_cast<byte>(0x00), 3);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    PBOC_v1_0_AppAdapter() : ICCardAppAdapter() { _init(); }
    //----------------------------------------------------- 
    /* 相关的交易属性 */
    /// 交易初始化时的密钥索引 
    byte KeyIndex;
    /// 交易金额(以分为单位) 
    uint AmountCash;
    /// 终端序号(6字节) 
    ByteBuilder TerminalID;
    /// 交易日期(4字节)
    ByteBuilder AmountDate;
    /// 交易时间(3字节) 
    ByteBuilder AmountTime;
    //----------------------------------------------------- 
    /**
     * @brief 选择应用
     * @param [in] sAid 输入的AID 
     * @param [in] pAidData 选文件后获取到的AID数据 
     * @retval bool
     * @return 
     */
    bool SelectAid(const char* sAid, ByteBuilder* pAidData = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "应用目录:<" << _strput(sAid) << ">\n");
        ByteBuilder selectCmd(8);
        ByteBuilder recv(64);

        selectCmd.Clear();
        SelectCmd::Make(selectCmd, DevCommand::FromAscii(sAid), SelectCmd::First, SelectCmd::ByAid);
        recv.Clear();
        ASSERT_FuncInfoRet(_apdu(selectCmd, recv), "选文件失败");

        if(pAidData != NULL)
            (*pAidData) += recv;

        return _logRetValue(true);
    }
    /**
     * @brief 获取卡片余额 
     * @param [out] balance 获取到的余额(以分为单位) 
     * @param [in] purseType [default:EP电子钱包] 要获取的钱包类型(EP,ED) 
     */ 
    bool GetBalance(uint& balance, GetBalanceCmd::PurseType purseType = GetBalanceCmd::EP)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"purseType:";
        switch(purseType)
        {
        case GetBalanceCmd::ED:
            _log.WriteLine("ED");
            break;
        case GetBalanceCmd::EP:
            _log.WriteLine("EP");
            break;
        default:
            _log.WriteLine(_hex(static_cast<byte>(purseType)));
            break;
        });

        ByteBuilder cmd(8);
        GetBalanceCmd::Make(cmd, purseType);

        ByteBuilder recv(8);
        ASSERT_FuncInfoRet(_apdu(cmd, recv), "取余额失败");
        // 余额大小为4字节 
        ByteConvert::ToObject(recv, balance);
        LOGGER(_log<<"余额:<"<<balance<<">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 圈存初始化 
     * @param [in] pinAscii 用户PIN码 
     * @param [out] data 圈存初始化的返回数据 
     * @param [in] transType [default:EP_Load电子钱包圈存] 圈存类型 
     */ 
    bool LoadInit(const char* pinAscii, ByteBuilder& data, PBOC_v1_0_TransType::Type transType = PBOC_v1_0_TransType::EP_Load)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "PIN码:<" << _strput(pinAscii) << ">\n");
        
        ByteBuilder pin(8);
        PinFormater::FromAscii(pinAscii, pin);

        ByteBuilder cmd(8);
        ByteBuilder recv(8);
        VerifyCmd::Make(cmd, pin);

        // PIN码验证错误,剩余的尝试次数可以通过GetSW来判断出来 
        ASSERT_FuncErrInfoRet(_apdu(cmd, recv), DeviceError::DevVerifyErr, "PIN码错误");
        /* 圈存初始化 */
        recv.Clear();
        ByteConvert::FromObject(AmountCash, recv);

        LOGGER(
        _log<<"交易金额:<"<<AmountCash<<">,HEX:<";
        _log.WriteStream(recv)<<">\n";
        _log<<"交易类型:<"<<_hex(_itobyte(transType))<<">,密钥索引:<"<<_hex(KeyIndex)<<">,终端序号:<";
        _log.WriteStream(TerminalID)<<">\n");

        cmd.Clear();
        InitializeForLoadCmd::Make(cmd, transType, KeyIndex, recv, TerminalID);
        recv.Clear();

        ASSERT_FuncInfoRet(_apdu(cmd, recv), "圈存初始化失败");

        LOGGER(_log.WriteLine("圈存初始化数据:").WriteStream(recv)<<endl);
        data += recv;

        return _logRetValue(true);
    }
    /**
     * @brief 圈存
     * @param [in] MAC2_4 圈存时4字节MAC2数据 
     * @param [out] pTAC_4 [default:NULL] 写卡后返回的4字节TAC数据
     * @retval bool
     * @return 
     */
    bool Load(const ByteArray& MAC2_4, ByteBuilder* pTAC_4 = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"交易日期:<";
        _log.WriteStream(AmountDate)<<">,交易时间:<";
        _log.WriteStream(AmountTime)<<">,MAC2:<";
        _log.WriteStream(MAC2_4)<<">\n");

        ByteBuilder cmd(16);
        ByteBuilder recv(8);
        CreditForLoadCmd::Make(cmd, AmountDate, AmountTime, MAC2_4);

        ASSERT_FuncInfoRet(_apdu(cmd, recv), "圈存失败");

        if(pTAC_4 != NULL)
            pTAC_4->Append(recv);

        return _logRetValue(true);
    }
    /**
     * @brief 消费初始化 
     * @param [out] data 消费初始化后返回的数据 
     * @param [in] transType [default:EP电子钱包消费] 消费类型 
     */ 
    bool PurchaseInit(ByteBuilder& data, PBOC_v1_0_TransType::Type transType = PBOC_v1_0_TransType::EP_Purchase)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"消费类型:<"<<_hex(_itobyte(transType))<<">\n");

        ByteBuilder cmd(16);
        ByteBuilder recv(8);
        ByteConvert::FromObject(AmountCash, recv);

        LOGGER(
        _log<<"交易金额:<"<<AmountCash<<">,HEX:<";
        _log.WriteStream(recv)<<">\n";
        _log<<"交易类型:<"<<_hex(_itobyte(transType))<<">,密钥索引:<"<<_hex(KeyIndex)<<">,终端序号:<";
        _log.WriteStream(TerminalID)<<">\n");

        InitializeForPurchaseCmd::Make(cmd, transType, KeyIndex, recv, TerminalID);

        recv.Clear();
        ASSERT_FuncInfoRet(_apdu(cmd, recv), "消费初始化失败");

        LOGGER(_log.WriteLine("消费初始化数据:").WriteStream(recv)<<endl);
        data += recv;

        return _logRetValue(true);
    }
    /**
     * @brief 消费 
     * @param [in] terminalAmountID_4 4字节终端交易序号 
     * @param [in] MAC1_4 4字节MAC1数据 
     * @param [out] pTAC_4 [default:NULL] 消费成功后返回的4字节TAC数据 
     * @param [out] pMAC2_4 [default:NULL] 消费成功后返回的4字节MAC2数据 
     */ 
    bool Purchase(const ByteArray& terminalAmountID_4, const ByteArray& MAC1_4, ByteBuilder* pTAC_4 = NULL, ByteBuilder* pMAC2_4 = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"终端交易序号:<";
        _log.WriteStream(TerminalID)<<"交易日期:<";
        _log.WriteStream(AmountDate)<<">,交易时间:<";
        _log.WriteStream(AmountTime)<<">,MAC1:<";
        _log.WriteStream(MAC1_4)<<">\n");

        ByteBuilder cmd(16);
        ByteBuilder recv(8);
        DebitForPurchaseCmd::Make(cmd, terminalAmountID_4, AmountDate, AmountTime, MAC1_4);

        ASSERT_FuncInfoRet(_apdu(cmd, recv), "消费失败");
        LOGGER(_log.WriteLine("消费返回数据:").WriteStream(recv)<<endl);

        if(pTAC_4 != NULL)
            StringConvert::Left(recv, 4, *pTAC_4);
        if(pMAC2_4 != NULL)
            StringConvert::Right(recv, 4, *pMAC2_4);

        return _logRetValue(true);
    }
    /**
     * @brief 修改初始化
     * @param [out] random 获取到的随机数 
     * @param [in] len [default:4] 需要获取的随机数长度
     */
    bool UpdateInit(ByteBuilder& random, byte len = GetChallengeCmd::len4)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "长度:<" << static_cast<int>(len) << ">\n");
        /* 取随机数 */
        ByteBuilder cmd(8);
        GetChallengeCmd::Make(cmd, len);

        return _logRetValue(_apdu(cmd, random));
    }
    /**
     * @brief 修改 
     * @param [in] 
     */
    bool Update(const ByteArray& mac)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log.WriteLine("MAC数据:");
        _log.WriteStream(mac) << endl);

        ByteBuilder recv;
        return _logRetValue(_apdu(mac, recv));
    }
    /**
     * @brief 读交易明细  
     * @param [in] sfi 交易记录文件的SFI 
     * @param [out] detaillist 获取到的交易记录 
     */ 
    bool GetDetails(byte sfi, list<ByteBuilder>& detaillist)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "SFI:<" << _hex(sfi) << ">\n");

        ByteBuilder cmd(8);
        ReadRecodeCmd::Make(cmd, sfi, 0);

        size_t count = 0;
        for(int i = 0;i < 10; ++i)
        {
            // 记录号 
            cmd[ICCardLibrary::P1_INDEX] = _itobyte(i+1);

            detaillist.push_back();
            if(!_apdu(cmd, detaillist.back()))
            {
                detaillist.pop_back();
                break;
            }
            ++count;
        }
        LOGGER(_log << "获取到明细:<" << count << ">条\n");

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// PBOC1.0 交易辅助器 
class PBOC_v1_0_AppHelper
{
protected:
    PBOC_v1_0_AppHelper() {}
public:
    //----------------------------------------------------- 
    /**
     * @brief 设置终端属性
     * @param [in] adapter 需要设置的适配器
     * @param [in] terminalNo [default:NULL] 终端序号
     * @param [in] amountDate [default:NULL] 交易日期(为NULL表示获取当前系统日期)
     * @param [in] amountTime [default:NULL] 交易时间(为NULL表示获取当前系统时间)
     */
    static bool Fill(PBOC_v1_0_AppAdapter& adapter, const char* terminalNo = NULL, const char* amountDate = NULL, const char* amountTime = NULL)
    {
        adapter.TerminalID.Clear();
        DevCommand::FromAscii(terminalNo, adapter.TerminalID);
        ByteConvert::Fill(adapter.TerminalID, 6);

        ByteBuilder date(4);
        ByteBuilder time(4);
        PBOC_Library::GetLocalTime(&date, &time);

        adapter.AmountDate.Clear();
        if(_is_empty_or_null(amountDate))
        {
            adapter.AmountDate = date;
        }
        else
        {
            DevCommand::FromAscii(amountDate, adapter.AmountDate);
            ByteConvert::Fill(adapter.AmountDate, 4);
        }

        adapter.AmountTime.Clear();
        if(_is_empty_or_null(amountTime))
        {
            adapter.AmountTime = time;
        }
        else
        {
            DevCommand::FromAscii(amountTime, adapter.AmountTime);
            ByteConvert::Fill(adapter.AmountTime, 3);
        }

        return true;
    }
    //----------------------------------------------------- 
    /**
     * @brief 解析交易记录
     * - 记录格式:
     *  - 1-2      联机或脱机交易序号    2
     *  - 3-5      透支限额    3
     *  - 6-9      交易金额    4
     *  - 10       交易类型    1
     *  - 11-16    终端机编号  6
     *  - 17-20    交易日期    4
     *  - 21-23    交易时间    3
     * .
     * @param [in] detail 单独的一条交易记录
     * @param [out] sInfo 以SPLIT_CHAR号分隔每个字段的交易记录数据
     * @param [in] splitFlag [default:SPLIT_STRING] 分隔符 
     */
    static bool UnpackDetail(const ByteArray& detail, ByteBuilder& sInfo, const char* splitFlag = SPLIT_STRING)
    {
        //int LEN_ARR[] = { 2, 3, 4, 1, 6, 4, 3 };
        if(detail.GetLength() < 23)
            return false;

        /* 交易序号 */
        ByteConvert::ToAscii(detail.SubArray(0, 2), sInfo);
        sInfo += _strput(splitFlag);
        /* 透支限额 */
        ByteBuilder tmp = detail.SubArray(2, 3);
        ByteConvert::Fill(tmp, sizeof(uint));
        uint balance = 0;
        ByteConvert::ToObject(tmp, balance);
        sInfo.Format("%d", balance);
        sInfo += _strput(splitFlag);
        /* 交易金额 */
        balance = 0;
        ByteConvert::ToObject(detail.SubArray(5, 4), balance);
        sInfo.Format("%d", balance);
        sInfo += _strput(splitFlag);
        /* 交易类型 */
        ByteConvert::ToAscii(detail.SubArray(9, 1), sInfo);
        sInfo += _strput(splitFlag);
        /* 终端机编号 */
        ByteConvert::ToAscii(detail.SubArray(10, 6), sInfo);
        sInfo += _strput(splitFlag);
        /* 交易日期 */
        ByteConvert::ToAscii(detail.SubArray(16, 4), sInfo);
        sInfo += _strput(splitFlag);
        /* 交易时间 */
        ByteConvert::ToAscii(detail.SubArray(20, 3), sInfo);

        return true;
    }
    /**
     * @brief 解析圈存初始化返回的数据
     * @param [in] adapter 应用适配器
     * @param [in] loadInitBuff 圈存初始化返回的数据源
     * @param [out] sInfo 获取到的数据 "余额(分)|交易序号|密钥版本|算法标识|随机数|MAC1|SESLK|MAC1计算数据|MAC2计算数据"
     * @param [in] splitFlag [default:SPLIT_STRING]分隔符
     * @param [in] transType [default:EP] 交易类型 
     */
    static bool UnpackLoadInit(PBOC_v1_0_AppAdapter& adapter, const ByteArray& loadInitBuff, ByteBuilder& sInfo,
        const char* splitFlag = SPLIT_STRING, PBOC_v1_0_TransType::Type transType = PBOC_v1_0_TransType::EP_Load)
    {
        //int LEN_ARR[] = { 4, 2, 1, 1, 4, 4 };
        if(loadInitBuff.GetLength() < 16)
            return false;

        /* 余额 */
        ushort nBalance = 0;
        ByteConvert::ToObject(loadInitBuff.SubArray(0, 4), nBalance);
        sInfo.Format("%d", nBalance);
        sInfo += _strput(splitFlag);
        /* 交易序号 */
        ByteConvert::ToAscii(loadInitBuff.SubArray(4, 2), sInfo);
        sInfo += _strput(splitFlag);
        /* 密钥版本 */
        ByteConvert::ToAscii(loadInitBuff.SubArray(6, 1), sInfo);
        sInfo += _strput(splitFlag);
        /* 算法标识 */
        ByteConvert::ToAscii(loadInitBuff.SubArray(7, 1), sInfo);
        sInfo += _strput(splitFlag);
        /* 随机数 */
        ByteConvert::ToAscii(loadInitBuff.SubArray(8, 4), sInfo);
        sInfo += _strput(splitFlag);
        /* MAC1 */
        ByteConvert::ToAscii(loadInitBuff.SubArray(12, 4), sInfo);
        sInfo += _strput(splitFlag);
        /* SESLK */
        // 随机数 
        ByteConvert::ToAscii(loadInitBuff.SubArray(8, 4), sInfo);
        // 交易序号 
        ByteConvert::ToAscii(loadInitBuff.SubArray(4, 2), sInfo);
        // 8000
        sInfo += "8000";
        sInfo += _strput(splitFlag);
        /* MAC1计算数据 */
        // 余额 
        ByteConvert::ToAscii(loadInitBuff.SubArray(0, 4), sInfo);
        // 交易金额 
        ByteBuilder tmp(8);
        ByteConvert::FromObject(adapter.AmountCash, tmp);
        ByteConvert::ToAscii(tmp, sInfo);
        // 交易类型 
        tmp.Clear();
        tmp += static_cast<byte>(transType);
        ByteConvert::ToAscii(tmp, sInfo);
        // 终端机编号
        ByteConvert::ToAscii(adapter.TerminalID, sInfo);
        sInfo += _strput(splitFlag);
        /* MAC2计算数据 */
        // 交易金额 
        tmp.Clear();
        ByteConvert::FromObject(adapter.AmountCash, tmp);
        ByteConvert::ToAscii(tmp, sInfo);
        // 交易类型 
        tmp.Clear();
        tmp += static_cast<byte>(transType);
        ByteConvert::ToAscii(tmp, sInfo);
        // 终端机编号
        ByteConvert::ToAscii(adapter.TerminalID, sInfo);
        // 交易日期 
        ByteConvert::ToAscii(adapter.AmountDate, sInfo);
        // 交易时间 
        ByteConvert::ToAscii(adapter.AmountTime, sInfo);

        return true;
    }
    /**
     * @brief 解析消费初始化返回的数据
     * @param [in] adapter 应用适配器
     * @param [in] purchaseInitBuff 消费初始化返回的数据源
     * @param [out] sInfo 获取到的数据 "余额(分)|交易序号|透支限额|密钥版本|算法标识|随机数|SESLK|MAC1计算数据"
     * @param [in] splitFlag [default:SPLIT_STRING]分隔符
     * @param [in] transType [default:EP] 交易类型 
     */
    static bool UpdatePurchaseInit(PBOC_v1_0_AppAdapter& adapter, const ByteArray& purchaseInitBuff, ByteBuilder& sInfo,
        const char* splitFlag = SPLIT_STRING, PBOC_v1_0_TransType::Type transType = PBOC_v1_0_TransType::EP_Load)
    {
        if(purchaseInitBuff.GetLength() < 15)
            return false;

        ByteBuilder tmp(8);
        /* 余额 */
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(0, 4), sInfo);
        sInfo += _strput(splitFlag);
        /* 交易序号 */
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(4, 2), sInfo);
        sInfo += _strput(splitFlag);
        /* 透支限额 */
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(6, 3), sInfo);
        sInfo += _strput(splitFlag);
        /* 密钥版本 */
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(9, 1), sInfo);
        sInfo += _strput(splitFlag);
        /* 算法标识 */
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(10, 1), sInfo);
        sInfo += _strput(splitFlag);
        /* 随机数 */
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(11, 4), sInfo);
        sInfo += _strput(splitFlag);
        /* SESPK */
        // 随机数
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(11, 4), sInfo);
        // 交易序号 
        ByteConvert::ToAscii(purchaseInitBuff.SubArray(4, 2), sInfo);
        sInfo += "0000";
        sInfo += _strput(splitFlag);
        /* MAC1计算数据 */
        // 交易金额
        tmp.Clear();
        ByteConvert::FromObject(adapter.AmountCash, tmp);
        ByteConvert::ToAscii(tmp, sInfo);
        // 交易类型
        tmp.Clear();
        tmp += static_cast<byte>(transType);
        ByteConvert::ToAscii(tmp, sInfo);
        // 终端机编号
        ByteConvert::ToAscii(adapter.TerminalID, sInfo);
        // 交易日期 
        ByteConvert::ToAscii(adapter.AmountDate, sInfo);
        // 交易时间  
        ByteConvert::ToAscii(adapter.AmountTime, sInfo);

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace v1_0 
} // namespace pboc
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_V1_0_APPADAPTER_H_
//========================================================= 