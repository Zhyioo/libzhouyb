//========================================================= 
/**@file AutoAdapter.h 
 * @brief 自动进行设备适配的辅助类
 * (单一的适配,不适合多个处理逻辑共用同一个设备的情况) 
 * 
 * @date 2013-08-25   20:26:06 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace special {
//---------------------------------------------------------
/**
 * 支持嵌套类型自动转换的转换器(模板元编程)
 *
 * lv 可以使用TAutoAdapter::level获取,作为单独的参数提供以便于手动控制层数
 */
template<class TBase, class TAutoAdapter, size_t lv = TAutoAdapter::level>
struct AutoBaseConverter
{
    /// 底层被适配的类型
    typedef typename TAutoAdapter::BaseType BaseType;
    static TBase& ConvertFrom(TAutoAdapter& obj)
    {
        return AutoBaseConverter<TBase, BaseType, lv - 1>::ConvertFrom(obj.Base());
    }
};
/// 对模板lv=0的特化,用于终结模板元递归
template<class TBase, class TAutoAdapter>
struct AutoBaseConverter<TBase, TAutoAdapter, static_cast<size_t>(0)>
{
    static TBase& ConvertFrom(TAutoAdapter& obj)
    {
        return obj;
    }
};
//---------------------------------------------------------
/**
 * @brief 自动适配上底层设备的容器适配器
 *
 * @param [in] TBase 底层的设备
 * @param [in] TAdapter适配器 TAdapter::SelectDevice(TBase) 的模式 
 * @param [in] TInterface TAdapter::SelectDevice的类型,默认为TBase 
 * @param [in] lv level表示AutoAdapter所适配的类被嵌套的层数(便于嵌套类型自动转换)
 * AutoAdapter<> =1,AutoAdapter<AutoAdapter<TBase,TAdapter>, TAdapter> =2
 */ 
template<class TBase, class TAdapter, class TInterface = TBase, size_t lv = 1>
class AutoAdapter : public TAdapter
{
protected:
    TBase _baseDev;

    /// 禁用以拷贝构造函数形式的转换  
    operator TBase() { return _baseDev; }
public:
    typedef TBase      BaseType;
    typedef TAdapter   AdapterType;
    typedef AutoAdapter<TBase, TAdapter, TInterface, lv> this_type;

    enum{ level = lv };

    AutoAdapter() : _baseDev(), TAdapter()
    {
        TInterface& interfaceObj = _baseDev;
        TAdapter::SelectDevice(interfaceObj);
    }
    /// 转换 
    operator TBase&()
    {
        return _baseDev;
    }
    /// 嵌套的AutoAdapter Base类型转换
    template<class T>
    operator T&()
    {
        return AutoBaseConverter<T, this_type, level>::ConvertFrom(*this);
    }
    /// 返回底层适配对象 
    inline TBase& Base()
    {
        return (_baseDev);
    }
    /// 返回顶层操作对象 
    inline TAdapter& Adapter()
    {
        return (*this);
    }
};
//--------------------------------------------------------- 
} // namespace special 
} // namespace extension 
} // namespace zhou_yb
//========================================================= 
