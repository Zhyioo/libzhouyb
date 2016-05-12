//========================================================= 
/**@file SmartCardReader.h
 * @brief ÖÇÄÜ¿¨ÔÄ¶ÁÆ÷
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
/// Í¨ÓÃµÄ¶Á¿¨Æ÷
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
    /// µ±Ç°ÒÑ¼¤»îµÄ¿¨²ÛºÅ
    byte ActiveID;
};
/// ´®¿ÚIC¿¨¶Á¿¨Æ÷

//--------------------------------------------------------- 
/// ÖÇÄÜ¿¨ÔÄ¶ÁÆ÷
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