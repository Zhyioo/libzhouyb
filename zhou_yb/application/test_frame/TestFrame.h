//========================================================= 
/**@file TestFrame.h 
 * @brief 测试接口和测试模块实现 
 *
 * - 测试的几个要素:
 *  - 1.测试的底层通信设备,负责设备数据通信 TDevice 
 *  - 2.测试设备的连接器,负责设备的连接操作 TLinker
 *  - 3.测试接口转换器,负责将设备通信的数据封装为应用的测试接口 TContainer
 *  - 4.测试的适配器,用于处理测试的准备流程 TAdapter
 *  - 5.测试案例,用于测试特定的接口 TTestCase
 *  - 6.测试模块,整个上述5个部件为一个整体的测试有机体 TestModule
 * .
 * 
 * @date 2015-01-02   16:01:20 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../extension/ability/TextPrinter.h"
using zhou_yb::extension::ability::TextPrinter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// 测试时的轮询间隔 
#ifndef DEV_TEST_INTERVAL
#   define DEV_TEST_INTERVAL 250
#endif
//--------------------------------------------------------- 
/**
 * @brief 测试案例接口
 *
 * @warning 该类以临时变量的方式来使用,故实现中不能保存私有数据 TestCase().Test();
 */ 
template<class TInterface>
struct ITestCase
{
    typedef TInterface InterfaceType;

    /// 测试案例接口 
    virtual bool Test(Ref<TInterface>& testObj, const ByteArray& testArg, TextPrinter& ) = 0;
};
/**
 * @brief 空的测试案例 
 */
template<class TInterface>
struct TestCase : public ITestCase< TInterface >
{
    /// 测试案例接口 
    virtual bool Test(Ref<TInterface>& , const ByteArray& , TextPrinter&) { return true; }
};
//---------------------------------------------------------   
/**
 * @brief 测试连接器,负责连接设备,默认为空实现
 * 
 * @waining 该类以临时变量的方式来使用,故实现中不能保存私有数据 TestLinker().Link(); TestLinker().UnLink();
 */ 
template<class TDevice>
struct TestLinker
{
    typedef TDevice DeviceType;

    /// 连接设备 
    virtual bool Link(TDevice& , const char* , TextPrinter& )
    {
        return true;
    }
    /// 断开连接 
    virtual bool UnLink(TDevice& , TextPrinter& )
    {
        return true;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 接口转换器接口 
 */ 
template<class TDevice, class TInterface>
struct ITestContainer
{
    typedef TDevice DeviceType;
    typedef TInterface InterfaceType;

    virtual Ref<TInterface> Adapter(const Ref<TDevice>&, TInterface*, TextPrinter&) = 0;
};
/**
 * @brief 测试容器,负责将设备底层接口适配为测试所需要接口 
 * @warning 该类以对象的方式使用,可以保存一定的状态,每个测试只包含一个容器
 * 测试中的所有接口类型都由该对象产生,故转换函数有多个重载 
 * 
 */ 
template<class TDevice, class TInterface>
struct TestContainer : public ITestContainer< TDevice, TInterface >
{
    /**
     * @brief 将一种接口转换为另外一种接口
     *  转换失败则返回无效的Ref引用  
     *  TInterface*参数用来区分重载 
     * @warning 子类实现只需要处理转换,有效性上层会做处理 
     */
    virtual Ref<TInterface> Adapter(const Ref<TDevice>& dev, TInterface*, TextPrinter& )
    {
        return Ref<TInterface>(dev);
    }
};
/**
 * @brief 测试适配转接器(不使用继承)  
 * 
 * 由于库中大部分都使用了Adapter的模式,特提供默认的将指定接口通过适配器转为其他接口的默认实现 
 * 
 */ 
template<class TInterface, class TTransInterface, class TAdapter>
class TestAdapterContainer : public ITestContainer< TInterface, TTransInterface >
{
public:
    typedef TTransInterface TransInterfaceType;
    typedef TAdapter AdapterType;
protected:
    TAdapter _testAdapter;
public:
    virtual Ref<TTransInterface> Adapter(const Ref<TInterface>& dev, TTransInterface*, TextPrinter& )
    {
        _testAdapter.SelectDevice(dev);
        return Ref<TTransInterface>(_testAdapter);
    }
};
/**
 * @brief 用于嵌套的TestAdapterContainer的适配转接 
 */
template<class TAdapterContainer, class TInterface, class TAdapter>
class TestAdapterExtractorContainer : public ITestContainer< typename TAdapterContainer::DeviceType, TInterface >
{
public:
    typedef typename TAdapterContainer::DeviceType DeviceType;
    typedef TAdapterContainer ContainerType;
    typedef typename TAdapterContainer::TransInterfaceType InterfaceType;
    typedef TInterface TransInterfaceType;
protected:
    TAdapterContainer _testContainer;
    TAdapter _testAdapter;
public:
    /// 显示的接口调用转换 
    virtual Ref<TInterface> Adapter(const Ref<DeviceType>& dev, TInterface*, TextPrinter& printer)
    {
        InterfaceType* pInterface = NULL;
        Ref<InterfaceType> ref = _testContainer.Adapter(dev, pInterface, printer);
        _testAdapter.SelectDevice(ref);
        return Ref<TInterface>(_testAdapter);
    }
};
//--------------------------------------------------------- 
/// 测试接口 
template<class TInterface>
struct ITest
{
    /// 连接设备准备测试 
    virtual bool BeginTest(const char* devArg) = 0;
    /// 准备测试数据 
    virtual bool PreTest(const char* preArg) = 0;
    /// 测试 
    virtual bool OnTest(ITestCase<TInterface>& testCase, const ByteArray& testArg) = 0;
    /// 结束测试 
    virtual bool EndTest() = 0;
};
//--------------------------------------------------------- 
/** 
 * @brief 测试适配器 
 * 
 * 将多个分散的模块整合用于定制TestModule
 * 在此仅仅作为一个参考框架,可以参考ICCardTestAdapter的实现
 * 
 * @param [in] TDevice 底层的设备类型 
 * @param [in] TInterface 测试的接口类型
 * @param [in] TLinker 设备连接器的类型 
 * @param [in] TContainer 测试接口转换器 
 */
template<
    class TDevice, 
    class TInterface = TDevice,
    class TLinker = TestLinker<TDevice>,
    class TContainer = TestContainer<TDevice, TInterface> >
class TestAdapter : 
    public ITest<TInterface>,
    public TextPrinter, 
    public BaseDevAdapterBehavior<TDevice>,
    public InterruptBehavior,
    public RefObject
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
    /// 设备接口转换器 
    TContainer _testContainer;
    /// 测试的接口 
    Ref<TInterface> _testInterface;
    /// 初始化数据 
    inline void _init()
    {
        IsDelay = false;
    }
    /// 输出测试信息  
    void _Print(TextPrinter::TextMode mode, const char* msg)
    {
        string str = Title;
        str += msg;

        TextPrint(mode, str.c_str());
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    TestAdapter() { _init(); }
    virtual ~TestAdapter()
    {
        EndTest();

        ReleaseDevice();
        Release();
    }
    //----------------------------------------------------- 
    /// 连接设备准备测试 
    virtual bool BeginTest(const char* devArg)
    {
        ASSERT_Func(IsValid());
        _Print(TextTips, "设备检测中,请连接设备...");
        if(TLinker().Link(_pDev, devArg, *this))
        {
            _Print(TextNote, "设备已连接");
            return true;
        }
        _Print(TextWarning, "设备未连接");
        return false;
    }
    /// 准备测试数据 
    virtual bool PreTest(const char*)
    {
        ASSERT_Func(IsValid());
        _testInterface = _testContainer.Adapter(_pDev, reinterpret_cast<TInterface*>(NULL), *this);
        if(_testInterface.IsNull())
        {
            _Print(TextWarning, "接口类型转换失败");
            return false;
        }
        return true;
    }
    /// 测试 
    virtual bool OnTest(ITestCase<TInterface>& testCase, const ByteArray& testArg)
    {
        ASSERT_Func(IsValid() && !_testInterface.IsNull());
        bool bTest = testCase.Test(_testInterface, testArg, *this);

        if(bTest && IsDelay)
        {
            _Print(TextPrinter::TextInfo, "操作完成,请确认结果");
            while(IsDelay)
            {
                if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
                {
                    _Print(TextPrinter::TextWarning, "确认操作被取消");
                    break;
                }
                Timer::Wait(DEV_OPERATOR_INTERVAL);
            }
        }

        return bTest;
    }
    /// 结束测试 
    virtual bool EndTest()
    {
        bool bUnLink = true;

        _Print(TextNote, "操作结束,断开设备");
        // 设备已经释放则直接退出 
        if(IsValid())
            bUnLink = TLinker().UnLink(_pDev, *this);

        if(bUnLink)
        {
            _Print(TextNote, "正常退出");
        }
        else
        {
            _Print(TextWarning, "异常退出");
        }

        return bUnLink;
    }
    //----------------------------------------------------- 
    /// 测试成功后是否延时等待用户确认 
    bool IsDelay;
    //----------------------------------------------------- 
    /// 测试显示的标题 
    string Title;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/**
 * @brief 测试器封装 
 * @param [in] TTestAdapter 测试适配器类型 
 * @param [in] TDeviceEx 实际测试的设备类型(这样可以支持子类使用父类的TestLinker和TestAdapter) 
 */
template<class TTestAdapter, class TDeviceEx = typename TTestAdapter::DeviceType, class Alloc = simple_alloc<ITestCase<typename TTestAdapter::InterfaceType>>>
class TestModule : 
    public AutoAdapter<TDeviceEx, TTestAdapter, typename TTestAdapter::DeviceType>,
    public TimeoutBehavior
{
public:
    //----------------------------------------------------- 
    /// 测试适配器类型 
    typedef TTestAdapter TestAdapterType;
    /// 底层的实际设备类型 
    typedef TDeviceEx DeviceType;
    /// 测试接口类型 
    typedef typename TTestAdapter::InterfaceType InterfaceType;
    /// 自动适配器类型 
    typedef AutoAdapter<TDeviceEx, TTestAdapter, typename TTestAdapter::DeviceType> AutoAdapterType;
    /// 测试案例类型 
    typedef ITestCase<typename TTestAdapter::InterfaceType> TestCaseType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 测试案例集合 
    list<TestCaseType*> _testCaseList;
    /// 测试案例参数 
    list<ByteBuilder> _testArgList;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    TestModule(const char* title = "测试") 
    {
        TTestAdapter::Title = _strput(title);
        _waitInterval = DEV_TEST_INTERVAL;
    }
    virtual ~TestModule()
    {
        Clear();
    }
    //----------------------------------------------------- 
    /// 增加一个测试案例 
    template<class TTestCase>
    void Append(const ByteArray& testArg = ByteArray())
    {
        typedef typename Alloc::template rebind<TTestCase>::other AllocOther;

        TTestCase* pObj = AllocOther::allocate();
        AllocOther::construct(pObj);

        _testCaseList.push_back(pObj);
        _testArgList.push_back(testArg);
    }
    /// 删除最后一个测试案例 
    inline void Remove()
    {
        _testCaseList.pop_back();
        _testArgList.pop_back();
    }
    /// 当前测试案例的数目 
    inline size_t Size() const
    {
        return _testCaseList.size();
    }
    /// 清空所有测试案例 
    inline void Clear()
    {
        _testCaseList.clear();
        _testArgList.clear();
    }
    //----------------------------------------------------- 
    /// 测试集合中的所有测试案例
    bool OnTest()
    {
        ByteBuilder msg(32);
        size_t index = 0;
        list<TestCaseType*>::iterator itr;
        list<ByteBuilder>::iterator itrArg;

        for(itr = _testCaseList.begin(), itrArg = _testArgList.begin();
            itr != _testCaseList.end() && itrArg != _testArgList.end();
            ++itr, ++itrArg)
        {
            msg.Clear();
            msg.Format("第<%d>项,共<%d>项...", ++index, Size());
            _Print(TextNote, msg.GetString());

            if(!TTestAdapter::OnTest(*(*itr), *itrArg))
            {
                msg.Format("第<%d>项失败", index);
                _Print(TextWarning, msg.GetString());

                return false;
            }
            msg.Clear();
            msg.Format("第<%d>项成功", index);
            _Print(TextNote, msg.GetString());
        }
        _Print(TextNote, "全部完成");
        return true;
    }
    /// 测试 
    bool Test(const char* devArg = NULL, const char* preArg = NULL)
    {
        _Print(TextPrinter::TextTips, "准备中...");

        bool bIsTest = false;
        Timer timer;
        while(timer.Elapsed() < _waitTimeout)
        {
            if(BeginTest(devArg))
            {
                _Print(TextPrinter::TextNote, "已就绪");
                bIsTest = true;
                break;
            }

            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                _Print(TextPrinter::TextError, "连接失败,操作被取消");
                return false;
            }

            Timer::Wait(_waitInterval);
        }

        if(timer.Elapsed() > _waitTimeout)
        {
            _Print(TextPrinter::TextError, "连接失败,等待超时");
            return false;
        }

        if(!bIsTest)
        {
            _Print(TextPrinter::TextError, "连接失败");
            return false;
        }

        _Print(TextPrinter::TextTips, "初始化中...");
        timer.Restart();

        bIsTest = false;
        while(timer.Elapsed() < _waitTimeout)
        {
            if(PreTest(preArg))
            {
                _Print(TextPrinter::TextNote, "初始化成功");
                bIsTest = true;
                break;
            }

            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                _Print(TextPrinter::TextError, "初始化失败,操作被取消");
                break;
            }

            Timer::Wait(_waitInterval);
        }
        if(timer.Elapsed() > _waitTimeout)
        {
            _Print(TextPrinter::TextError, "初始化失败,等待超时");
        }

        if(!bIsTest)
        {
            _Print(TextPrinter::TextError, "初始化失败");
            EndTest();
            return false;
        }

        _Print(TextPrinter::TextTips, "操作中,请稍等...");
        bIsTest = OnTest();

        EndTest();

        if(bIsTest)
        {
            _Print(TextPrinter::TextResult, "成功");
        }
        else
        {
            _Print(TextPrinter::TextError, "失败");
        }

        return bIsTest;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace test
} // namespace application 
} // namespace zhou_yb
//========================================================= 