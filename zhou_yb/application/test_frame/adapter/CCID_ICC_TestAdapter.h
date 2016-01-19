//========================================================= 
/**@file CCID_ICC_TestAdapter.h 
 * @brief CCID IC卡测试适配器 
 *
 * 提供模板将CCID设备转换为指定测试接口(可以是存储卡的特定接口) 
 * 
 * @date 2015-01-24   19:30:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"
#include "ICCardTestAdapter.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief CCID读卡器测试程序
 * @param [in] TInterface 测试的接口类型
 * @param [in] TContainer 接口类型转换器
 */
template<class TInterface,
    class TContainer = TestContainer< IICCardDevice, TInterface > >
class CCID_ICC_TestAdapter : 
    public ICCardTestAdapter <
        // 设备类型 
        CCID_Device,
        // 测试的接口类型 
        TInterface,
        // 设备连接器,由于BeginTest已经处理设备连接状态,所以直接使用空的连接器,而不用 CCID_DeviceLinker  
        TestLinker< CCID_Device >,
        // IICCardDevice接口容器,CCID_Device原始类型即支持,使用默认
        TestContainer< CCID_Device, IICCardDevice >,
        // 接口转换器 
        TContainer>
{
public:
    //----------------------------------------------------- 
    /// 底层设备类型 
    typedef CCID_Device DeviceType;
    /// 测试的接口类型 
    typedef TInterface InterfaceType;
    /// 测试接口转换类型 
    typedef TContainer ContainerType;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 上次的卡片状态 
    DWORD _eventState;
    //----------------------------------------------------- 
    /// 获取状态变化 
    bool _SCardStatusChanged(DWORD from, DWORD to)
    {
        DWORD currentState = 0;
        DWORD lastEventState = 0;

        bool bRet = false;

        lastEventState = _eventState;
        // 已经测试过卡片,等待卡片拿走进行下一测试  
        if(_pDev->SCardReaderState(currentState, _eventState, _sPowerOnArg.c_str()))
        {
            if((lastEventState & from) && (_eventState & to))
            {
                if(_eventState & SCARD_STATE_CHANGED)
                {
                    bRet = true;
                }
            }
        }

        return bRet;
    }
    //----------------------------------------------------- 
    /// 测试前等待卡片进入事件 
    virtual bool _IsPresent()
    {
        return _SCardStatusChanged(SCARD_STATE_EMPTY, SCARD_STATE_PRESENT);
    }
    /// 测试后等待卡片移走事件 
    virtual bool _IsRemoved()
    {
        _eventState = SCARD_STATE_EMPTY;
        // 如果卡片被移走,或者设备直接被移除则直接返回 
        if(_SCardStatusChanged(SCARD_STATE_PRESENT, SCARD_STATE_EMPTY) || _eventState & SCARD_STATE_EMPTY)
            return true;
        return false;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    CCID_ICC_TestAdapter() : ICCardTestAdapter() { _eventState = SCARD_STATE_EMPTY; }
    //----------------------------------------------------- 
    /// 查找完成的读卡器名称 
    virtual bool BeginTest(const char* sArg)
    {
        ASSERT_Func(IsValid());
        TextPrint(TextTips, "设备检测中,请连接读卡器...");
        _sPowerOnArg = "";

        list<string> devlist;
        list<string>::iterator itr;

        _pDev->EnumDevice(devlist);

        for(itr = devlist.begin(); itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), sArg, true))
            {
                _sPowerOnArg = *itr;
                break;
            }
        }

        if(_sPowerOnArg.length() < 1)
            return false;

        return ICCardTestAdapter::BeginTest(sArg);
    }
    /// 结束测试 
    virtual bool EndTest()
    {
        TextPrint(TextNote, "断开设备,结束测试");
        if(IsValid())
            _pDev->PowerOff();
        
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 过滤设备虚拟的3B00卡片
template<class TInterface, class TContainer = TestContainer< IICCardDevice, TInterface > >
class CCID_ICC_3B00_TestAdapter : public CCID_ICC_TestAdapter< TInterface, TContainer >
{
public:
    //----------------------------------------------------- 
    /// 上电时过滤掉返回3B00的设备
    virtual bool BeginTest(const char* sArg)
    {
        bool bTest = CCID_ICC_TestAdapter::BeginTest(sArg);
        if(bTest)
        {
            const byte EMPTY_ATR[] = { 0x3B, 0x00 };
            ByteArray emptyAtr(EMPTY_ATR, SizeOfArray(EMPTY_ATR));

            ByteBuilder atr(8);
            CCID_Device dev;

            // 识别出ATR为虚拟的卡片则返回连接失败 
            if(dev.PowerOn(_sPowerOnArg.c_str(), &atr) && atr.IsEqual(emptyAtr))
                return false;
        }
        return bTest;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 