//========================================================= 
/**@file ICCardTestAdapter.h 
 * @brief IC卡设备测试程序 
 *
 * IC卡类的测试适配器,支持识别卡片放上和拿走  
 * 
 * @date 2015-01-10   15:48:25 
 * @author Zhyioo 
 * @version 1.0
 */
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/BaseDevice.h"
#include "../../../include/Device.h"
#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief IC卡测试模块
 * 
 * 提供基本的卡片连接功能,测试的接口和实现由子类去实现 
 *
 * @param [in] TDevice 底层设备类型  
 * @param [in] TInterface 测试案例类型 
 * @param [in] TLinker 设备连接器类型
 * @param [in] TICCardContainer 设备转换为IICCardDevice的转换器 
 */ 
template<class TDevice, 
    class TInterface,
    class TLinker = TestLinker<TDevice>,
    class TICCardContainer = TestContainer<TDevice, IICCardDevice>>
class ICCardBaseTestAdapter : public TestAdapter<TDevice, IICCardDevice, TLinker, TICCardContainer>
{
public:
    //----------------------------------------------------- 
    /// 底层设备类型 
    typedef TDevice DeviceType;
    /// 测试的接口类型 
    typedef TInterface InterfaceType;
    /// 测试接口转换类型 
    typedef TICCardContainer ContainerType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 上电的参数 
    string _sPowerOnArg;
    //----------------------------------------------------- 
    inline void _init()
    {
        _sPowerOnArg = "";
    }
    //----------------------------------------------------- 
    /// 测试前等待卡片进入事件 
    virtual bool _IsPresent()
    {
        bool isPresent = _testInterface->PowerOn(_sPowerOnArg.c_str());
        if(isPresent)
            _testInterface->PowerOff();

        return isPresent;
    }
    /// 测试后等待卡片移走事件 
    virtual bool _IsRemoved()
    {
        // 卡片上电失败则认为已经拿走卡片 
        return !_IsPresent();
    }
    /// 检测到卡片后实际的测试动作 
    virtual bool _OnTest(ITestCase<TInterface>& testCase, const ByteArray& testArg) = 0;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ICCardBaseTestAdapter() : TestAdapter() { _init(); }
    //----------------------------------------------------- 
    /**
     * @brief 检测是否有卡可以测试 
     */
    virtual bool PreTest(const char* preArg)
    {
        ASSERT_Func(TestAdapter::PreTest(preArg));
        if(!_is_empty_or_null(preArg))
            _sPowerOnArg = preArg;

        TextPrint(TextTips, "检测卡片中,请放卡...");

        if(_IsPresent())
        {
            TextPrint(TextNote, "卡片已连接");
            return true;
        }
        TextPrint(TextWarning, "卡片未连接");

        return false;
    }
    /**
     * @brief 开始测试 
     * @param [in] testCase 测试案例 
     * @param [in] testArg 测试的参数 
     */ 
    virtual bool OnTest(ITestCase<TInterface>& testCase, const ByteArray& testArg)
    {
        ASSERT_Func(IsValid() && !_testInterface.IsNull());
        bool bTest = _OnTest(testCase, testArg);

        if(IsDelay)
        {
            if(bTest)
                TextPrint(TextInfo, "测试成功,请移走卡片以结束测试...");
            else
                TextPrint(TextError, "测试失败,请移走卡片重新测试...");
            
            while(IsDelay)
            {
                // 如果卡片被移走,或者设备直接被移除则直接返回 
                if(_IsRemoved())
                {
                    TextPrint(TextWarning, "卡片已移除");
                    break;
                }
                if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
                {
                    TextPrint(TextWarning, "卡片检测已取消");
                    break;
                }
                Timer::Wait(DEV_OPERATOR_INTERVAL);
            }
        }

        return bTest;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/**
 * @brief IC卡测试模块,可以从IICCardDevice转换为测试接口TInterface的适配器
 *
 * 提供基本的卡片连接功能,将IICCardDevice转换为具体的IC卡测试接口提供测试功能  
 *
 * @param [in] TDevice 底层设备类型
 * @param [in] TInterface 测试案例类型
 * @param [in] TLinker 设备连接器类型
 * @param [in] TICCardContainer 设备转换为IICCardDevice的转换器
 * @param [in] TContainer 测试接口容器类型(接口必须支持IICCardDevice转到TInterface)
 */
template<class TDevice,
    class TInterface = IICCardDevice,
    class TLinker = TestLinker<TDevice>,
    class TICCardContainer = TestContainer<TDevice, IICCardDevice>,
    class TContainer = TestContainer<IICCardDevice, TInterface >>
class ICCardTestAdapter : public ICCardBaseTestAdapter<TDevice, TInterface, TLinker, TICCardContainer>
{
public:
    //----------------------------------------------------- 
    /// 底层设备类型 
    typedef TDevice DeviceType;
    /// 测试的接口类型 
    typedef IICCardDevice InterfaceType;
    /// 测试接口转换类型 
    typedef TICCardContainer ContainerType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    virtual bool _OnTest(ITestCase<TInterface>& testCase, const ByteArray& testArg)
    {
        ByteBuilder atr(32);
        if(!_testInterface->PowerOn(_sPowerOnArg.c_str(), &atr))
        {
            TextPrint(TextError, "测试时连接卡片失败...");
            return false;
        }
        string msg = "ATR:";
        msg += ArgConvert::ToString(atr);
        TextPrint(TextMessage, msg.c_str());

        TContainer testContainer;
        Ref<TInterface> pInterface = testContainer.TContainer::Adapter(_testInterface, reinterpret_cast<TInterface*>(NULL), *this);
        if(pInterface.IsNull())
        {
            TextPrint(TextError, "测试数据初始化失败");
            return false;
        }

        TextPrint(TextTips, "检测到卡片,测试中,请不要移走卡片...");

        bool bTest = testCase.Test(pInterface, testArg, *this);
        _testInterface->PowerOff();

        return bTest;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ICCardTestAdapter() : ICCardBaseTestAdapter() { _init(); }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/**
 * @brief IC卡测试模块,测试时,不支持IICCardDevice接口转换,需要直接从TDevice转换的IC卡测试适配器 
 *
 * 提供基本的卡片连接功能,将IICCardDevice转换为具体的IC卡测试接口提供测试功能
 *
 * @param [in] TDevice 底层设备类型
 * @param [in] TInterface 测试案例类型
 * @param [in] TLinker 设备连接器类型
 * @param [in] TICCardContainer 设备转换为IICCardDevice的转换器
 * @param [in] TContainer 测试接口容器类型(接口必须支持TDevice转到TInterface)
 */
template<class TDevice,
    class TInterface,
    class TLinker = TestLinker<TDevice>,
    class TICCardContainer = TestContainer<TDevice, IICCardDevice>,
    class TContainer = TestContainer<TDevice, TInterface> >
class ICCardExTestAdapter : public ICCardBaseTestAdapter<TDevice, TInterface, TLinker, TICCardContainer>
{
public:
    //----------------------------------------------------- 
    /// 底层设备类型 
    typedef TDevice DeviceType;
    /// 测试的接口类型 
    typedef TInterface InterfaceType;
    /// 测试接口转换类型 
    typedef TContainer ContainerType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    virtual bool _OnTest(ITestCase<TInterface>& testCase, const ByteArray& testArg)
    {
        TContainer testContainer;
        Ref<TInterface> pInterface = testContainer.TContainer::Adapter(_pDev, reinterpret_cast<TInterface*>(NULL), *this);
        if(pInterface.IsNull())
        {
            TextPrint(TextError, "测试数据初始化失败");
            return false;
        }

        TextPrint(TextTips, "检测到卡片,测试中,请不要移走卡片...");

        bool bTest = testCase.Test(pInterface, testArg, *this);
        return bTest;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ICCardExTestAdapter() : ICCardBaseTestAdapter() { _init(); }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 