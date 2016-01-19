//========================================================= 
/**@file IDevUpdater.h 
 * @brief 设备固件升级接口DLL函数导出声明(使用自动的工具宏导出为C方式) 
 * 
 * @date 2015-01-02   16:00:45 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../updater/DevUpdater.h"
using zhou_yb::application::updater::IDevUpdater;
using zhou_yb::application::updater::DevUpdater;

#include "../../extension/ability/TextPrinter.h"
using zhou_yb::extension::ability::TextPrinter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace tools {
//--------------------------------------------------------- 
/// 升级工具的辅助函数 
class DevUpdaterHelper
{
protected:
    DevUpdaterHelper() {}
public:
    /**
     * @brief 适配相应的设备完成升级功能 
     *
     * - 所实现的升级机制
     *  - 1.输出相应步骤的信息 
     *  - 2.循环的轮询设备,是否连接成功(Link)
     *  - 3.打开并转换相应的升级文件 
     *  - 4.开始升级 
     * .
     * 
     * @param [in] updater 需要适配的升级设备 
     * @param [in] sPath 升级文件路径 
     * @param [in] sArg [default:NULL] 升级时Link的参数
     * @param [in] textPrint [default:NULL] 测试时信息输出回调函数
     * @param [in] interruptRef [default:NULL] 中断等待超时的信号(NULL表示不需要)
     * @param [in] waitTimeout [default:DEV_WAIT_TIMEOUT] 测试中等待的超时时间(ms)
     * @param [in] operatorInterval [default:DEV_OPERATOR_INTERVAL] 等待时轮询的间隔时间(ms)
     * @retval bool 升级是否成功 
     */
    static bool Update(IDevUpdater& updater, const char* sPath, const char* sArg = NULL, 
        TextPrinter::TextPrintCallback textPrint = NULL, Ref<IInterrupter> interruptRef = Ref<IInterrupter>(),
        uint waitTimeout = DEV_WAIT_TIMEOUT, uint operatorInterval = DEV_OPERATOR_INTERVAL)
    {
        TextPrinter printer;
        printer.Select(textPrint);
        Timer timer;

        printer.TextPrint(TextPrinter::TextTips, "连接设备中...");
        while(timer.Elapsed() < waitTimeout)
        {
            if(updater.Link(sArg))
            {
                printer.TextPrint(TextPrinter::TextMessage, "连接设备成功");
                break;
            }

            if(!interruptRef.IsNull() && interruptRef->InterruptionPoint())
            {
                printer.TextPrint(TextPrinter::TextError, "连接设备失败,操作被取消");
                return false;
            }
            Timer::Wait(operatorInterval);
        }

        if(!updater.IsValid())
        {
            printer.TextPrint(TextPrinter::TextError, "连接设备失败");
            return false;
        }

        // 准备升级数据 
        bool bUpdate = false;
        printer.TextPrint(TextPrinter::TextTips, "升级数据初始化中...");
        timer.Restart();
        while(timer.Elapsed() < waitTimeout)
        {
            if(updater.PreUpdate(sPath))
            {
                printer.TextPrint(TextPrinter::TextMessage, "数据初始化成功");
                bUpdate = true;
                break;
            }

            if(!interruptRef.IsNull() && interruptRef->InterruptionPoint())
            {
                printer.TextPrint(TextPrinter::TextError, "数据初始化失败,操作被取消");
                return false;
            }
            Timer::Wait(operatorInterval);
        }

        if(!bUpdate)
        {
            printer.TextPrint(TextPrinter::TextError, "加载升级数据失败");
            return false;
        }

        printer.TextPrint(TextPrinter::TextTips, "升级中...");
        bUpdate = updater.Update();
        updater.UnLink();

        if(bUpdate)
        {
            printer.TextPrint(TextPrinter::TextInfo, "升级成功");
        }
        else
        {
            printer.TextPrint(TextPrinter::TextError, "升级失败");
        }
        
        return bUpdate;
    };
};
//--------------------------------------------------------- 
} // namespace tools
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
/* 宏声明 */
/// 函数调用方式
#ifndef DEVUPDATER_CALL
#   define DEVUPDATER_CALL __cdecl
#endif
/// DLL函数导出方式 
#ifndef DEVUPDATER_API
#   define DEVUPDATER_API extern "C" __declspec(dllexport)
#endif
//--------------------------------------------------------- 
/* 升级工具DLL需要支持的接口 */
typedef int(DEVUPDATER_CALL *UpdateByFileCallback)(
    const char* sPath, 
    const char* sArg, 
    TextPrinter::TextPrintCallback textPrint, 
    DevUpdater::UpdateProcessCallback updateProcess,
    Ref<IInterrupter>* pInterruptRef);
//---------------------------------------------------------
/// 导出相应的函数 
#define DEVUPDATER_EXPORT(UpdaterType, funcName) \
    DEVUPDATER_API int DEVUPDATER_CALL funcName( \
        const char* sPath, \
        const char* sArg, \
        TextPrinter::TextPrintCallback textPrint, \
        DevUpdater::UpdateProcessCallback updateProcess, \
        Ref<IInterrupter>* pInterruptRef) \
    { \
        UpdaterType updater; \
        updater.SetProcessCallback(updateProcess); \
        Ref<IInterrupter> interrupter; \
        if(pInterruptRef != NULL) \
            interrupter = (*pInterruptRef); \
        updater.Interrupter = interrupter; \
        bool bUpdate = DevUpdaterHelper::Update(updater, sPath, sArg, textPrint, interrupter); \
        return ToBOOL(bUpdate); \
    }
//--------------------------------------------------------- 
//========================================================= 