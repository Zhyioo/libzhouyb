//========================================================= 
/**@file BaseTestCase.h
 * @brief 基础测试案例 
 * 
 * @date 2015-06-26   21:25:36
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// 发送指定指令测试案例 
struct CommandSendTestCase : public ITestCase < IInteractiveTrans >
{
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "Command Send 测试");
        return testObj->Write(testArg);
    }
};
//--------------------------------------------------------- 
} // namespace test
} // namespace application 
} // namespace zhou_yb
//========================================================= 