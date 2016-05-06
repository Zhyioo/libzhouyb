//========================================================= 
/**@file ICC_TestCase.h
 * @brief IC卡基本测试案例
 *
 * @date 2015-01-12   17:16:52
 * @author Zhyioo
 * @version 1.0
 */
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICC_TESTCASE_H_
#define _LIBZHOUYB_ICC_TESTCASE_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"
#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
* @brief IC卡取随机数测试案例
* 发送0084000004测试获取随机数,不判断状态码  
*/
struct ICC_RandomTestCase : public ITestCase<IICCardDevice>
{
    /// 取随机数 
    virtual bool Test(Ref<IICCardDevice>& testObj, const ByteArray& , TextPrinter& printer) 
    {
        printer.TextPrint(TextPrinter::TextNote, "取随机数测试");

        ByteBuilder cmd(8);
        ByteBuilder random(4);
        
        DevCommand::FromAscii("00 84 00 00 04", cmd);

        if(ICCardDevice::AutoApdu(testObj, cmd, random))
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
struct ICC_TransCommandTestCase : public ITestCase<IICCardDevice>
{
    /// 交换APDU  
    virtual bool Test(Ref<IICCardDevice>& testObj, const ByteArray& testArg, TextPrinter& printer)
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
struct ICC_ApduTestCase : public ITestCase<IICCardDevice>
{
    /// 交换APDU  
    virtual bool Test(Ref<IICCardDevice>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "交换APDU指令测试");

        ByteBuilder rApdu(4);
        if(ICCardDevice::AutoApdu(testObj, testArg, rApdu))
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
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICC_TESTCASE_H_
//========================================================= 