//========================================================= 
/**@file AdapterDevice.h 
 * @brief 自动嵌套的设备类型定义
 * 
 * @date 2013-08-28   20:07:38 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "AutoAdapter.h"

#include "../../include/Base.h"
#include "../../include/BaseDevice.h"
#include "../../include/Device.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace special {
//--------------------------------------------------------- 
/// 日志行为追加器(为给在AutoAdapter中嵌套不支持日志的类加上虚的功能,方便下面函数调用) 
template<class TBase>
struct EmptyLoggerBehaviorAppender : public TBase, public LoggerBehavior
{

};
//---------------------------------------------------------
/**
 * @brief 嵌套选择日志的辅助类
 *
 * 并不是所有的设备或者适配器都支持日志,所以AutoAdapter只支持
 * 必须的SelectDevice接口, 对于支持日志的设备可以使用该类嵌套
 * 加载日志
 */
template<class TAutoAdapter, size_t lv = TAutoAdapter::level>
struct AutoAdapterHelper
{
    /// 加载日志
    static void SelectLogger(TAutoAdapter& adapter, const LoggerAdapter& log)
    {
        adapter.SelectLogger(log);
        AutoAdapterHelper<typename TAutoAdapter::BaseType, lv - 1>::SelectLogger(adapter.Base(), log);
    }
    /// 卸载日志
    static void ReleaseLogger(TAutoAdapter& adapter, const LoggerAdapter& log)
    {
        adapter.ReleaseLogger();
        AutoAdapterHelper<typename TAutoAdapter::BaseType, lv - 1>::ReleaseLogger(adapter.Base(), log);
    }
    /// 重新建立SelectDevice关系 
    static void ReLink(TAutoAdapter& adapter)
    {
        adapter.Adapter().SelectDevice(adapter.Base());
        AutoAdapterHelper<typename TAutoAdapter::BaseType, lv - 1>::ReLink(adapter.Base());
    }
    /**
     * @brief 设置设备底层的共享关系 将adapter引用到ownerAdapter中 
     * @param [out] adapter 需要设置的目的适配器 
     * @param [in] ownerAdapter 底层设备的源适配器 
     */ 
    template<class TBase>
    static void Shared(TAutoAdapter& adapter, TBase& ownerAdapter)
    {
        TBase& baseAdapter = adapter;
        baseAdapter = ownerAdapter;
        AutoAdapterHelper<TAutoAdapter, lv>::ReLink(adapter);
    }
};
/// 对模板lv=0的特化,用于终结模板元递归
template<class TAutoAdapter>
struct AutoAdapterHelper<TAutoAdapter, static_cast<size_t>(0)>
{
    /// 加载日志
    static void SelectLogger(TAutoAdapter& adapter, const LoggerAdapter& log)
    {
        adapter.SelectLogger(log);
    }
    /// 卸载日志
    static void ReleaseLogger(TAutoAdapter& adapter, const LoggerAdapter&)
    {
        adapter.ReleaseLogger();
    }
    /// 重新建立SelectDevice关系 
    inline static void ReLink(TAutoAdapter&)
    {
        
    }
};
//--------------------------------------------------------- 
} // namespace special 
} // namespace extension 
} // namespace zhou_yb
//========================================================= 
