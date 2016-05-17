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
    //----------------------------------------------------- 
    LC_CMD_ADAPTER(IICCardDevice, _icAdapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    /**
     * @brief 获取IC卡数据
     * @date 2016-05-06 18:17
     * 
     * @param [in] arglist
     * - 参数
     *  - AID 需要获取的AID
     *  - FLAG 需要获取的IC卡数据(ABCDE等)
     * .
     * @retval INFO 获取到卡片数据
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
        PbocTlvConverter tlvConverter;
        PBOC_AppHelper::transFromTLV(_appData, info, 3, InformationTABLE, tlvConverter, true);

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
     * @param [in] arglist
     * - 参数
     *  - TAG 需要读取的IC卡标签
     * .
     * 
     * @retval TLV 获取到的标签数据
     */
    LC_CMD_METHOD(ReadTLV)
    {
        return true;
    }
    /**
     * @brief 获取IC卡标签(支持获取55域数据)
     * @date 2016-05-06 18:15
     * 
     * @param [in] arglist
     * - 参数
     *  - TAG 需要获取的IC卡标签
     * .
     * @retval TLV 获取到的标签数据
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