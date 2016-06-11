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
    LOGGER(LoggerInvoker _logInvoker);
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
        LOGGER(_logInvoker.select(_icAdapter));

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _objErr.Invoke(_lasterr, _errinfo);
        _lastErr.Select(_icAdapter, "V2.0");
        _lastErr.Select(_objErr);

        _Registe("GetInformation", (*this), &PBOC_CmdDriver::GetInformation);
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
     * @brief 
     * @date 2016-06-11 22:01
     * 
     * @param [in] AID : string 需要获取的AID
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
     * @brief 获取IC卡数据
     * @date 2016-05-06 18:17
     * 
     * @param [in] AID : string 需要获取的AID
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
    LC_CMD_METHOD(GenARQC)
    {
        return true;
    }
    LC_CMD_METHOD(RunARPC)
    {
        return true;
    }
    LC_CMD_METHOD(GetDetail)
    {
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
     * @param [in] TAG : string 需要读取的IC卡标签
     * 
     * @retval TLV : string 获取到的标签数据
     */
    LC_CMD_METHOD(ReadTLV)
    {
        return true;
    }
    /**
     * @brief 获取IC卡标签(支持获取55域数据)
     * @date 2016-05-06 18:15
     * 
     * @param [in] TAG : string 需要获取的IC卡标签
     * 
     * @retval TLV : string 获取到的标签数据
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