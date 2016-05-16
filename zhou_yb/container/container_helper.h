//========================================================= 
/**@file container_helper.h 
 * @brief 容器类型辅助函数,为容器提供一些简便操作
 * 
 * @date 2012-10-17   15:46:43 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CONTAINER_HELPER_H_
#define _LIBZHOUYB_CONTAINER_HELPER_H_
//--------------------------------------------------------- 
#include "list.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
/// 迭代辅助操作名称 
#define IterativeName(func) func##_helper
/**
 * @brief 迭代操作辅助器
 */
#define IterativeHelper(func) \
template<class T> \
class IterativeName(func) \
{ \
protected: \
    T* _pContainer; \
public: \
    IterativeName(func) (T& container) : _pContainer(&container) { } \
    IterativeName(func) & operator ,(const typename T::value_type& val) \
    { \
        _pContainer->func(val); \
        return (*this); \
    } \
    static IterativeName(func) <T> func(T& container) \
    { \
        return IterativeName(func)<T>(container); \
    } \
}
//--------------------------------------------------------- 
/* 预定义的操作器(示例) */
/// 自动迭代向后追加 
IterativeHelper(push_back);
/// 自动迭代向前追加 
IterativeHelper(push_front);
//--------------------------------------------------------- 
/// 选择器
template<class T>
class selecter
{
public:
    typedef T value_type;
protected:
    list<T> _linkList;
    typename list<T>::iterator _find(const T& val)
    {
        typename list<T>::iterator itr;
        for(itr = _linkList.begin();itr != _linkList.end(); ++itr)
        {
            if((*itr) == val)
                return itr;
        }
        return _linkList.end();
    }
public:
    bool select(const T& val)
    {
        typename list<T>::iterator itr = _find(val);
        if(itr == _linkList.end())
        {
            _linkList.push_back(val);
            return true;
        }
        return false;
    }
    void release(const T& val)
    {
        typename list<T>::iterator itr = _find(val);
        if(itr != _linkList.end())
        {
            _linkList.erase(val);
        }
    }
    inline void release()
    {
        _linkList.clear();
    }
};
/// 选择器辅助操作
IterativeHelper(select);
//--------------------------------------------------------- 
/**
 * @brief 简单的封装序列容器的一些常用简洁操作(可在需要时增加)
 *
 * 需要底层的容器支持 T::value_type
 * 
 */ 
template<class T>
class container_helper
{
protected:
    container_helper() {}
public:
    /// 取索引元素 iterator
    static typename T::iterator index_of(T& container, size_t index)
    {
        typename T::iterator itr = container.begin();
        while(itr != container.end() && index-- > 0)
            ++itr;
        return itr;
    }
    /// 获取元素索引
    static size_t position(T& container, const typename T::value_type& val)
    {
        typename T::iterator itr = container.begin();
        size_t index = 0;
        for(itr = container.begin();itr != container.end(); ++itr)
        {
            if((*itr) == val)
                return index;
            ++index;
        }
        return SIZE_EOF;
    }
    /// 重复追加操作 
    static void push_back(T& container, const T& val, size_t n)
    {
        for(size_t i = 0;i < n; ++i)
            container.push_back(val);
    }
    /// 重复追加操作 
    static void push_front(T& container, const T& val, size_t n)
    {
        for(size_t i = 0;i < n; ++i)
            container.push_front(val);
    }
    /// 重复删除末尾操作,返回实际删除的操作  
    static size_t pop_back(T& container, size_t n)
    {
        size_t count = 0;

        while(container.begin() != container.end() && n > 0)
        {
            container.pop_back();
            ++count;
        }

        return count;
    }
    /// 重复删除第一个元素操作,返回实际删除的操作  
    static size_t pop_front(T& container, size_t n)
    {
        size_t count = 0;

        while(container.begin() != container.end() && n > 0)
        {
            container.pop_front();
            ++count;
        }

        return count;
    }
    /// 删除当前迭代器,返回上一个位置 
    static typename T::iterator erase(T& container, typename T::iterator itr)
    {
        typename T::iterator last = itr;
        --last;
        container.erase(itr);
        return last;
    }
    /// 删除指定索引位置上的数据
    static bool erase_at(T& container, size_t index)
    {
        typename T::iterator itr = index_of(container, index);
        if(itr == container.end())
            return false;
        container.erase(itr);
        return true;
    }
    /// 删除结点值为_value的元素,默认只删除找到的第一个结点
    static size_t remove(T& container, const typename T::value_type& _value, bool _all = false)
    {
        typename T::iterator tmp;
        size_t _count = 0;
        for(typename T::iterator i = container.begin();i != container.end();)
        {
            if(*i == _value)
            {
                tmp = i;
                ++i;
                ++_count;
                container.erase(tmp);

                /// 只删除第一个找到的结点
                if(_all == false)
                    break;
                continue;
            }
            ++i;
        }
        return _count;
    }
    /// 删除满足条件_pFunc的元素,默认只删除找到的第一个元素
    template<class ObjFunc>
    static size_t remove_if(T& container, ObjFunc _pFunc, bool _all = false)
    {
        typename T::iterator tmp;
        size_t _count = 0;
        for(typename T::iterator i = container.begin();i != container.end();)
        {
            if(_pFunc(*i))
            {
                tmp = i;
                ++i;
                ++_count;
                container.erase(tmp);

                /// 只删除第一个找到的结点
                if(_all == false)
                    break;
                continue;
            }
            ++i;
        }
        return _count;
    }
    /// 返回指定值的元素个数  
    static size_t count(T& container, const typename T::value_type& _value)
    {
        size_t _count = 0;
        for(typename T::iterator i = container.begin();i != container.end(); ++i)
        {
            if(*i == _value)
                ++_count;
        }
        return _count;
    }
    /// 统计满足条件_pFunc的元素 
    template<class ObjFunc>
    static size_t count_if(T& container, ObjFunc _pFunc)
    {
        size_t _count = 0;
        for(typename T::iterator i = container.begin();i != container.end(); ++i)
        {
            if(_pFunc(*i))
                ++_count;
        }
        return _count;
    }
    /// 返回是否包含指定值的元素 
    static bool contains(T& container, const typename T::value_type& _value)
    {
        for(typename T::iterator i = container.begin();i != container.end(); ++i)
        {
            if(*i == _value)
                return true;
        }
        return false;
    }
    /// 返回满足条件_pFunc的元素 
    template<class ObjFunc>
    static bool contains_if(T& container, ObjFunc _pFunc)
    {
        for(typename T::iterator i = container.begin();i != container.end(); ++i)
        {
            if(_pFunc(*i))
                return true;
        }
        return false;
    }
    /// 查找值为_value的元素，并返回
    static typename T::iterator find(T& container, const typename T::value_type& _value)
    {
        for(typename T::iterator i = container.begin();i != container.end();++i)
            if(*i == _value)
                return i;
        return container.end();
    }
    /// 查找匹配判断条件_pFunc的元素,并返回 
    template<class ObjFunc>
    static typename T::iterator find_if(T& container, ObjFunc _pFunc)
    {
        for(typename T::iterator i = container.begin();i != container.end();)
        {
            if(_pFunc(*i))
                return i;
        }
        return container.end();
    }
};
//--------------------------------------------------------- 
/// 链表容器的定义 
template<class T>
class list_helper : public container_helper<list<T> >
{
protected:
    list_helper() {}
public:
    /// 将T2中和T1相同的数据剪切出去  
    static void split(list<T>& l1, const list<T>& l2)
    {
        for(typename list<T>::const_iterator itr = l2.begin();itr != l2.end(); ++itr)
        {
            container_helper<list<T> >::remove(l1, *itr);
        }
    }
    /// 获取两条链表中相同的部分 
    static size_t intersect(const list<T>& l1, const list<T>& l2, list<T>& dst)
    {
        typename list<T>::const_iterator itr1;
        typename list<T>::const_iterator itr2;
        size_t count = 0;

        for(itr1 = l1.begin();itr1 != l1.end(); ++itr1)
        {
            for(itr2 = l2.begin();itr2 != l2.end(); ++itr2)
            {
                if((*itr1) == (*itr2))
                {
                    dst.push_back(*itr1);
                    ++count;
                }
            }
        }
        return count;
    }
    /// 往链表指定结点后插入数据 
    static typename list<T>::iterator insert_back(list<T>& l, typename list<T>::iterator itr, const T& val)
    {
        typename list<T>::iterator tmpItr = itr;
        ++tmpItr;
        return l.insert(tmpItr, val);
    }
};
//---------------------------------------------------------
} // namespace container
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CONTAINER_HELPER_H_
//=========================================================
