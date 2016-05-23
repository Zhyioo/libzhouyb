//========================================================= 
/**@file property_tree.h 
 * @brief 属性树,结构由横竖的两条list构成 
 * 
 * @date 2013-01-26   11:27:19 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PROPERTY_TREE_H_
#define _LIBZHOUYB_PROPERTY_TREE_H_
//--------------------------------------------------------- 
// 引用_Nonconst_traits,_Const_traits迭代器const属性萃取器 
#include "list.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
/// 树结点基础结构  
struct property_tree_node_base
{
    typedef property_tree_node_base* base_ptr;
    
    /// 父结点 
    base_ptr parent;
    /// 左边相邻的结点 
    base_ptr left;
    /// 右边相邻的结点 
    base_ptr right;
    /// 子结点 
    base_ptr children;
};
//--------------------------------------------------------- 
/// 树结点结构 
template<class T>
struct property_tree_node : public property_tree_node_base
{
    /// 数据项 
    T data;
};
//--------------------------------------------------------- 
/// 树迭代器基础结构 
struct property_tree_iterator_base
{
    typedef property_tree_node_base::base_ptr base_ptr;
    property_tree_node_base* node;

    property_tree_iterator_base() : node(NULL) {}
    explicit property_tree_iterator_base(base_ptr x) : node(x) {}

    bool operator==(const property_tree_iterator_base& x)
    {
        return node == x.node;
    }
    bool operator!=(const property_tree_iterator_base& x)
    {
        return node != x.node;
    }
};
//--------------------------------------------------------- 
/// 辅助完成结点横向的一些基础操作(兄弟结点) 
struct property_tree_node_helper
{
    typedef property_tree_node_base::base_ptr base_ptr;
    
    /// 返回结点是否是最右边的结点 
    static bool is_rightmost(const base_ptr _node)
    {
        return _node->right == _node;
    }
    /// 返回结点是否为最左边的结点 
    static bool is_leftmost(const base_ptr _node)
    {
        return _node->left == _node;
    }
    /// 返回结点是否是最顶层的结点  
    static bool is_topmost(const base_ptr _node)
    {
        return _node->parent == _node;
    }
    /// 返回结点是否是最底层的结点 
    static bool is_bottommost(const base_ptr _node)
    {
        return _node->children == _node;
    }

    /// 获取该结点同层最左边的结点 
    static base_ptr leftmost(base_ptr _node)
    {
        while(!is_leftmost(_node))
            _node = _node->left;
        return _node;
    }
    /// 获取该结点同层最右边的结点 
    static base_ptr rightmost(base_ptr _node)
    {
        while(!is_rightmost(_node))
            _node = _node->right;
        return _node;
    }
    /// 获取该结点最顶层的结点 
    static base_ptr topmost(base_ptr _node)
    {
        while(!is_topmost(_node))
            _node = _node->parent;
        return _node;
    }
    /// 获取该结点最底层的结点 
    static base_ptr bottommost(base_ptr _node)
    {
        while(!is_bottommost(_node))
            _node = _node->children;
        return _node;
    }

    /// 获取该结点的父亲结点 
    static base_ptr direct_parent(base_ptr _node)
    {
        _node = leftmost(_node);
        _node = _node->parent;
        return _node;
    }
};
//--------------------------------------------------------- 
/// 属性树迭代器
template<class T, class Traits>
struct property_tree_iterator : public property_tree_iterator_base
{
    // 模板类型简写定义 
    typedef typename property_tree_iterator_base::base_ptr base_ptr;
    typedef property_tree_node<T>* node_ptr;
    typedef typename Traits::reference reference;
    typedef typename Traits::pointer pointer;
    typedef size_t difference_type;

    typedef property_tree_iterator<T, Traits> self;
    typedef property_tree_iterator<T, typename Traits::_NonConstTraits> iterator;
    typedef property_tree_iterator<T, typename Traits::_ConstTraits> const_iterator;
    typedef T value_type;

    property_tree_iterator() : property_tree_iterator_base(){}
    property_tree_iterator(base_ptr x) : property_tree_iterator_base(x){}
    property_tree_iterator(const iterator& x) : property_tree_iterator_base(x.node){}

    /// *this
    reference operator*() const
    {
        return reinterpret_cast<node_ptr>(node)->data;
    }
    /// this->data
    pointer operator->() const
    {
        return &(operator*());
    }
    self parent()
    {
        base_ptr p = node;
        // 为父结点的第一个子结点
        while(!property_tree_node_helper::is_leftmost(p))
            p = p->left;
        return self(p->parent);
    }

    /// 在当前层中前进一个结点 
    bool next()
    {
        bool can_next = !(property_tree_node_helper::is_rightmost(node));
        if(can_next)
            node = node->right;
        return can_next;
    }
    /// 在当前层中后退一个结点 
    bool prev()
    {
        bool can_prev = !(property_tree_node_helper::is_leftmost(node));
        if(can_prev)
            node = node->left;
        return can_prev;
    }
    /// 往上移动一层 
    bool up()
    {
        bool can_up = !(property_tree_node_helper::is_topmost(node));
        if(can_up)
            node = node->parent;
        return can_up;
    }
    /// 往下移动一层 
    bool down()
    {
        bool can_down = !(property_tree_node_helper::is_bottommost(node));
        if(can_down)
            node = node->children;
        return can_down;
    }
};
//--------------------------------------------------------- 
/// 属性树(需要T支持无参数构造函数和拷贝构造函数) 
template<class T, class Alloc = simple_alloc<T> >
class property_tree
{
public:
    typedef property_tree_node_base::base_ptr base_ptr;
    typedef property_tree_node<T> node;
    typedef property_tree_node<T>* node_ptr;
    typedef property_tree_iterator<T, _Nonconst_traits<T> > iterator;
    typedef property_tree_iterator<T, _Const_traits<T> > const_iterator;
    typedef typename property_tree_iterator<T, _Nonconst_traits<T> >::reference reference;
    typedef typename property_tree_iterator<T, _Const_traits<T> >::reference const_reference;
    typedef T value_type;
    typedef Alloc allocator_type;
    typedef typename Alloc::template rebind<node>::other AllocOther;

    enum insert_place
    {
        insert_back,
        insert_front,
        insert_up,
        insert_down,
        insert_child
    };
protected:
    /// 跟结点 
    property_tree_node_base _root;
    /// 初始化head结点 
    inline void _init() 
    {
        _root.children = &_root;
        _root.left = &_root;
        _root.right = &_root;
        _root.parent = &_root;
    }
    /// 初始化一个结点
    node_ptr _init_node(node_ptr node)
    {
        node->children = node;
        node->right = node;
        node->left = node;
        node->parent = node;

        return node;
    }
    /// 产生一个值为_value的结点,并返回起始地址
    node_ptr _creat_memory(const value_type& _value = T())
    {
        node_ptr temp = AllocOther::allocate();
        Alloc::construct(&(temp->data), _value);

        return _init_node(temp);
    }
    /// 产生一个使用默认构造函数生成的结点
    node_ptr _create_memory()
    {
        node_ptr temp = AllocOther::allocate();
        Alloc::construct(&(temp->data));

        return _init_node(temp);
    }
    /// 从树中剪切出_node结点(附带子结点),并返回结点的指针
    inline node_ptr _snip(base_ptr _node)
    {
        // 为最左边的结点 
        if(property_tree_node_helper::is_leftmost(_node))
        {
            // 该结点没有兄弟结点 
            if(property_tree_node_helper::is_rightmost(_node))
            {
                // 直接修改父结点 
                _node->parent->children = _node->parent;
                _node->parent = _node;
            }
            // 设置父结点的子结点为该结点的兄弟结点 
            else
            {
                _node->parent->children = _node->right;
                _node->right->parent = _node->parent;
                _node->right->left = _node->right;

                _node->parent = _node;
                _node->right = _node;
            }
            return reinterpret_cast<node_ptr>(_node);
        }
        // 同一层中最后一个结点 
        if(property_tree_node_helper::is_rightmost(_node))
        {
            _node->left->right = _node->left;
            _node->left = _node;

            return reinterpret_cast<node_ptr>(_node);
        }

        // 有左右结点 
        base_ptr _left = _node->left;
        base_ptr _right = _node->right;

        _left->right = _right;
        _right->left = _left;

        _node->left = _node;
        _node->right = _node;

        return reinterpret_cast<node_ptr>(_node);
    }
    /// 删除结点 
    void _erase(base_ptr _node)
    {
        base_ptr tmp = _snip(_node);
        /// 删除_node结点和所有子结点(递归删除结点) 
        if(property_tree_node_helper::is_bottommost(tmp))
        {
            node_ptr p = reinterpret_cast<node_ptr>(tmp);
            Alloc::destroy(&(p->data));
            AllocOther::deallocate(p);

            return ;
        }
        _erase(_node->children);
    }
    /// 在x结点后插入已有的结点_value
    inline void _insert_node_back(base_ptr x, base_ptr _value)
    {
        if(!property_tree_node_helper::is_rightmost(x))
        {
            _value->right = x->right;
            x->right->left = _value;
        }
        x->right = _value;
        _value->left = x;
    }
    /// 在x结点前插入已有的结点_value
    inline void _insert_node_front(base_ptr x, base_ptr _value)
    {
        if(!property_tree_node_helper::is_leftmost(x))
        {
            _value->left = x->left;
            x->left->right = _value;
        }
        else
        {
            // 设置父结点 
            _value->parent = x->parent;
            x->parent->children = _value;
            x->parent = x;
        }
        x->left = _value;
        _value->right = x;
    }
    /// 在x结点上插入已有的结点_value
    inline void _insert_node_up(base_ptr x, base_ptr _value)
    {
        if(!property_tree_node_helper::is_topmost(x))
        {
            _value->parent = x->parent;
            x->parent->children = _value;
        }
        x->parent = _value;
        _value->children = x;
    }
    /// 在x结点下插入已有的结点_value
    inline void _insert_node_down(base_ptr x, base_ptr _value)
    {
        if(!property_tree_node_helper::is_bottommost(x))
        {
            _value->children = x->children;
            x->children->parent = _value;
        }
        x->children = _value;
        _value->parent = x;
    }
    /// 在x结点的子节点中插入已有的结点_value
    inline void _insert_node_child(base_ptr x, base_ptr _value)
    {
        // 暂时没有子结点 
        if(property_tree_node_helper::is_bottommost(x))
        {
            _insert_node_down(x, _value);
        }
        else
        {
            base_ptr i = property_tree_node_helper::rightmost(x->children);
            _insert_node_back(i, _value);
        }
    }
    /// 返回x结点的子结点数(递归) 
    size_t _get_count(base_ptr x)
    {
        if(property_tree_node_helper::is_bottommost(x))
            return 0;
        size_t len = 0;
        base_ptr tmp = x->children;
        while(!property_tree_node_helper::is_rightmost(tmp))
        {
            ++len;
            // 获取子结点的子结点数目 
            len += _get_count(tmp);

            tmp = tmp->right;
        }
        ++len;
        len += _get_count(tmp);

        return len;
    }
public:
    property_tree(){ _init(); }
    property_tree(iterator root)
    {
        _init();
        _insert_node_down(_root, root.node);
    }
    virtual ~property_tree(){ clear(); }

    /// 返回树是否为空
    inline bool empty() const 
    {
        return property_tree_node_helper::is_bottommost(reinterpret_cast<base_ptr>(const_cast<property_tree_node_base*>(&_root)));
    }

    /// 属性树根结点 
    iterator root()
    {
        return iterator(&_root);
    }
    /// 属性树根结点 
    const_iterator root() const
    {
        return const_iterator(&_root);
    }
    /// 属性树尾结点(无效的结点)
    iterator eof()
    {
        return iterator(NULL);
    }
    /// 属性树尾结点(无效的结点)
    const_iterator eof() const
    {
        return const_iterator(NULL);
    }
    /* 编辑接口 */
    /// 在指定的结点位置插入已经存在的空结点 
    iterator insert(iterator _x, iterator _value, insert_place place)
    {
        // 不允许在空的根结点的前后上面面追加其他结点 
        if(empty() && _x == root())
        { 
            _insert_node_down(_x.node, _value.node);
            return _value;
        }

        switch(place)
        {
        case insert_front:
            _insert_node_front(_x.node, _value.node);
            break;
        case insert_up:
            _insert_node_up(_x.node, _value.node);
            break;
        case insert_down:
            _insert_node_down(_x.node, _value.node);
            break;
        case insert_back:
            _insert_node_back(_x.node, _value.node);
            break;
        case insert_child:
            _insert_node_child(_x.node, _value.node);
            break;
        default:
            break;
        }
        return _value;
    }
    /// 在指定结点位置插入值为value的新结点
    iterator insert(iterator _x, const value_type& _value, insert_place place)
    {
        node_ptr itr = _creat_memory(_value);
        return insert(_x, iterator(itr), place);
    }
    /// 在指定结点位置插入一个新的结点
    iterator insert(iterator _x, insert_place place)
    {
        node_ptr itr = _creat_memory();
        return insert(_x, iterator(itr), place);
    }
    /// 剪切结点 
    iterator snip(iterator x)
    {
        // 必须保证root结点下面有有效结点 
        if(x == root())
            return _snip(x.node->children);
        return _snip(x.node);
    }
    /// 删除指定的结点 
    void erase(iterator x)
    {
        _erase(x.node);
    }
    /// 清空树 
    void clear()
    {
        clear(root());
    }
    /// 清空树的子结点 
    void clear(iterator x)
    {
        while(!property_tree_node_helper::is_bottommost(x.node))
            _erase(x.node->children);
    }
    /// 返回树中指定结点的子结点数(递归) 
    size_t count(iterator itr)
    {
        return _get_count(itr.node);
    }
};
//--------------------------------------------------------- 
} // namespace container
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PROPERTY_TREE_H_
//========================================================= 
