//========================================================= 
/**@file IDCardCmdDriver.h
 * @brief 二代证阅读器命令驱动
 * 
 * @date 2016-05-04   21:02:01
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IDCARDCMDDRIVER_H_
#define _LIBZHOUYB_IDCARDCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"
#include "CommonCmdDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 二代证命令驱动
class IDCardCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>, 
    public InterruptBehavior,
    public CommandCollection, 
    public RefObject
{
protected:
    InterruptInvoker _interrupt;
    SdtApiDevAdapter _sdtapiAdapter;
    IDCardParser _idParser;
public:
    static void ToArgParser(IDCardInformation& idInfo, ICommandHandler::CmdArgParser& rlt)
    {
        rlt.PushValue("Name", idInfo.Name);
        rlt.PushValue("ID", idInfo.ID);
        rlt.PushValue("Gender", idInfo.Gender);
        rlt.PushValue("Nation", idInfo.Nation);
        rlt.PushValue("Birthday", idInfo.Birthday);
        rlt.PushValue("Address", idInfo.Address);
        rlt.PushValue("Department", idInfo.Department);
        rlt.PushValue("StartDate", idInfo.StartDate);
        rlt.PushValue("EndDate", idInfo.EndDate);
        rlt.PushValue("GenderCode", ArgConvert::ToString<uint>(idInfo.GenderCode));
        rlt.PushValue("NationCode", ArgConvert::ToString<uint>(idInfo.NationCode));
    }

    IDCardCmdDriver()
    {
        _idParser.SelectDevice(_sdtapiAdapter);

        select_helper<InterruptInvoker::SelecterType>::select(_interrupt),
            _idParser;

        RegisteInterrupter(IDCardCmdDriver);
    }
    LC_CMD_INTERRUPT(_interrupt);
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IDCARDCMDDRIVER_H_
//========================================================= 