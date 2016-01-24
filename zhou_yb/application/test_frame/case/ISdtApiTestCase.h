//========================================================= 
/**@file ISdtApiTestCase.h 
 * @brief 公安部SdtApi接口设备测试案例 
 * 
 * @date 2015-01-24   22:58:02 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"
#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// SDTAPI接口设备身份证测试案例 
struct ISdtApiTestCase : public ITestCase < ISdtApi >
{
    /// 读取身份证基本信息,并显示身份证号 
    virtual bool Test(Ref<ISdtApi>& testObj, const ByteArray& , TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "读身份证号测试");
        // 寻卡
        if(!testObj->FindIDCard())
        {
            printer.TextPrint(TextPrinter::TextLogger, "寻卡失败");
            return false;
        }
        // 选卡
        if(!testObj->SelectIDCard())
        {
            printer.TextPrint(TextPrinter::TextLogger, "选卡失败");
            return false;
        }
        // 读卡 
        ByteBuilder txtInfo(256);
        if(!testObj->ReadBaseMsg(&txtInfo, NULL))
        {
            printer.TextPrint(TextPrinter::TextLogger, "读卡失败");
            return false;
        }
        // 解析数据 
        IDCardParser idParser;
        wIDCardInformation wIdInfo;
        if(!idParser.ParseToTXT(txtInfo, wIdInfo))
        {
            printer.TextPrint(TextPrinter::TextLogger, "解析数据失败");
            return false;
        }

        CharConvert convert;
        string msg = "身份证号:";
        msg += convert.to_char(wIdInfo.ID.c_str());
        printer.TextPrint(TextPrinter::TextMessage, msg.c_str());

        return true;
    }

};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 