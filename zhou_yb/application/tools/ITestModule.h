//========================================================= 
/**@file ITestModule.h 
 * @brief 测试模块接口DLL函数导出声明(使用自动的工具宏导出为C方式) 
 * 
 * @date 2015-01-02   16:01:20 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ITESTMODULE_H_
#define _LIBZHOUYB_ITESTMODULE_H_
//--------------------------------------------------------- 
#include "../../extension/ability/TextPrinter.h"
using zhou_yb::extension::ability::TextPrinter;
//--------------------------------------------------------- 
/* 宏声明 */
/// 函数调用方式 
#ifndef TEST_CALL 
#   define TEST_CALL __cdecl
#endif
/// 函数导出方式 
#ifndef TEST_API
#   define TEST_API EXTERN_C __declspec(dllexport)
#endif 
/// 测试模块的导出函数 
#define TEST_EXPORT(funcName) \
    TEST_API int TEST_CALL funcName(TextPrinter::TextPrintCallback testTextPrintCallBack, \
        const char* devArg, const char* preArg, const char* testArg, const char* appArg, Ref<IInterrupter>* pInterruptRef)
/// 调用其他的测试模块
#define TEST_CALL_METHOD(funcName) \
    funcName(testTextPrintCallBack, devArg, preArg, testArg, appArg, pInterruptRef)
//--------------------------------------------------------- 
/**
 * @brief 动态库中的测试接口
 *
 * @param [in] testTextPrintCallBack 测试的输出回调函数
 * @param [in] pDevArg 连接设备时使用的参数
 * @param [in] pPreArg 测试初始化的参数
 * @param [in] pTestArg 测试的参数
 * @param [in] pAppArg 应用相关配置的参数
 * @param [in] pInterruptRef [default:NULL] 中断信号
 */
typedef int(TEST_CALL *DoTestCallback)(
    TextPrinter::TextPrintCallback testTextPrintCallBack,
    const char* pDevArg,
    const char* pPreArg,
    const char* pTestArg,
    const char* pAppArg,
    Ref<IInterrupter>* pInterruptRef);
//--------------------------------------------------------- 
/* 工具宏 */
/// 测试模块辅助操作 
#define TEST_MODULE_FUNC(testModule) \
    testModule.Select(testTextPrintCallBack); \
    Ref<IInterrupter> interrupter; \
    if(pInterruptRef != NULL) \
        interrupter = (*pInterruptRef); \
    testModule.Interrupter = interrupter; \
    ArgParser appCfg; \
    appCfg.Parse(appArg); \
    testModule.IsDelay = true; \
    ArgConvert::FromConfig<bool>(appCfg, "IsDelay", testModule.IsDelay); \
    uint waitTimeout = DEV_WAIT_TIMEOUT; \
    ArgConvert::FromConfig<uint>(appCfg, "WaitTime", waitTimeout); \
    testModule.SetWaitTimeout(waitTimeout); \
    uint interval = DEV_TEST_INTERVAL; \
    ArgConvert::FromConfig<uint>(appCfg, "Interval", interval); \
    testModule.SetOperatorInterval(interval)
/// 返回测试结果 
#define return_TEST(testModule) return ToBOOL(testModule.Test(devArg, preArg))
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ITESTMODULE_H_
//========================================================= 