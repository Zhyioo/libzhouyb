//========================================================= 
/**@file LastErr_Simple.h
 * @brief 
 * 
 * @date 2016-05-05   20:53:09
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LASTERR_SIMPLE_H_
#define _LIBZHOUYB_LASTERR_SIMPLE_H_
//--------------------------------------------------------- 
#include "../zhouyb_lib.h"

#include "../application/idcard/IDCardDevAdapter.h"
using zhou_yb::application::idcard::IDCardDevAdapter;
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct LastErr_Simple
{
    static void DisplayErrMsg(ILastErrBehavior& lasterr)
    {

    }
    static void Main(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        HidDevice dev;
        HidCmdAdapter<HidDevice, 0, 1> hidCmdAdapter;
        CommandFilter cmdFilter;
        ComICCardCmdAdapter cmdAdapter;
        ComContactICCardDevAdapter contactIC;
        ComContactlessICCardDevAdapter contactlessIC;
        PBOC_v2_0_AppAdapter icAdapter;
        IDCardDevAdapter idDevAdapter;
        SdtApiDevAdapter sdtapiAdapter;
        IDCardParser idParser;

        LastErrExtractor icLastErr;
        LastErrExtractor pbocLastErr;
        LastErrExtractor idLastErr;
        LastErrExtractor appLastErr;
        LastErrExtractor lastErr;

        hidCmdAdapter.SelectDevice(dev);
        cmdFilter.SelectDevice(hidCmdAdapter);
        cmdAdapter.SelectDevice(cmdFilter);
        contactIC.SelectDevice(cmdAdapter);
        contactlessIC.SelectDevice(cmdAdapter);

        idDevAdapter.SelectDevice(dev);
        sdtapiAdapter.SelectDevice(idDevAdapter);
        idParser.SelectDevice(sdtapiAdapter);

        cmdFilter.SetReadTX(0x02, 0x03);
        dev.SetWaitTimeout(500);
        idParser.SetWaitTimeout(500);

        HidDeviceHelper::OpenDevice<HidDevice>(dev, "BP8903-H002");
        dev.Write(DevCommand::FromAscii("1B 24 49"));
        contactlessIC.PowerOn();

        icLastErr.Select(contactIC, "ContactIC");
        icLastErr.Select(contactlessIC, "ContactlessIC");
        icLastErr.IsLayerMSG = false;

        pbocLastErr.Select(icLastErr, "IcDev");
        pbocLastErr.Select(icAdapter, "IcAdapter");
        pbocLastErr.IsLayerMSG = true;

        idLastErr.Select(sdtapiAdapter, "SdtApi");
        idLastErr.Select(idParser, "IdParser");
        idLastErr.IsLayerMSG = true;

        appLastErr.Select(pbocLastErr, "PBOC");
        appLastErr.Select(idLastErr, "ID");
        appLastErr.IsLayerMSG = false;

        lastErr.Select(dev, "DEV");
        lastErr.Select(cmdAdapter, "CMD");
        lastErr.Select(appLastErr, "APP");
        lastErr.IsLayerMSG = true;

        ByteBuilder msg(8);
        icAdapter.SelectDevice(contactIC);
        icAdapter.GetCardNumber(msg);
        DisplayErrMsg(icAdapter);
        DisplayErrMsg(lastErr);
        lastErr.ResetErr();

        icAdapter.GetCardNumber(msg);
        DisplayErrMsg(icAdapter);
        DisplayErrMsg(lastErr);
        lastErr.ResetErr();

        idParser.GetBaseInformation(msg);
        DisplayErrMsg(idParser);
        DisplayErrMsg(lastErr);
        lastErr.ResetErr();

        contactIC.PowerOn();
        DisplayErrMsg(contactIC);
        DisplayErrMsg(lastErr);
        lastErr.ResetErr();

        sdtapiAdapter.FindIDCard();
        DisplayErrMsg(sdtapiAdapter);
        DisplayErrMsg(lastErr);
        lastErr.ResetErr();
    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LASTERR_SIMPLE_H_
//========================================================= 