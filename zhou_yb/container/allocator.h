//========================================================= 
/**@file allocator.h 
 * @brief 内存池 代码摘自SGI自定义的内存池 
 * 
 * @date 2012-05-26   13:10:28 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
// 忽略对destory(T* p)中p参数未引用的警告 
#pragma warning(disable: 4100)
//--------------------------------------------------------- 
#include "../base/Define.h"
#include <stddef.h>

#include <string.h>
#include <stdlib.h>

#ifndef __APPLE__
#include <new>
#endif
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_MEMORY_POOL
    /// 内存池块间隔大小 
    #ifndef MEMORY_POOL_ALIGN_SIZE
        #define MEMORY_POOL_ALIGN_SIZE 8
    #endif
    /// 内存池最大分配的缓冲区大小 
    #ifndef MEMORY_POOL_MAX_BYTES
        #define MEMORY_POOL_MAX_BYTES 512
    #endif
#endif
//--------------------------------------------------------- 
/**
 * @brief malloc_alloc 系统C方式分配内存适配器 
 *
 * 直接使用系统的malloc,free,realloc操作内存 
 */ 
class malloc_alloc
{
private:
    /// 分配失败时的最大尝试次数 
    enum{ _MAX_REQUEST_COUNT = 256 };
    /// 带尝试次数的malloc
    static void* _malloc(size_t n)
    {
        void* _result;
        // 最多尝试MAX_REQUEST_COUNT次 
        for (int i = 0;i < _MAX_REQUEST_COUNT; ++i) 
        {
            _result = malloc(n);
            if (_result) 
                return(_result);
        }
        return NULL;
    }
    /// 带尝试次数的realloc
    static void* _realloc(void* ptr, size_t n)
    {
        void* _result;
        // 最多尝试MAX_REQUEST_COUNT次 
        for (int i = 0;i < _MAX_REQUEST_COUNT; ++i) 
        {
            _result = realloc(ptr, n);
            if (_result) 
                return(_result);
        }
        return NULL;
    }
public:
    /// 分配空间 malloc 
    static void* allocate(size_t n)
    {
        void* _result = malloc(n);
        if (NULL == _result)
        {
            _result = _malloc(n);
        }

        return _result;
    }
    /// 释放空间 free 
    static void deallocate(void* p, size_t )
    {
        free(p);
    }
    /// 在指定指针上重新分配空间 realloc 
    static void* reallocate(void* p, size_t , size_t new_sz)
    {
        void* _result = realloc(p, new_sz);
        if (NULL == _result) _result = _realloc(p, new_sz);
        return _result;
    }
};
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_MEMORY_POOL
/**
 * @brief 内存池占用内存链表  
 */ 
class memory_link_alloc
{
private:
    struct memory_block_node
    {
        memory_block_node* _M_next;
    };
    static memory_block_node* _M_head;
public:
    /// 分配size_t大小的内存空间 
    static void* allocate(size_t n)
    {
        void* _result;

        _result = malloc(sizeof(memory_block_node) + n);
        if(NULL != _result)
        {
            if(NULL != _M_head)
            {
                memory_block_node* next = reinterpret_cast<memory_block_node*>(_result);
                next->_M_next = _M_head->_M_next;

                _M_head->_M_next = next;
            }
            else
            {
                _M_head = reinterpret_cast<memory_block_node*>(_result);
                _M_head->_M_next = NULL;
            }

            return (reinterpret_cast<char*>(_result) + sizeof(memory_block_node));
        }
        return NULL;
    }
    /**
     * @brief 释放所有内存空间 
     * @retval void
     * @return 
     * @warning 需要确保释放时内存不再被使用中 
     */
    static void release_memory()
    {
        memory_block_node* _node = _M_head;
        void* _buf;
        while(NULL != _node)
        {
            _buf = _node;
            _node = _node->_M_next;

            free(_buf);
        }
        _M_head = NULL;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 内存池 
 */ 
class memory_pool_alloc
{
private:
    /// 分配系数 
    enum {_ALIGN = MEMORY_POOL_ALIGN_SIZE};
    /// 每块内存块的大小 SGI Default: 128
    enum {_MAX_BYTES = MEMORY_POOL_MAX_BYTES};
    /// 内存池大小(内存块数目), SGI Default:16
    enum {_NFREELISTS = _MAX_BYTES / _ALIGN};
    /// 上调内存大小为_ALIGN的整块数 
    static size_t _round_up(size_t n) 
    {
        return (((n) + static_cast<size_t>(_ALIGN) - 1) & ~(static_cast<size_t>(_ALIGN) - 1)); 
    }
private:
    /// 内存池块存储结构(联合体) 
    union MemoryBlockObj
    {
        union MemoryBlockObj* _M_free_list_link;
        char _M_client_data[1];
    };
    static char* _start_free;
    static char* _end_free;
    static size_t _heap_size;
    /// 自由链表 
    static MemoryBlockObj* volatile _free_list[_NFREELISTS];
private:
    /// 获取自由链表中指定大小内存的位置 
    static  size_t _freelist_index(size_t byte_sz) 
    {
        return (((byte_sz)+static_cast<size_t>(_ALIGN)-1) / static_cast<size_t>(_ALIGN) - 1);
    }

    static void* _refill(size_t n)
    {
        int szobjs = 20;
        char* chunk = _chunk_alloc(n, szobjs);
        MemoryBlockObj* volatile* my_free_list;
        MemoryBlockObj* _result;
        MemoryBlockObj* current_obj;
        MemoryBlockObj* next_obj;
        int i;

        if (1 == szobjs) 
            return (chunk);

        my_free_list = _free_list + _freelist_index(n);

        /* 构建自由链表 */
        _result = reinterpret_cast<MemoryBlockObj*>(chunk);
        *my_free_list = next_obj = reinterpret_cast<MemoryBlockObj*>(chunk + n);
        for (i = 1; ; i++) 
        {
            current_obj = next_obj;
            next_obj = reinterpret_cast<MemoryBlockObj*>(reinterpret_cast<char*>(next_obj) + n);
            if (szobjs - 1 == i) 
            {
                current_obj -> _M_free_list_link = NULL;
                break;
            } 
            else 
            {
                current_obj -> _M_free_list_link = next_obj;
            }
        }
        return (_result);
    }
    static char* _chunk_alloc(size_t n, int& szobjs)
    {
        char* _result;
        size_t total_bytes = n * szobjs;
        size_t bytes_left = _end_free - _start_free;

        if (bytes_left >= total_bytes) 
        {
            _result = _start_free;
            _start_free += total_bytes;

            return (_result);
        }
        else if (bytes_left >= n) 
        {
            szobjs = static_cast<int>(bytes_left / n);
            total_bytes = n * szobjs;
            _result = _start_free;
            _start_free += total_bytes;

            return (_result);
        }
        else
        {
            size_t bytes_to_get = 2 * total_bytes + _round_up(_heap_size >> 4);
            MemoryBlockObj* volatile* my_free_list;
            if (bytes_left > 0) 
            {
                my_free_list = _free_list + _freelist_index(bytes_left);

                (reinterpret_cast<MemoryBlockObj*>(_start_free)) -> _M_free_list_link = *my_free_list;
                *my_free_list = reinterpret_cast<MemoryBlockObj*>(_start_free);
            }

            // SGI 中的做法,直接在退出程序时由操作系统回收内存池所占用内存 
            //_start_free = reinterpret_cast<char*>(malloc(bytes_to_get));

            _start_free = reinterpret_cast<char*>(memory_link_alloc::allocate(bytes_to_get));
            
            if (NULL == _start_free) 
            {
                size_t i;
                MemoryBlockObj* ptr;
                
                for(i = n; i <= static_cast<size_t>(_MAX_BYTES); i += static_cast<size_t>(_ALIGN))
                {
                    my_free_list = _free_list + _freelist_index(i);
                    ptr = *my_free_list;
                    if(NULL != ptr)
                    {
                        *my_free_list = ptr->_M_free_list_link;
                        _start_free = reinterpret_cast<char*>(ptr);
                        _end_free = _start_free + i;

                        return (_chunk_alloc(n, szobjs));
                    }
                }
                _end_free = NULL;
                _start_free = reinterpret_cast<char*>(malloc_alloc::allocate(bytes_to_get));
            }
            _heap_size += bytes_to_get;
            _end_free = _start_free + bytes_to_get;

            return (_chunk_alloc(n, szobjs));
        }
    }
public:
    /// 分配空间 
    static void* allocate(size_t n)
    {
        void* ret = NULL;

        if(n > static_cast<size_t>(_MAX_BYTES))
        {
            ret = malloc_alloc::allocate(n);
        }
        else 
        {
            MemoryBlockObj* volatile* my_free_list = _free_list + _freelist_index(n);
            MemoryBlockObj* _result = *my_free_list;
            if (NULL == _result)
            {
                ret = _refill(_round_up(n));
            }
            else 
            {
                *my_free_list = _result -> _M_free_list_link;
                ret = _result;
            }
        }
        return ret;
    };
    /// 释放空间 
    static void deallocate(void* p, size_t n)
    {
        if (n > static_cast<size_t>(_MAX_BYTES))
        {
            malloc_alloc::deallocate(p, n);
        }
        else
        {
            MemoryBlockObj* volatile*  my_free_list = _free_list + _freelist_index(n);
            MemoryBlockObj* q = static_cast<MemoryBlockObj*>(p);

            q->_M_free_list_link = *my_free_list;
            *my_free_list = q;
        }
    }
    /// 重新分配空间 
    static void* reallocate(void* ptr, size_t old_sz, size_t new_sz)
    {
        void* _result;
        size_t copy_sz;

        if(old_sz > static_cast<size_t>(_MAX_BYTES) && new_sz > static_cast<size_t>(_MAX_BYTES))
        {
            return (realloc(ptr, new_sz));
        }
        if(_round_up(old_sz) == _round_up(new_sz))
            return ptr;

        _result = memory_pool_alloc::allocate(new_sz);
        copy_sz = new_sz > old_sz ? old_sz : new_sz;
        memcpy(_result, ptr, copy_sz);
        memory_pool_alloc::deallocate(ptr, old_sz);

        return (_result);
    }
};
//---------------------------------------------------------
typedef memory_pool_alloc default_alloc;
//--------------------------------------------------------- 
#else
typedef malloc_alloc default_alloc;
#endif // NO_INCLUDE_MEMORY_POOL
//--------------------------------------------------------- 
template<class T, class _Alloc = default_alloc>
class simple_alloc
{
public:
    template <class _Tp1> struct rebind 
    {
        typedef simple_alloc<_Tp1> other;
    };

    static T* allocate(size_t n)
    {
        return reinterpret_cast<T*>(_Alloc::allocate(n * sizeof(T)));
    }
    static T* allocate()
    {
        return reinterpret_cast<T*>(_Alloc::allocate(sizeof(T)));
    }
    static void deallocate(T* p, size_t n)
    {
        _Alloc::deallocate(p, n * sizeof(T));
    }
    static void deallocate(T* p)
    {
        _Alloc::deallocate(p, sizeof(T));
    }
    static T* reallocate(T* p, size_t old_sz, size_t new_sz)
    {
        return _Alloc::reallocate(p, old_sz * sizeof(T), new_sz * sizeof(T));
    }
    static void construct(T* p)
    {
        new(p) T();
    }
    static void construct(T* p, const T& val) 
    {
        new(p) T(val);
    }
    static void destroy(T* p)
    {
        p->~T();
    }
};
//--------------------------------------------------------- 
/**
 * @brief 简单的内存分配器(SGI版本)  
 */ 
 template <class T>
 class simple_allocator 
 {
 public:
     typedef size_t     size_type;
     typedef ptrdiff_t  difference_type;
     typedef T*       pointer;
     typedef const T* const_pointer;
     typedef T&       reference;
     typedef const T& const_reference;
     typedef T        value_type;

     template <class _Tp1> struct rebind
     {
         typedef simple_allocator<_Tp1> other;
     };

     simple_allocator() {}
     simple_allocator(const simple_allocator<T>&) {}
     //template <class _Tp1> simple_allocator(const simple_allocator<_Tp1>&) {}
     ~simple_allocator() {}

     pointer address(reference __x) const { return &__x; }
     const_pointer address(const_reference __x) const { return &__x; }

     T* allocate(size_type __n, const void* = NULL) 
     {
         return (__n != 0) ? static_cast<T*>(default_alloc::allocate(__n * sizeof(T))) : NULL;
     }

     void deallocate(pointer __p, size_type __n) { default_alloc::deallocate(__p, __n * sizeof(T)); }

     size_type max_size() const { return static_cast<size_t>(-1) / sizeof(T); }

     void construct(pointer __p, const T& __val) { new(__p) T(__val); }
     void destroy(pointer __p) { __p->~T(); }
 };
 //--------------------------------------------------------- 
 template<>
 class simple_allocator<void>
 {
 public:
     typedef size_t      size_type;
     typedef ptrdiff_t   difference_type;
     typedef void*       pointer;
     typedef const void* const_pointer;
     typedef void        value_type;

     template <class _Tp1> struct rebind 
     {
         typedef simple_allocator<_Tp1> other;
     };
 };
//--------------------------------------------------------- 
#pragma warning(default: 4100)
//--------------------------------------------------------- 
} // napespace container
} // namespace zhou_yb
//========================================================= 
