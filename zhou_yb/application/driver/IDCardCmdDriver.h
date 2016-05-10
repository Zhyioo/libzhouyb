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
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 二代证命令驱动
class IDCardCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>, 
    public CommandCollection, 
    public RefObject
{
protected:
    SdtApiDevAdapter _sdtapiAdapter;
    IDCardParser _idParser;
public:
    IDCardCmdDriver()
    {
        _idParser.SelectDevice(_sdtapiAdapter);
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IDCARDCMDDRIVER_H_
//========================================================= 