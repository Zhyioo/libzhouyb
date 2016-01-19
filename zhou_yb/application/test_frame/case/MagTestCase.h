//========================================================= 
/**@file MagTestCase.h 
 * @brief 磁条测试案例 
 * 
 * @date 2015-01-24   19:49:55 
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
/// 基本的读磁条测试
struct MagReadTestCase : ITestCase< IInteractiveTrans >
{
    /**
     * @brief 读磁条测试,三个磁道的数据不能为空 
     * @param [in] testObj 测试的接口对象 
     * @param [in] testArg 读取磁条的模式 "123","23"
     * @param [in] printer 信息输出器  
     */ 
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "磁条读功能测试");

        MagneticDevAdapter magAdapter;
        magAdapter.SelectDevice(testObj);

        char tr1[256] = { 0 };
        char tr2[256] = { 0 };
        char tr3[256] = { 0 };

        string msg;
        msg = "读<";
        msg += testArg.GetString();
        msg += ">磁道,请刷卡...";

        printer.TextPrint(TextPrinter::TextTips, msg.c_str());

        if(MagneticDevAdapterHelper::ReadMagneticCardWithCheck(magAdapter, testArg.GetString(), tr1, tr2, tr3) ==
            DevHelper::EnumSUCCESS)
        {
            msg = "磁道数据:";
            msg += tr1;
            msg += ',';
            msg += tr2;
            msg += ',';
            msg += tr3;

            printer.TextPrint(TextPrinter::TextMessage, msg.c_str());

            return true;
        }

        return false;
    }
};
//--------------------------------------------------------- 
/// 基本的写磁条测试
struct MagWriteTestCase : ITestCase < IInteractiveTrans >
{
    /// 随机生成指定长度的磁条数据 
    static void RandTrack(char* tr, size_t len)
    {
        time_t tRand;
        time(&tRand);
        srand(static_cast<uint>(tRand));

        const char TR_RAND[] = "0123456789=";
        for(size_t i = 0; i < len; ++i)
        {
            tr[i] = TR_RAND[rand() % 11];
        }
    }
    /**
     * @brief 写磁条测试,将随机生成数据写入后再读取比对 
     * @param [in] testObj 测试的接口对象
     * @param [in] testArg 写磁条的模式 "123","23"
     * @param [in] printer 信息输出器
     */
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "磁条写功能测试");

        MagneticDevAdapter magAdapter;
        LOGGER(magAdapter.SelectLogger(printer.GetLogger()));
        magAdapter.SelectDevice(testObj);

        char tr1[256] = { 0 };
        char tr2[256] = { 0 };
        char tr3[256] = { 0 };

        // 随机生成数据 
        RandTrack(tr1, 75);
        RandTrack(tr2, 35);
        RandTrack(tr3, 100);

        string msg;
        msg = "写<";
        msg += testArg.GetString();
        msg += ">磁道,请刷卡...";

        printer.TextPrint(TextPrinter::TextTips, msg.c_str());

        msg = "写入数据:";
        msg += tr1;
        msg += ',';
        msg += tr2;
        msg += ',';
        msg += tr3;

        printer.TextPrint(TextPrinter::TextMessage, msg.c_str());

        if(MagneticDevAdapterHelper::WriteMagneticCardWithCheck(magAdapter, testArg.GetString(), tr1, tr2, tr3) != DevHelper::EnumSUCCESS)
        {
            printer.TextPrint(TextPrinter::TextLogger, "写磁条数据失败");
            return false;
        }

        msg = "写入后重新读取数据,请刷卡...";
        printer.TextPrint(TextPrinter::TextTips, msg.c_str());

        char _tr1[256] = { 0 };
        char _tr2[256] = { 0 };
        char _tr3[256] = { 0 };

        if(MagneticDevAdapterHelper::ReadMagneticCardWithCheck(magAdapter, testArg.GetString(), _tr1, _tr2, _tr3) != DevHelper::EnumSUCCESS)
        {
            printer.TextPrint(TextPrinter::TextLogger, "写入后读取磁条数据失败");
            return false;
        }

        bool bCompare = true;
        if(bCompare && StringConvert::ContainsChar(testArg, '1'))
        {
            if(strcmp(tr1, _tr1) != 0)
            {
                bCompare = false;
                printer.TextPrint(TextPrinter::TextLogger, "一磁道数据比对失败");
            }
        }
        if(bCompare && StringConvert::ContainsChar(testArg, '2'))
        {
            if(strcmp(tr2, _tr2) != 0)
            {
                bCompare = false;
                printer.TextPrint(TextPrinter::TextLogger, "二磁道数据比对失败");
            }
        }
        if(bCompare && StringConvert::ContainsChar(testArg, '3'))
        {
            if(strcmp(tr3, _tr3) != 0)
            {
                bCompare = false;
                printer.TextPrint(TextPrinter::TextLogger, "三磁道数据比对失败");
            }
        }

        if(!bCompare)
            printer.TextPrint(TextPrinter::TextLogger, "磁条数据写入后比对失败");

        return bCompare;
    }
};
/// 磁条参数测试
struct MagConfigurationTestCase : ITestCase< IInteractiveTrans >
{
    /**
    * @brief 读磁条测试,三个磁道的数据不能为空
    * @param [in] testObj 测试的接口对象
    * @param [in] testArg 磁条的参数 [TrBpi:<75>][TrMode:<High/Low/None>][TrI:<240>] // 二磁道BPI,高低抗模式,电流(mA)
    * @param [in] printer 信息输出器
    */
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter& printer)
    {
        printer.TextPrint(TextPrinter::TextNote, "磁条参数设置测试");

        ArgParser cfg;
        // 参数格式错误,不需要进行设置
        if(!cfg.Parse(testArg.GetString()))
        {
            printer.TextPrint(TextPrinter::TextLogger, "没有解析到需要设置的参数");
            return true;
        }

        int trBpi = 0;
        bool bRet = true;
        ArgConvert::FromConfig<int>(cfg, "TrBpi", trBpi);
        switch(trBpi)
        {
        case 75:
            printer.TextPrint(TextPrinter::TextLogger, "设置:75BPI");
            bRet = MagneticDevAdapter::SetTrack2BPI(testObj, MagneticDevAdapter::Bpi75);
            break;
        case 210:
            printer.TextPrint(TextPrinter::TextLogger, "设置:210BPI");
            bRet = MagneticDevAdapter::SetTrack2BPI(testObj, MagneticDevAdapter::Bpi210);
            break;
        }
        if(!bRet)
        {
            printer.TextPrint(TextPrinter::TextLogger, "设置BPI失败");
            return false;
        }

        string trMode = "None";
        ArgConvert::FromConfig<string>(cfg, "TrMode", trMode);
        // 不设置 
        if(StringConvert::IsEqual(trMode.c_str(), "None", true))
        {
            printer.TextPrint(TextPrinter::TextLogger, "设置:None");
        }
        else if(StringConvert::IsEqual(trMode.c_str(), "High", true))
        {
            // 高抗 
            printer.TextPrint(TextPrinter::TextLogger, "设置:高抗");
            bRet = MagneticDevAdapter::SetImpedanceMode(testObj, MagneticDevAdapter::HighImpedance);
        }
        else if(StringConvert::IsEqual(trMode.c_str(), "Low", true))
        {
            // 抵抗
            printer.TextPrint(TextPrinter::TextLogger, "设置:抵抗");
            bRet = MagneticDevAdapter::SetImpedanceMode(testObj, MagneticDevAdapter::LowImpedance);
        }
        if(!bRet)
        {
            printer.TextPrint(TextPrinter::TextLogger, "设置阻抗模式失败");
            return false;
        }

        uint trI = 0;
        if(ArgConvert::FromConfig<uint>(cfg, "TrI", trI))
        {
            string msg = "设置电流:";
            msg += ArgConvert::ToString(trI);
            printer.TextPrint(TextPrinter::TextLogger, msg.c_str());

            bRet = MagneticDevAdapter::SetTrElectric(testObj, trI);
            if(!bRet)
            {
                printer.TextPrint(TextPrinter::TextLogger, "设置电流失败");
                return false;
            }
        }

        return true;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 