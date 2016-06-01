//========================================================= 
/**@file SmartCardReader.h
 * @brief 智能卡阅读器
 * 
 * @date 2016-04-25   21:57:37
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_SMARTCARDREADER_H_
#define _LIBZHOUYB_SMARTCARDREADER_H_
//--------------------------------------------------------- 
#include "../../include/BaseDevice.h"
#include "../../include/Device.h"
#include "../../include/Extension.h"

#include "../../application/tools/ParamHelper.h"
using zhou_yb::application::tools::ParamHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace iccard {
//--------------------------------------------------------- 
/* [Arg:<NULL>][Slot:<FF>] */
/// 通用的读卡器
class CommonSmartCardReader :
    public DevAdapterBehavior<IInteractiveTrans>,
    public IICCardDevice,
    public RefObject
{
protected:
    LastErrExtractor _icLastErr;
    LastErrExtractor _lastErr;
    ComICCardCmdAdapter _cmdAdapter;
    ComContactICCardBaseDevAdapter _contactIC;
    ComContactlessICCardDevAdapter _contactlessIC;
public:
    CommonSmartCardReader()
    {
        _icLastErr.IsFormatMSG = false;
        _icLastErr.IsLayerMSG = false;
        _icLastErr.Select(_contactIC);
        _icLastErr.Select(_contactlessIC);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_cmdAdapter);
        _lastErr.Select(_icLastErr);
    }
    virtual bool PowerOn(const char*readerName, ByteBuilder*pAtr = NULL)
    {
        return true;
    }
    virtual bool HasPowerOn() const
    {
        return true;
    }
    virtual bool PowerOff()
    {
        return true;
    }
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        return true;
    }
    virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv)
    {
    }
    /// 当前已激活的卡槽号
    byte ActiveID;
};
/// 串口IC卡读卡器

//--------------------------------------------------------- 
/// 智能卡阅读器
class SmartCardReader : public IICCardDevice, public RefObject
{
protected:
    ComDevice _comdev;
    HidCmdAdapter<HidDevice, 0, 1> _hiddev;
    
public:

};
//--------------------------------------------------------- 
} // namespace iccard
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_SMARTCARDREADER_H_
//========================================================= 