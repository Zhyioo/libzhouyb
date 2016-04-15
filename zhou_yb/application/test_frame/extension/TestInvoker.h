//========================================================= 
/**@file TestInvoker.h
 * @brief DLL测试驱动接口封装 
 * 
 * @date 2015-04-07   21:04:49
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TESTINVOKER_H_
#define _LIBZHOUYB_TESTINVOKER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
#include "../../../include/Container.h"

#include "../../tools/ITestModule.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
namespace extension {
//--------------------------------------------------------- 
/// 对DLL中测试接口的封装 
class TestInvoker
{
protected:
    //----------------------------------------------------- 
    DoTestCallback _doTest;
    HMODULE _hDll;
    //----------------------------------------------------- 
    inline void _init()
    {
        _hDll = NULL;
        _doTest = NULL;

        DevArg = "";
        PreArg = "";
        TestArg = "";
        AppArg = "";
        Title = "";

        ID = 0;
        MaxCount = 0;
        OKCount = 0;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    TestInvoker()
    {
        _init();
    }
    //----------------------------------------------------- 
    /// 测试时连接设备的参数 
    string DevArg;
    /// 测试时初始化参数 
    string PreArg;
    /// 测试参数 
    string TestArg;
    /// 应用参数 
    string AppArg;
    /// 测试名称 
    string Title;
    /// 测试说明 
    string Tips;
    /// ID号 
    size_t ID;
    /// 最大测试次数,正数表示需要全部测试过,负数表示只要成功一次即成功  
    size_t MaxCount;
    /// 多次测试后将结果判定位成功的次数 
    size_t OKCount;
    //----------------------------------------------------- 
    bool Load(const char* driver, const char* entryPoint)
    {
        UnLoad();

        CharConverter cvt;
        _hDll = LoadLibrary(cvt.to_char_t(driver));
        if(_hDll == NULL)
            return false;

        _doTest = (DoTestCallback)GetProcAddress(_hDll, entryPoint);
        if(_doTest == NULL)
        {
            UnLoad();
            return false;
        }

        return true;
    }
    bool Load(const IniGroup& grp)
    {
        if(!Load(grp["Driver"].Value.c_str(), grp["EntryPoint"].Value.c_str()))
            return false;

        Title = grp.Name;
        DevArg = grp["DevArg"].Value;
        PreArg = grp["PreArg"].Value;
        TestArg = grp["TestArg"].Value;
        AppArg = grp["AppArg"].Value;

        Tips = grp["Tips"].Value;

        string sCount = grp["TestCount"].Value;
        if(sCount.length() > 0)
        {
            MaxCount = ArgConvert::FromString<size_t>(sCount.c_str());
            OKCount = MaxCount;

            sCount = grp["OKCount"].Value;
            if(sCount.length() > 0)
            {
                OKCount = ArgConvert::FromString<size_t>(sCount.c_str());
                if(OKCount > MaxCount)
                    OKCount = MaxCount;
            }
        }

        return true;
    }
    inline bool IsValid() const
    {
        return _hDll != NULL && _doTest != NULL;
    }
    bool OnTest(TextPrinter::TextPrintCallback testTextPrintCallBack, Ref<IInterrupter>& interruptRef)
    {
        if(_doTest)
            return _doTest(testTextPrintCallBack, DevArg.c_str(), PreArg.c_str(), TestArg.c_str(), AppArg.c_str(), &interruptRef) == TRUE;
        return false;
    }
    void UnLoad()
    {
        if(_hDll)
        {
            FreeLibrary(_hDll);
            _hDll = NULL;

            _doTest = NULL;
        }
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace extension
} // namespace test
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TESTINVOKER_H_
//========================================================= 