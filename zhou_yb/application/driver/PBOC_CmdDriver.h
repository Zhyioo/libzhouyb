//========================================================= 
/**@file PBOC_CmdDriver.h
 * @brief PBOC应用命令
 * 
 * @date 2016-04-26   17:18:40
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_CMDDRIVER_H_
#define _LIBZHOUYB_PBOC_CMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"

#include "../pboc/pboc_app.h"
using namespace zhou_yb::application::pboc;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// PBOC IC卡命令驱动
class PBOC_CmdDriver : 
    public DevAdapterBehavior<IICCardDevice>, 
    public CommandCollection,
    public RefObject
{
protected:
    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;
    ByteBuilder _appData;
    PBOC_v2_0_AppAdapter _icAdapter;
public:
    //----------------------------------------------------- 
    PBOC_CmdDriver()
    {
        InformationTABLE = PBOC_TransTable::INFORMATION;
        AmountTABLE = PBOC_TransTable::AMOUNT;
        DetailTABLE = PBOC_TransTable::DETAIL;
        TlvConvert = NULL;
        _logInvoker.select(_icAdapter);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _objErr.Invoke(_lasterr, _errinfo);
        _lastErr.Select(_icAdapter, "V2.0");
        _lastErr.Select(_objErr);

        _Registe("GetCardNumber", (*this), &PBOC_CmdDriver::GetCardNumber);
        _Registe("EnumAid", (*this), &PBOC_CmdDriver::EnumAid);
        _Registe("GetBalance", (*this), &PBOC_CmdDriver::GetBalance);
        _Registe("GetInformation", (*this), &PBOC_CmdDriver::GetInformation);
        _Registe("GenARQC", (*this), &PBOC_CmdDriver::GenARQC);
        _Registe("RunARPC", (*this), &PBOC_CmdDriver::RunARPC);
        _Registe("GetDetail", (*this), &PBOC_CmdDriver::GetDetail);
        _Registe("GetAmtDetail", (*this), &PBOC_CmdDriver::GetAmtDetail);
        _Registe("ReadTLV", (*this), &PBOC_CmdDriver::ReadTLV);
        _Registe("GetTLV", (*this), &PBOC_CmdDriver::GetTLV);
    }
    /* 相关属性 */
    /// 读取信息的对照表
    const ushort* InformationTABLE;
    /// 交易信息的对照表
    const ushort* AmountTABLE;
    /// 交易明细的对照表
    const ushort* DetailTABLE;
    /// TLV格式转换器
    PbocTlvConverter::fpTlvAnsConvert TlvConvert;
    //----------------------------------------------------- 
    LC_CMD_ADAPTER(IICCardDevice, _icAdapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    /**
     * @brief 设置终端数据
     * @date 2016-06-21 19:32
     * 
     * @param [in] TLV : hex 需要设置的终端数据
     */
    LC_CMD_METHOD(SetTerminalTLV)
    {
        return true;
    }
    /**
     * @brief 
     * @date 2016-06-11 22:01
     * 
     * @param [in] AID : hex 需要获取的AID
     * 
     * @retval INFO : string 获取到的卡号  
     */
    LC_CMD_METHOD(GetCardNumber)
    {
        string sAid = arg["AID"].To<string>();

        ByteBuilder aid(8);
        DevCommand::FromAscii(sAid.c_str(), aid);

        ByteBuilder cardNumber(8);
        // 如果AID为空则先尝试从A0000333获取
        if(aid.IsEmpty())
        {
            DevCommand::FromAscii(SYS_PBOC_V2_0_BASE_AID, aid);
            if(!_icAdapter.GetCardNumber(cardNumber, true, aid))
            {
                aid.Clear();
                // 枚举应用列表
                list<PBOC_Library::AID> aidlist;
                if(!_icAdapter.EnumAid("", aidlist))
                    return false;

                list<PBOC_Library::AID>::iterator itr;
                for(itr = aidlist.begin();itr != aidlist.end(); ++itr)
                {
                    if(_icAdapter.GetCardNumber(cardNumber, true, itr->aid))
                        break;
                }
                if(itr == aidlist.end())
                    return false;
            }
        }
        else
        {
            if(!_icAdapter.GetCardNumber(cardNumber, true, aid))
                return false;
        }

        rlt.PushValue("INFO", cardNumber.GetString());
        return true;
    }
    /**
     * @brief 获取卡片余额
     * @date 2016-06-21 18:00
     * 
     * @param [in] AID : hex 需要获取余额的AID
     * 
     * @retval Balance : uint 余额(以分为单位)
     */
    LC_CMD_METHOD(GetBalance)
    {
        string sAid = arg["Aid"].To<string>();
        uint balance = 0;
        ByteBuilder aid(8);
        DevCommand::FromAscii(sAid.c_str(), aid);
        if(!_icAdapter.GetBalance(balance, aid))
            return false;

        rlt.PushValue("Balance", ArgConvert::ToString<uint>(balance));
        return true;
    }
    /**
     * @brief 枚举卡片应用列表
     * @date 2016-06-14 21:51
     * 
     * @param [in] Aid : hex 枚举的子应用列表
     * 
     * @retval Aid : hex 枚举到的AID
     * @retval Name : string 枚举到的AID名称
     * @retval Priority : uint 优先级
     */
    LC_CMD_METHOD(EnumAid)
    {
        string sAid = arg["Aid"].To<string>();
        list<PBOC_Library::AID> aidlist;
        if(!_icAdapter.EnumAid(DevCommand::FromAscii(sAid.c_str()), aidlist))
            return false;

        list<PBOC_Library::AID>::iterator itr;
        ByteBuilder tmp(16);
        for(itr = aidlist.begin();itr != aidlist.end(); ++itr)
        {
            tmp.Clear();
            ByteConvert::ToAscii(itr->aid, tmp);
            rlt.PushValue("Aid", tmp.GetString());

            tmp.Clear();
            if(TlvConvert == NULL)
            {
                rlt.PushValue("Name", itr->name.GetString());
            }
            else
            {
                TlvConvert(itr->name.GetString(), itr->name.GetLength(), tmp);
                rlt.PushValue("Name", tmp.GetString());
            }

            rlt.PushValue("Priority", ArgConvert::ToString<uint>(static_cast<uint>(itr->priority)));
        }
        return true;
    }
    /**
     * @brief 获取IC卡数据
     * @date 2016-05-06 18:17
     * 
     * @param [in] AID : hex 需要获取的AID
     * @param [in] FLAG : string 需要获取的IC卡数据(ABCDE等)
     * 
     * @retval INFO : string 获取到卡片数据
     */
    LC_CMD_METHOD(GetInformation)
    {
        string sAid = arg["AID"].To<string>();
        string sFlag = arg["FLAG"].To<string>();

        ByteBuilder tag(8);
        PBOC_AppHelper::getTagHeader(sFlag.c_str(), InformationTABLE, tag);

        if(!_icAdapter.GetInformation(DevCommand::FromAscii(sAid.c_str()), tag, "", _appData))
            return false;

        // 组数据
        ByteBuilder info(64);
        PbocTlvConverter tlvConverter(TlvConvert);
        PBOC_AppHelper::transFromTLV(_appData, info, 3, sFlag.c_str(), InformationTABLE, tlvConverter, true);

        rlt.PushValue("INFO", info.GetString());
        return true;
    }
    /**
     * @brief 获取55域数据
     * @date 2016-06-21 17:51
     * 
     * @param [in] Aid : hex 需要获取ARQC的AID
     * @param [in] TransINFO : string 交易数据(PQRST格式)
     * @param [in] TAG : string 生成55域的标签数据
     * 
     * @retval ARQC : hex 获取到的55域数据  
     */
    LC_CMD_METHOD(GenARQC)
    {
        return true;
    }
    /**
     * @brief 执行写卡脚本
     * @date 2016-06-21 17:59
     * 
     * @param [in] ARPC : hex 后台返回的写卡脚本
     * @param [in] GacMode : string GAC模式
     * - 参数:
     *  - None 不处理GAC
     *  - First GAC于脚本优先
     *  - Normal GAC于脚本之后
     * .
     * 
     * @retval DF31 : hex 写卡脚本通知
     * @retval TC : hex 写卡成功后返回的TC数据
     */
    LC_CMD_METHOD(RunARPC)
    {
        return true;
    }
    /**
     * @brief 获取卡片脱机交易明细
     * @date 2016-06-21 18:02
     * 
     * @param [in] Aid : hex 需要获取明细的AID
     * @param [in] Index : uint 需要获取的明细索引号
     * @warning 为0时表示获取所有交易明细
     * 
     * @retval INFO : string 获取到的交易明细(PQRST)
     */
    LC_CMD_METHOD(GetDetail)
    {
        string sAid = arg["Aid"].To<string>();
        uint index = arg["Index"].To<uint>(0);

        ByteBuilder detailFormat(16);
        list<ByteBuilder> detailList;

        ByteBuilder aid(8);
        DevCommand::FromAscii(sAid.c_str(), aid);
        if(!_icAdapter.GetDealDetail(detailFormat, detailList, aid, index))
            return false;

        ByteBuilder detailData(256);
        ByteBuilder detailBuff(256);
        PbocTlvConverter tlvConverter(TlvConvert);
        list<ByteBuilder>::iterator itr;
        for(itr = detailList.begin();itr != detailList.end();++itr)
        {
            detailData.Clear();
            PBOC_AppHelper::packFormatData(detailFormat, *itr, detailBuff);
            PBOC_AppHelper::transFromTLV(detailBuff, detailData, 3, "", DetailTABLE, tlvConverter, true);

            rlt.PushValue("INFO", detailData.GetString());
        }
        return true;
    }
    LC_CMD_METHOD(GetAmtDetail)
    {
        return true;
    }
    /**
     * @brief 读取IC卡标签
     * @date 2016-05-06 18:06
     * 
     * @param [in] TAG : hex 需要读取的IC卡标签
     * 
     * @retval TLV : hex 获取到的标签数据
     */
    LC_CMD_METHOD(ReadTLV)
    {
        return true;
    }
    /**
     * @brief 获取IC卡标签(支持获取55域数据)
     * @date 2016-05-06 18:15
     * 
     * @param [in] TAG : hex 需要获取的IC卡标签
     * 
     * @retval TLV : hex 获取到的标签数据
     */
    LC_CMD_METHOD(GetTLV)
    {
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_CMDDRIVER_H_
//========================================================= 