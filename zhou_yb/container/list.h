//========================================================= 
/**@file list.h 
 * @brief 简单的双链表类
 * 
 * VC6不支持类函数中返回list<T>::iterator,自己定义一个支持的版本 
 * 内容参照SGI版本 list 
 *
 * @date 2012-05-26   22:19:11 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LIST_H_
#define _LIBZHOUYB_LIST_H_
//--------------------------------------------------------- 
#include "allocator.h"
//--------------------------------------------------------- 
// 使用BOOST+STL的方式
#ifdef NO_INCLUDE_LIST_SOURCE
#include <list>
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
/// 类型const属性萃取器(VC6不支持模板偏特化无法使用template<bool isConst,class _Const_Class,class _Class>来萃取) 
template <class T>
struct _Nonconst_traits;

template <class T>
struct _Const_traits 
{
  typedef T value_type;
  typedef const T&  reference;
  typedef const T*  pointer;
  typedef _Const_traits<T> _ConstTraits;
  typedef _Nonconst_traits<T> _NonConstTraits;
};

template <class T>
struct _Nonconst_traits 
{
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef _Const_traits<T> _ConstTraits;
  typedef _Nonconst_traits<T> _NonConstTraits;
};
//--------------------------------------------------------- 
} }
using stlport::list;
//--------------------------------------------------------- 
#else
//--------------------------------------------------------- 
#include <iterator>
using std::bidirectional_iterator_tag;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//---------------------------------------------------------
/// 结点基础结构 
struct list_node_base
{
    typedef list_node_base* base_ptr;
    /// 前驱 
    base_ptr next;
    /// 后继 
    base_ptr prev;
};
//-----------------------------------------------------
/// 实际的数据结点 
template<class T>
struct list_node : public list_node_base
{
    /// 结点数据项
    T data;
};
//-----------------------------------------------------
/// 迭代器基础结构 
struct list_iterator_base 
{
    list_node_base* node;
    list_iterator_base() : node(NULL){}
    explicit list_iterator_base(list_node_base* x) : node(x){}

    /// 前进一个结点
    inline void ahead() 
    {
        node = node->next;
    }
    /// 后退一个结点
    inline void back()
    {
        node = node->prev;
    }

    bool operator==(const list_iterator_base& x)
    {
        return node == x.node;
    }
    bool operator!=(const list_iterator_base& x)
    {
        return node != x.node;
    }
};
//-----------------------------------------------------
/// 类型const属性萃取器(VC6不支持模板偏特化无法使用template<bool isConst,class _Const_Class,class _Class>来萃取) 
template <class T>
struct _Nonconst_traits;

template <class T>
struct _Const_traits 
{
  typedef T value_type;
  typedef const T&  reference;
  typedef const T*  pointer;
  typedef _Const_traits<T> _ConstTraits;
  typedef _Nonconst_traits<T> _NonConstTraits;
};

template <class T>
struct _Nonconst_traits 
{
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef _Const_traits<T> _ConstTraits;
  typedef _Nonconst_traits<T> _NonConstTraits;
};
//---------------------------------------------------------
/// 链表迭代器
template<class T, class Traits>
struct list_iterator : public list_iterator_base
{
    // 模板类型简写定义 
    typedef list_node<T>* node_ptr;
    typedef typename Traits::reference reference;
    typedef typename Traits::pointer pointer;
    typedef size_t difference_type;
    typedef bidirectional_iterator_tag iterator_category;

    typedef list_iterator<T, Traits> self;
    typedef list_iterator<T, typename Traits::_NonConstTraits> iterator;
    typedef list_iterator<T, typename Traits::_ConstTraits> const_iterator;
    typedef T value_type;

    list_iterator() : list_iterator_base(){}
    list_iterator(node_ptr x) : list_iterator_base(x){}
    list_iterator(const iterator& x) : list_iterator_base(x.node){}

    // *this
    reference operator*() const
    {
        return (reinterpret_cast<node_ptr>(node))->data;
    }
    /// this->data
    pointer operator->() const
    {
        return &(operator*());
    }
    /// ++(*this)
    self& operator++()
    {
        ahead();
        return *this;
    }
    /// (*this)++
    self operator++(int)
    {
        self temp = *this;
        ahead();
        return temp;
    }
    /// --(*this)
    self& operator--()
    {
        back();
        return *this;
    }
    /// (*this)--
    self operator--(int)
    {
        self temp = *this;
        back();
        return temp;
    }
    /// 扩展:将指定迭代器结点向前移动distance的距离
    self operator +(size_t distance)
    {
        while(distance-- > 0)
            ahead();
        return *this;
    }
    /// 扩展:将指定迭代器结点向前移动distance的距离
    self& operator +=(size_t distance)
    {
        return operator+(distance);
    }
    /// 扩展:将指定迭代器结点向后移动distance的距离
    self operator -(size_t distance)
    {
        while(distance-- >0)
            back();
        return *this;
    }
    /// 扩展:将指定迭代器结点向后移动distance的距离
    self& operator -=(size_t distance)
    {
        return operator-(distance);
    }
};
//---------------------------------------------------------
/// 链表反向迭代器
template<class T, class Traits>
struct list_reverse_iterator : public list_iterator_base
{
    // 类型定义 
    typedef list_node<T>* node_ptr;
    typedef typename Traits::reference reference;
    typedef typename Traits::pointer pointer;
    typedef list_reverse_iterator<T, Traits> self;
    typedef list_reverse_iterator<T, typename Traits::_NonConstTraits> iterator;
    typedef list_reverse_iterator<T, typename Traits::_ConstTraits> const_iterator;
    typedef T value_type;

    list_reverse_iterator() : list_iterator_base(){}
    explicit list_reverse_iterator(node_ptr x) : list_iterator_base(x){}
    list_reverse_iterator(const iterator& x) : list_iterator_base(x.node){}

    /// *this
    reference operator*() const
    {
        return (reinterpret_cast<node_ptr>(node))->data;
    }
    /// this->data
    pointer operator->() const
    {
        return &(operator*());
    }
    /// ++(*this)
    self& operator++()
    {
        back();
        return *this;
    }
    /// (*this)++
    self operator++(int)
    {
        self temp = *this;
        back();
        return temp;
    }
    /// --(*this)
    self& operator--()
    {
        ahead();
        return *this;
    }
    /// (*this)--
    self operator--(int)
    {
        self temp = *this;
        ahead();
        return temp;
    }
    /// 扩展:将指定迭代器结点向前移动distance的距离
    self operator +(size_t distance)
    {
        while(distance-- > 0)
            back();
        return *this;
    }
    /// 扩展:将指定迭代器结点向前移动distance的距离
    self& operator +=(size_t distance)
    {
        return operator+(distance);
    }
    /// 扩展:将指定迭代器结点向后移动distance的距离
    self operator -(size_t distance)
    {
        while(distance-- > static_cast<size_t>(0))
            ahead();
        return *this;
    }
    /// 扩展:将指定迭代器结点向后移动distance的距离
    self& operator -=(size_t distance)
    {
        return operator-(distance);
    }
};
//---------------------------------------------------------
/// 双链表(需要T支持无参数构造函数和拷贝构造函数) 
template<class T, class Alloc = simple_alloc<T> >
class list
{
public:
    // 类型接口定义 
    typedef list_node_base* base_ptr;
    typedef list_node<T> node;
    typedef list_node<T>* node_ptr;
    typedef list_iterator<T, _Nonconst_traits<T> > iterator;
    typedef list_iterator<T, _Const_traits<T> > const_iterator;
    typedef list_reverse_iterator<T, _Nonconst_traits<T> > reverse_iterator;
    typedef list_reverse_iterator<T, _Const_traits<T> > const_reverse_iterator;
    typedef typename list_iterator<T, _Nonconst_traits<T> >::reference reference;
    typedef typename list_iterator<T, _Const_traits<T> >::reference const_reference;
    typedef T value_type;
    typedef Alloc allocator_type;
    typedef typename Alloc::template rebind<node>::other AllocOther;
protected:
    /// 头结点
    list_node_base head;
    /// 结点个数
    size_t count;
    /// 产生一个值为新的结点,并返回起始地址
    node_ptr creat_memory()
    {
        node_ptr temp = AllocOther::allocate();
        Alloc::construct(&(temp->data));

        return temp;
    }
    /// 产生一个值为_value的结点,并返回起始地址
    node_ptr creat_memory(const value_type& _value)
    {
        node_ptr temp = AllocOther::allocate();
        Alloc::construct(&(temp->data), _value);

        return temp;
    }
    /// 初始化head结点
    inline void init() 
    {
        head.next = &head;
        head.prev = &head;
        count = 0;
    }
    /// 从链表中剪切出_node结点,并返回结点的指针
    inline node_ptr _snip(base_ptr _node)
    {
        _node->prev->next = _node->next;
        _node->next->prev = _node->prev;
        --count;

        return reinterpret_cast<node_ptr>(_node);
    }
    /// 删除_node结点
    inline void _erase(base_ptr _node)
    {
        node_ptr p = _snip(_node);
        Alloc::destroy(&(p->data));
        AllocOther::deallocate(p);
    }
    /// 将v结点连接到x结点后
    inline void _splice(base_ptr x, base_ptr v)
    {
        x->next = v;
        v->prev = x;
    }
    /// 在x结点后新建一个结点
    inline node_ptr _insert_back(const base_ptr x)
    {
        node_ptr temp = creat_memory();
        _insert_node_back(x, reinterpret_cast<base_ptr>(temp));
        return temp;
    }
    /// 在x结点后新建一个值为_value的结点
    inline node_ptr _insert_back(const base_ptr x, const value_type& _value)
    {
        node_ptr temp = creat_memory(_value);
        _insert_node_back(x,reinterpret_cast<base_ptr>(temp));
        return temp;
    }
    /// 在x结点前新建一个结点
    inline node_ptr _insert_front(const base_ptr x)
    {
        node_ptr temp = creat_memory();
        _insert_node_front(x, reinterpret_cast<base_ptr>(temp));
        return temp;
    }
    /// 在x结点前新建一个值为_value的结点
    inline node_ptr _insert_front(const base_ptr x, const value_type& _value)
    {
        node_ptr temp = creat_memory(_value);
        _insert_node_front(x, reinterpret_cast<base_ptr>(temp));
        return temp;
    }
    /// 在x结点后插入已有的_value结点
    inline node_ptr _insert_node_back(base_ptr x, base_ptr _value)
    {
        /*x->next->prev = _value;
        _value->next = x->next;
        x->next = _value;
        _value->prev = x;*/
        _splice(_value,x->next);
        _splice(x,_value);
        ++count;
        return reinterpret_cast<node_ptr>(_value);
    }
    /// 在x结点前插入已有的_value结点
    inline node_ptr _insert_node_front(base_ptr x, base_ptr _value)
    {
        /*x->prev->next = _value;
        _value->prev = x->prev;
        _value->next = x;
        x->prev = _value;*/
        /*_splice(x->prev,_value);
        _splice(_value,x);
        ++count;
        return (node_ptr)_value;*/
        return _insert_node_back(x->prev,_value);
    }
    /// 计算[_start,_end)区间的距离(结点数)
    int _distance(iterator _start, iterator _end) const
    {
        int length = 0;
        while(_start++ != _end)
            ++length;
        return length;
    }
public:
    list(){ init(); }
    list(const list<T>& l)
    {
        init();
        operator +=(l);
    }
    list(const value_type& _value, int count = 1)
    {
        init();
        while(count-- > 0)
            push_back(_value);
    }
    list(const value_type* p_value, int length = 1)
    {
        init();
        while(length-- > 0)
            push_front(p_value[length]);
    }
    virtual ~list()
    {
        clear();
    }

    /// 链表的复制操作l1 = l2;
    void operator =(const list<T>& _list)
    {
        clear();
        operator+=(_list);
    }
    /// 将_list中的元素拷贝到链表中
    void operator +=(const list<T>& _list)
    {
        for(const_iterator i = _list.begin();i != _list.end();++i)
            push_back(*i);
    }
    /// 返回链表是否为空
    inline bool empty() const 
    {
        return head.next == &head;
    }
    /// 返回链表中元素个数(链表长度)
    inline size_t size() const 
    {
        return count;
    }
    /// 返回链表的起始结点
    iterator begin()
    {
        return reinterpret_cast<node_ptr>(head.next);
    }
    /// 返回链表的起始结点
    const_iterator begin() const
    {
        return ctype_cast(node_ptr)(head.next);
    }
    /// 返回链表的反向起点
    reverse_iterator rbegin()
    {
        return reinterpret_cast<node_ptr>(head.prev);
    }
    /// 返回链表的反向起点
    const_reverse_iterator rbegin() const
    {
        return ctype_cast(node_ptr)(head.prev);
    }
    /// 返回链表的末尾元素
    iterator end()
    {
        return reinterpret_cast<node_ptr>(&head);
    }
    /// 返回链表的末尾元素
    const_iterator end() const
    {
        return ctype_cast(node_ptr)(&head);
    }
    /// 返回链表的反向末点
    reverse_iterator rend()
    {
        return reinterpret_cast<node_ptr>(&head);
    }
    /// 返回链表的反向末点
    const_reverse_iterator rend() const
    {
        return ctype_cast(node_ptr)(&head);
    }
    /// 返回链表起始元素
    reference front()
    {
        return (*begin());
    }
    /// 返回链表起始元素
    const_reference front() const
    {
        return (*begin());
    }
    /// 返回链表末尾元素
    reference back()
    {
        return (*(end()-1));
    }
    /// 返回链表末尾元素
    const_reference back() const
    {
        return (*(end()-1));
    }
    /// 在链表末尾追加元素
    iterator push_back(const value_type& _value)
    {
        return _insert_front(&head,_value);
    }
    /// 在链表末尾追加元素
    iterator push_back()
    {
        return _insert_front(&head);
    }
    /// 在链表末尾追加元素
    iterator push_back(const iterator _x)
    {
        return _insert_front(&head,*_x);
    }
    /// 在链表开头添加元素
    iterator push_front(const value_type& _value)
    {
        return _insert_back(&head,_value);
    }
    /// 在链表开头添加元素
    iterator push_front()
    {
        return _insert_back(&head);
    }
    /// 在链表开头添加元素
    iterator push_front(const iterator _x)
    {
        return _insert_back(&head,*_x);
    }
    /// 在指定结点前插入值为value的新结点
    iterator insert(const iterator _x, const value_type& _value)
    {
        return _insert_front(_x.node,_value);
    }
    /// 在指定的结点前插入值为value的新结点 
    iterator insert_front(const iterator _x, const value_type& _value)
    {
        return _insert_front(_x.node,_value);
    }
    /// 在指定结点后插入值为value的新结点
    iterator insert_back(const iterator _x, const value_type& _value)
    {
        return _insert_back(_x.node,_value);
    }
    /// 删除起始结点
    void pop_front()
    {
        _erase(head.next);
    }
    /// 删除尾结点
    void pop_back()
    {
        _erase(head.prev);
    }
    /// 删除所指元素 
    iterator erase(iterator x)
    {
        iterator next = x;
        ++next;
        _erase(x.node);

        return next;
    }
    /// 删除[_begin,_end)区间上的元素
    void erase(iterator _begin, iterator _end)
    {
        while(_begin != _end)
            _erase(_begin.node);
    }
    /// 交换迭代器_x和_value的值
    void swap(iterator _x, iterator _value)
    {
        /*//交换值
        value_type temp = *_value;
        *_value = *_x;
        *_x = temp;*/
        //交换指针位置
        base_ptr temp = _x.node->prev;//保存x的上一个结点
        _insert_node_back(_value.node,_snip(_x.node));//将x剪切到v后
        _insert_node_back(temp,_snip(_value.node));//将v剪切到原来的x的位置上
    }
    /// 交换2条链表
    void swap(list<T>& _list)
    {
        //直接交换头结点极其长度即可
        list_node_base temp = head;
        int size = count;
        //将_list的头结点换成head
        _splice(&head,_list.head.next);
        _splice(_list.head.prev,&head);
        count = _list.count;
        //将this的头结点换成_list的head
        _splice(&_list.head,temp.next);
        _splice(temp.prev,&_list.head);
        _list.count = size;
    }
    /**
     * @brief 将链表中[_start,_end)之间值为_x的元素替换为_value,
     * 并返回找到的第一个元素位置,无则返回end(),
     * _all:是否替换区间内的所有元素
     */ 
    iterator swap_value(const value_type& _x, const value_type& _value,
        iterator _start, iterator _end, bool _all = true)
    {
        iterator temp = end();
        for(iterator i = _start;i != _end;++i)
        {
            if(*i == _x)
            {
                *i = _value;
                /// 设置找到的第一个元素位置
                if(temp == end())
                    temp = i;
                /// 只替换第一个找到的元素
                if(_all == false)
                    break;
            }
        }
        return temp;
    }
    /**
     * @brief 将链表中值为_x的元素替换为_value,
     * 并返回找到的第一个元素位置,无则返回end(),
     * _all:是否替换链表中的所有元素 
     */ 
    iterator swap_value(const value_type& _x, const value_type& _value, bool _all = false)
    {
        return swap_value(_x, _value, begin(), end(), _all);
    }
    /// 剪切出指定结点，并返回该结点(用于在链表之间对结点进行删除与插入)
    iterator snip(iterator x)
    {
        return _snip(x.node);
    }
    /// 剪切出指定结点，并返回该结点(用于在链表之间对结点进行删除与插入)
    iterator snip(size_t index)
    {
        iterator i = begin();
        for(i = begin();index != 0;++i, --index)
            ;
        return _snip(i.node);
    }
    /// 在链表末尾插入已断开的_x结点
    void splice(iterator _x)
    {
        _insert_node_front(&head,_x.node);
    }
    /// 将已断开的结点_value插入到_x后
    iterator splice(iterator _x, iterator _value)
    {
        return _insert_node_back(_x.node,_value.node);
    }
    /// 将链表中的[_start,_end)区间连接到_dest后,并将其从原链表中剔除,剔除后的长度需要原链表更新
    iterator splice(iterator _dest, iterator _start, iterator _end)
    {
        base_ptr temp = _end.node->prev;//保存原链表中最后一个结点的位置(区间为闭区间,更改_end后无法找到)
        //将要粘接的链表段从原链表中删除
        //不能使用_splice((_start-1).node,_end.node);iterator.operator+()返回临时变量
        _splice(_start.node->prev,_end.node);//_end.node->prev = _start.node->prev;
        //将剪切出的链表段添加到新链表中的_dest后
        count += distance(_start,_end);//更新链表长度
        _splice(temp,_dest.node->next);
        _splice(_dest.node,_start.node);
        return _start;
    }
    /// 将链表_list中的所有元素粘接到_dest后
    iterator splice(iterator _dest, list<T>& _list)
    {
        iterator temp = splice(_dest,_list.begin(),_list.end());
        _list.size();
        return temp;
    }
    /// 反转链表
    void reverse()
    {
        //依次将末尾剪切下并粘贴在链表的开头
        iterator temp = end();
        for(size_t i = 0;i < count;++i)
            temp = splice(temp,snip(--end()));
    }
    /// 清空链表
    void clear()
    {
        while(!empty() && count > 0)
        {
            _erase(reinterpret_cast<base_ptr>(head.next));
        }
        //erase(begin(),end());
    }
    /// 刷新链表长度
    size_t size()
    {
        count = _distance(begin(),end());
        return count;
    }
};
//-----------------------------------------------------
/// 键值大小比较仿函数
template<class T>
struct CompareExample
{
    inline bool operator()(const T& x, const T& y) const
    {
        // 小到大排序
        return x < y;
    }
};
//-----------------------------------------------------
/// 带排序功能的双链表 
template<class T, class Alloc = simple_allocator<T> >
class sort_list : public list<T, Alloc>
{
public:
    typedef typename list<T, Alloc>::value_type value_type;
    typedef typename list<T, Alloc>::iterator iterator;
    typedef list<T, Alloc> base_type;

    sort_list() : base_type() {}
    sort_list(const value_type& _value, int count = 1) : base_type(_value, count) {}
    sort_list(const sort_list<T, Alloc>& sl) : base_type(sl) {}

    /// 按照指定的算法排序(默认为从小到大)
    template<class Compare>
    void sort()
    {
        if(base_type::length() < 2)
            return;
        // 直接插入排序
        for(iterator i = base_type::begin() + 1;i != base_type::end();++i)
        {
            for(iterator j = base_type::begin();j != i;++j)
            {
                if(Compare()(*i, *j))
                {
                    ++i, ++j;
                    base_type::_insert_node_front(j - 1, base_type::snip(i - 1));
                    break;
                }
            }
        }
    }
    /// 按局部小到大排序插入一个值为value的新结点
    template<class Compare>
    iterator sort_insert(const value_type& _value)
    {
        iterator i = base_type::begin();
        while(i != base_type::end() && Compare(*i,_value))
            ++i;
        return base_type::_insert_front(i.node,_value);
    }
    /// 以排序的方式将结点_x插入到链表中
    template<class Compare>
    iterator sort_insert(iterator _x)
    {
        iterator i = base_type::begin();
        while(i != base_type::end() && Compare(*i,*_x))
            ++i;
        return base_type::_insert_front(i.node,_x.node);
    }
};
//---------------------------------------------------------
} // namespace container
} // namespace zhou_yb
//--------------------------------------------------------- 
using zhou_yb::container::list;
using zhou_yb::container::list_iterator;
using zhou_yb::container::list_reverse_iterator;
using zhou_yb::container::sort_list;
//--------------------------------------------------------- 
#endif//#if _STLP_USE_BOOST_SUPPORT
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LIST_H_
//=========================================================
