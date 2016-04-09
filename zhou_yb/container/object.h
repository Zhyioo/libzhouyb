//========================================================= 
/**@file object.h
* @brief 常用的一些简单对象
*
* @date 2013-09-08   11:55 :02
* @author Zhyioo
* @version 1.0
*/
#pragma once 
//--------------------------------------------------------- 
#include "allocator.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
/// 引用计数器对象 
template<class T>
struct refcount_obj
{
    /// 引用计数 
    size_t refcount;
    /// 数据 
    T obj;

    refcount_obj() : refcount(1), obj() {}
};
//--------------------------------------------------------- 
/**
 * @brief 一个简单的可被多个元素共享的对象(引用计数)
 * 1. 需要对象有无参构造函数
 * 2. shared_obj一定持有一个对象
 * 3. 第一个shared_obj负责创建实例,其他的指针指向
 * 4. 最后一个shared_obj释放对象
 * 5. shared_obj为强引用,只要对象被使用,原始的持有者析构后不
 *    会导致对象失效,只有当所有引用无效后才会释放对象
 */
template<class T, class Alloc = simple_alloc<T> >
class shared_obj
{
public:
    //----------------------------------------------------- 
    typedef T value_type;
    typedef Alloc AllocType;
    typedef typename Alloc::template rebind<refcount_obj<T> >::other AllocOther;
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    refcount_obj<value_type>* _obj;
    /// 创建一个新的对象 
    void _create_obj()
    {
        _obj = AllocOther::allocate();
        Alloc::construct(&(_obj->obj));
        _obj->refcount = 1;
    }
    /// 删除旧的对象(没有其他引用则删除) 
    void _destory_obj()
    {
        --(_obj->refcount);
        if(_obj->refcount < 1)
        {
            Alloc::destroy(&(_obj->obj));
            AllocOther::deallocate(_obj);
            _obj = NULL;
        }
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    shared_obj()
    {
        _create_obj();
    }
    shared_obj(const shared_obj& other)
    {
        _obj = other._obj;
        ++(_obj->refcount);
    }
    virtual ~shared_obj()
    {
        _destory_obj();
    }
    /// 拷贝
    shared_obj& operator =(const value_type& val)
    {
        _obj.obj = val;
        return (*this);
    }
    /// 拷贝 
    shared_obj& operator =(const shared_obj& other)
    {
        // this和other引用的是不同的对象 
        if(_obj != other._obj)
        {
            // 释放旧对象 
            _destory_obj();

            _obj = other._obj;
            ++(_obj->refcount);
        }

        return (*this);
    }
    /// 返回对象的引用计数 
    inline size_t ref_count() const
    {
        return _obj->refcount;
    }
    /// 重置引用的对象 
    value_type& reset()
    {
        _destory_obj();
        _create_obj();

        return obj();
    }
    /// 转换函数 
    operator value_type&()
    {
        return _obj->obj;
    }
    operator const value_type&() const
    {
        return _obj->obj;
    }
    /// 返回对象 
    value_type& obj()
    {
        return _obj->obj;
    }
    const value_type& obj() const
    {
        return _obj->obj;
    }
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief 逻辑判断符号
     */
    bool operator ==(const shared_obj& other)
    {
        return (_obj == other.obj);
    }
    bool operator !=(const shared_obj& other)
    {
        return !(operator ==(other));
    }
    //@
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 可使用Ref<T>进行安全引用的对象   
class RefObject
{
protected:
    /// 引用对象是否还有效 
    shared_obj<bool> _ref;
public:
    RefObject()
    {
        _ref.obj() = true;
    }
    virtual ~RefObject()
    {
        _ref.obj() = false;
    }
    /// 返回对对象的引用(用于Ref<T>类使用,外部不要单独调用)
    inline const shared_obj<bool>& Reference() const
    {
        return _ref;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 弱引用(临时对象的引用)
 *
 * IInterface& => Ref<IInterface>
 */
template<class TInterface>
class WeakRef : public RefObject
{
protected:
    TInterface* _pObj;
public:
    WeakRef(TInterface& obj)
    {
        _pObj = &obj;
    }
    virtual ~WeakRef()
    {
        _ref.obj() = false;
    }

    inline operator TInterface&()
    {
        return (*_pObj);
    }
    inline operator const TInterface&() const
    {
        return (*_pObj);
    }
};
//---------------------------------------------------------  
/**
 * @brief 对对象指定接口的引用(强引用)
 */
template<class TInterface>
class Ref
{
protected:
    //----------------------------------------------------- 
    /// 引用的数据是否有效 
    shared_obj<bool> _ref;
    /// 引用的接口指针(和对象的指针有可能不是同一个) 
    TInterface* _pInterface;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    Ref()
    {
        _pInterface = NULL;
        _ref.obj() = false;
    }
    template<class T>
    Ref(const Ref<T>& ref)
    {
        const TInterface& interfaceObj = (*ref);
        _pInterface = const_cast<TInterface*>(&interfaceObj);
        _ref = ref.Reference();
    }
    template<class TRefObject>
    Ref(const TRefObject& obj)
    {
        const TInterface& interfaceObj = obj;
        _pInterface = const_cast<TInterface*>(&interfaceObj);
        _ref = obj.Reference();
    }
    //-----------------------------------------------------  
    /// 创建引用 
    template<class TRefObject>
    Ref& operator =(const TRefObject& obj)
    {
        const TInterface& interfaceObj = obj;
        _pInterface = const_cast<TInterface*>(&interfaceObj);
        _ref = obj.Reference();

        return (*this);
    }
    /// 解除引用
    inline void Free()
    {
        _pInterface = NULL;
        _ref.reset() = false;
    }
    /// 返回引用的对象是否有效
    inline operator bool() const
    {
        return !IsNull();
    }
    /// 返回引用的对象是否为NULL 
    inline bool IsNull() const
    {
        return !(_ref.obj());
    }
    /// 返回对对象的引用(用于Ref<T>类使用,外部不要单独调用)
    inline const shared_obj<bool>& Reference() const
    {
        return _ref;
    }
    /// 返回引用的是否是同一个对象 
    inline bool operator ==(const TInterface& obj) const
    {
        return _pInterface == &obj;
    }
    /// 返回引用的是否是不同的对象 
    inline bool operator !=(const TInterface& obj) const
    {
        return !operator ==(obj);
    }
    /// 返回引用 
    inline operator TInterface&()
    {
        return (*_pInterface);
    }
    /// 返回引用 
    inline operator const TInterface&() const
    {
        return (*_pInterface);
    }
    /// 取成员操作 
    inline TInterface& operator *()
    {
        return (*_pInterface);
    }
    /// 取成员操作 
    inline TInterface* operator ->()
    {
        return &(operator *());
    }
    /// 取成员操作 
    inline const TInterface& operator *() const
    {
        return (*_pInterface);
    }
    /// 取成员操作 
    inline const TInterface* operator ->() const
    {
        return &(operator *());
    }
    //-----------------------------------------------------  
};
//--------------------------------------------------------- 
} // namespace container
} // namespace zhou_yb
//========================================================= 
