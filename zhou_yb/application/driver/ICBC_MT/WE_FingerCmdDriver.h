//========================================================= 
/**@file WE_FingerCmdDriver.h
 * @brief 
 * 
 * @date 2016-06-14   20:48:27
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WE_FINGERCMDDRIVER_H_
#define _LIBZHOUYB_WE_FINGERCMDDRIVER_H_
//--------------------------------------------------------- 
#include "../CommonCmdDriver.h"

#include "../../finger/FingerDevAdapter.h"
using zhou_yb::application::finger::WE_FingerDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// ICBC维尔指纹仪命令驱动
class WE_FingerCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>,
    public CommandCollection,
    public RefObject
{
protected:
    CommandFilter _cmdFilter;
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;
    WE_FingerDevAdapter _fingerAdapter;
public:
    WE_FingerCmdDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_fingerAdapter, "WE");
        _lastErr.Select(_objErr);

        _cmdFilter.SetReadTX(0x02, 0x03);
        _fingerAdapter.SelectDevice(_cmdFilter);

        _Registe("GetFeature", (*this), &WE_FingerCmdDriver::GetFeature);
        _Registe("GetTemplate", (*this), &WE_FingerCmdDriver::GetTemplate);
        _Registe("SaveFingerImage", (*this), &WE_FingerCmdDriver::SaveFingerImage);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _cmdFilter);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_LOGGER(_fingerAdapter);
    /**
     * @brief 获取指纹模板
     * @date 2016-06-12 20:01
     * 
     * @param [in] Encode : string 指纹格式
     * - 参数:
     *  - Hex 
     *  - Base64
     * .
     * 
     * @retval Finger : string 指纹模板  
     */
    LC_CMD_METHOD(GetFeature)
    {
        string enc = arg["Encode"].To<string>("Base64");
        ByteBuilder finger(64);
        if(!_fingerAdapter.GetFeature(finger))
            return false;

        ByteBuilder buff(64);
        CommandDriverHelper::Encoding(enc.c_str(), finger, buff);
        rlt.PushValue("Finger", buff.GetString());
        return true;
    }
    /**
     * @brief 获取指纹模板
     * @date 2016-06-12 20:10
     * 
     * @param [in] Encode : string 指纹格式
     * - 参数:
     *  - Hex
     *  - Base64
     * .
     * @retval Finger : string 
     */
    LC_CMD_METHOD(GetTemplate)
    {
        string enc = arg["Encode"].To<string>("Base64");
        ByteBuilder finger(64);
        if(!_fingerAdapter.GetTemplate(finger))
            return false;

        ByteBuilder buff(64);
        CommandDriverHelper::Encoding(enc.c_str(), finger, buff);
        rlt.PushValue("Finger", buff.GetString());
        return true;
    }
    LC_CMD_METHOD(SaveFingerImage)
    {
        string path = arg["Path"].To<string>();
        ByteBuilder image(2048);
        if(!_fingerAdapter.GetImage(image))
            return false;
        return WE_FingerDevAdapter::ToBmp(image, path.c_str());
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WE_FINGERCMDDRIVER_H_
//========================================================= 