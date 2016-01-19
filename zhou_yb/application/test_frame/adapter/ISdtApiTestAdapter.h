//========================================================= 
/**@file ISdtApiTestAdapter.h 
 * @brief 公安部身份证SdtApi设备测试适配器 
 *
 * 支持等待身份证放入和拿走的动作 
 * 
 * @date 2015-01-24   23:07:05 
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
 * @brief 公安部SdtApi测试模块
 *
 * 提供基本的卡片连接功能,具体的APDU指令测试项由测试案例提供
 *
 * @param [in] TDevice 底层设备类型
 * @param [in] TLinker 设备连接器类型
 * @param [in] TContainer 测试接口容器类型(接口必须支持ISdtApi转换)
 */
template<class TDevice,
    class TLinker = TestLinker<TDevice>,
    class TContainer = TestContainer<TDevice, ISdtApi> >
class ISdtApiTestAdapter : public TestAdapter<TDevice, ISdtApi, TLinker, TContainer>
{
public:
    //----------------------------------------------------- 
    /// 底层设备类型 
    typedef TDevice DeviceType;
    /// 测试的接口类型 
    typedef ISdtApi InterfaceType;
    /// 测试接口转换类型 
    typedef TContainer ContainerType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// ID卡接口容器 
    TContainer _idContainer;
    /// 身份证阅读器接口 
    Ref<ISdtApi> _pID;
    //----------------------------------------------------- 
    /// 测试前等待卡片进入事件 
    inline bool _IsPresent()
    {
        return _pID->FindIDCard();
    }
    /// 测试后等待卡片移走事件 
    inline bool _IsRemoved()
    {
        // 卡片上电失败则认为已经拿走卡片 
        return !_IsPresent();
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 检测是否有卡可以测试
     */
    virtual bool PreTest(const char* )
    {
        ASSERT_Func(IsValid());
        _pID = _idContainer.Adapter(_pDev, reinterpret_cast<ISdtApi*>(NULL), *this);
        ASSERT_Func(!_pID.IsNull());

        TextPrint(TextTips, "检测身份证中,请放卡...");

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
    virtual bool OnTest(ITestCase<ISdtApi>& testCase, const ByteArray& testArg)
    {
        ASSERT_Func(IsValid() && !_pID.IsNull());

        bool bTest = testCase.Test(_pID, testArg, *this);

        if(!bTest)
        {
            return false;
        }

        if(IsDelay)
        {
            TextPrint(TextInfo, "测试成功,请将身份证拿走以结束测试...");

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

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 