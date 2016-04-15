//========================================================= 
/**@file AT88SC102_TestCase.h 
 * @brief AT88SC102卡测试案例   
 * 
 * @date 2015-01-11   20:16:52 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_AT88SC102_TESTCASE_H_
#define _LIBZHOUYB_AT88SC102_TESTCASE_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../device/iccard/extension/base/IMemoryCard.h"
using zhou_yb::device::iccard::extension::base::IMemoryCard;

#include "../../../device/iccard/pboc/v1_0/PBOC_v1_0_Library.h"
using namespace zhou_yb::device::iccard::pboc;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief AT88SC102卡测试案例  
 */ 
struct AT88SC102_TestCase : public ITestCase< IMemoryCard >
{
    /// 读写特定区域,并校验用户密码读写测试区 
    virtual bool Test(Ref<IMemoryCard>& testObj, const ByteArray& usrKey, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "AT88SC102卡测试");

        ByteBuilder randomData(2);
        ByteBuilder recvBuff(4);
        ByteConvert::Random(randomData, 2);

        if(!testObj->WriteMemory(0xB0, randomData))
        {
            printer.TextPrint(TextPrinter::TextLogger, "写数据失败");
            return false;
        }
        
        if(!testObj->ReadMemory(0xB0, 2, recvBuff))
        {
            printer.TextPrint(TextPrinter::TextLogger, "读数据失败");
            return false;
        }

        if(randomData != recvBuff)
        {
            printer.TextPrint(TextPrinter::TextLogger, "数据写入后读取比对失败");
            return false;
        }

        if(!testObj->VerifyKey(usrKey))
        {
            printer.TextPrint(TextPrinter::TextLogger, "校验用户密码失败");
            return false;
        }

        randomData.Clear();
        randomData += static_cast<byte>(0x01);

        if(!testObj->WriteMemory(0xFF, randomData))
        {
            printer.TextPrint(TextPrinter::TextLogger, "测试102卡置C8脚为高电平状态失败");
            return false;
        }

        randomData.Clear();
        if(!testObj->ReadMemory(0x0A, 2, randomData))
        {
            printer.TextPrint(TextPrinter::TextLogger, "测试102卡(高电平)读密钥失败");
            return false;
        }

        if(randomData != usrKey)
        {
            printer.TextPrint(TextPrinter::TextLogger, "测试102卡(高电平)读密钥比对失败");
            return false;
        }

        randomData.Clear();
        randomData += static_cast<byte>(0x00);
        if(!testObj->WriteMemory(0xFF, randomData))
        {
            printer.TextPrint(TextPrinter::TextLogger, "测试102卡置C8脚为低电平状态失败");
            return false;
        }

        randomData.Clear();
        if(!testObj->ReadMemory(0x0A, 2, randomData))
        {
            printer.TextPrint(TextPrinter::TextLogger, "测试102卡(低电平)读密钥失败");
            return false;
        }
        if(randomData != DevCommand::FromAscii("FF FF"))
        {
            printer.TextPrint(TextPrinter::TextLogger, "测试102卡(低电平)读密钥比对失败");
            return false;
        }

        return true;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_AT88SC102_TESTCASE_H_
//========================================================= 