//========================================================= 
/**@file ICC_TestCase.h
 * @brief IC卡基本测试案例
 *
 * @date 2015-01-12   17:16:52
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
/// 基本的IC卡测试 
struct ICC_BaseTestCase : public ITestCase<ITransceiveTrans>, public ITestCase<IICCardDevice>
{
    /// 交换APDU 
    inline bool AutoApdu(Ref<ITransceiveTrans>& testObj, const ByteArray& sApdu, ByteBuilder& rApdu)
    {
        return ICCardDevice::AutoApdu(*testObj, sApdu, rApdu);
    }
    /// 子类需要修改覆盖的函数 
    virtual bool Test(Ref<ITransceiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer) = 0;
    /// 接口适配  
    virtual bool Test(Ref<IICCardDevice>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        Ref<ITransceiveTrans> dev(testObj);
        return Test(dev, testArg, printer);
    }
};
//--------------------------------------------------------- 
/**
* @brief IC卡取随机数测试案例
* 发送0084000004测试获取随机数,不判断状态码  
*/
struct ICC_RandomTestCase : public ICC_BaseTestCase
{
    /// 取随机数 
    virtual bool Test(Ref<ITransceiveTrans>& testObj, const ByteArray& , TextPrinter& printer) 
    {
        printer.TextPrint(TextPrinter::TextNote, "取随机数测试");

        ByteBuilder cmd(8);
        ByteBuilder random(4);
        
        DevCommand::FromAscii("00 84 00 00 04", cmd);

        if(AutoApdu(testObj, cmd, random))
        {
            if(ICCardLibrary::GetSW(random) == 0x6300)
                return false;

            string msg = "";
            msg += "随机数:";
            msg += ArgConvert::ToString(random);

            printer.TextPrint(TextPrinter::TextMessage, msg.c_str());

            return true;
        }

        return false;
    }
};
//--------------------------------------------------------- 
/**
* @brief 发送指定的APDU 
*/
struct ICC_TransCommandTestCase : public ICC_BaseTestCase
{
    /// 交换APDU  
    virtual bool Test(Ref<ITransceiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "交换IC卡指令测试");

        ByteBuilder rApdu(4);
        if(testObj->TransCommand(testArg, rApdu))
        {
            string msg = "";
            msg += "卡片返回:";
            msg += ArgConvert::ToString(rApdu);

            printer.TextPrint(TextPrinter::TextMessage, msg.c_str());
            return true;
        }

        return false;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 发送指定的APDU,并判断状态码是否为9000 
 */ 
struct ICC_ApduTestCase : public ICC_BaseTestCase
{
    /// 交换APDU  
    virtual bool Test(Ref<ITransceiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "交换APDU指令测试");

        ByteBuilder rApdu(4);
        if(AutoApdu(testObj, testArg, rApdu))
        {
            string msg = "";
            msg += "卡片返回:";
            msg += ArgConvert::ToString(rApdu);

            printer.TextPrint(TextPrinter::TextMessage, msg.c_str());
            return true;
        }

        return false;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 