//========================================================= 
/**@file ComICCardCmdDriver.h
 * @brief 三合一读卡器驱动逻辑
 * 
 * @date 2016-05-04   21:09:37
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BP8903CMDDRIVER_H_
#define _LIBZHOUYB_BP8903CMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"

#include "ICCardCmdDriver.h"
#include "PBOC_CmdDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 三合一命令驱动
class ComICCardCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>, 
    public CommandCollection,
    public RefObject
{
protected:
    LastErrInvoker _objErr;
    LastErrExtractor _icErr;
    LastErrExtractor _appErr;
    LastErrExtractor _lastErr;
    LoggerInvoker _logInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapter;
    ComICCardCmdAdapter _cmdAdapter;
    ComContactICCardDevAdapter _contactIC;
    ComContactlessICCardDevAdapter _contactlessIC;
    ComPsamICCardDevAdapter _psam1;
    ComPsamICCardDevAdapter _psam2;

    ICCardCmdDriver _icDriver;
    PBOC_CmdDriver _pbocDriver;

    /// 获取指定名称的读卡器
    Ref<IICCardDevice> _GetSLOT(const ByteArray& slotArray)
    {
        Ref<IICCardDevice> ic;
        if(StringConvert::Compare(slotArray, "Contact", true))
        {
            ic = _contactIC;
        }
        else if(StringConvert::Compare(slotArray, "Contactless", true))
        {
            ic = _contactlessIC;
        }
        else if(StringConvert::Compare(slotArray, "PSAM1", true))
        {
            ic = _psam1;
        }
        else if(StringConvert::Compare(slotArray, "PSAM2", true))
        {
            ic = _psam2;
        }
        return ic;
    }
public:
    ComICCardCmdDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _icErr.IsFormatMSG = false;
        _icErr.IsLayerMSG = false;
        _icErr.Select(_contactIC);
        _icErr.Select(_contactlessIC);
        _icErr.Select(_psam1);
        _icErr.Select(_psam2);

        _appErr.IsFormatMSG = false;
        _appErr.IsLayerMSG = false;
        _appErr.Select(_icDriver);
        _appErr.Select(_pbocDriver);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_cmdAdapter);
        _lastErr.Select(_icErr);
        _lastErr.Select(_appErr);
        _lastErr.Select(_objErr);

        _cmdAdapter.SelectDevice(_pDev);
        _contactIC.SelectDevice(_cmdAdapter);
        _contactlessIC.SelectDevice(_cmdAdapter);
        _psam1.SelectDevice(_cmdAdapter);
        _psam2.SelectDevice(_cmdAdapter);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _cmdAdapter, _contactIC, _contactlessIC, _psam1, _psam2, _icDriver, _pbocDriver;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType>::select(_adapter),
            _cmdAdapter;

        _Registe("PowerOn", (*this), &ComICCardCmdDriver::PowerOn);
        _Registe("IsCardPresent", (*this), &ComICCardCmdDriver::IsCardPresent);
        _Registe("SelectSLOT", (*this), &ComICCardCmdDriver::SelectSLOT);

        Registe(_icDriver.GetCommand(""));
        Registe(_pbocDriver.GetCommand(""));
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    /// 上电
    LC_CMD_METHOD(PowerOn)
    {
        if(!SelectSLOT(arg, rlt))
            return false;

        if(_icDriver.ActiveDevice() == _psam2)
            arg["Arg"].Value = "1";
        return true;
    }
    /**
     * @brief 判断读卡器上是否有卡
     * @date 2016-05-04 21:18
     * 
     * @param [in] arglist 
     * - 参数
     *  - SLOT 需要判断的卡片类型
     * .
     * 
     * @return True|False 有无卡
     */
    LC_CMD_METHOD(IsCardPresent)
    {

        return true;
    }
    /**
     * @brief 选择卡槽号
     * @date 2016-05-04 21:21
     * 
     * @param [in] arglist
     * - 参数
     *  - Contact 接触式
     *  - Contactless 非接
     *  - PSAM1 PSAM卡1
     *  - PSAM2 PSAM卡2
     * .
     */
    LC_CMD_METHOD(SelectSLOT)
    {
        string slot = arg["SLOT"].To<string>();
        Ref<IICCardDevice> ic = _GetSLOT(slot.c_str());
        if(ic.IsNull())
            return false;

        _icDriver.SelectDevice(ic);
        _pbocDriver.SelectDevice(ic);
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BP8903CMDDRIVER_H_
//========================================================= 