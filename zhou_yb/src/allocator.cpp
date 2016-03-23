//========================================================= 
/**@file allocator.cpp 
 * @brief base目录下allocator库中全局函数、变量的实现 
 * 
 * @date 2012-06-23   11:17:54 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../container/allocator.h"
#include "../container/object.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_MEMORY_POOL
/// 内存池起始地址 
char* memory_pool_alloc::_start_free = 0;
/// 内存池结束地址 
char* memory_pool_alloc::_end_free = 0;
/// 内存池堆大小 
size_t memory_pool_alloc::_heap_size = 0;
/// 内存池链表指针 
memory_pool_alloc::MemoryBlockObj*
    volatile memory_pool_alloc::_free_list[] = {0};

/// 内存块链表头结点指针 
memory_link_alloc::memory_block_node* memory_link_alloc::_M_head = NULL;
/// 全局负责释放内存的gc
memory_pool_alloc memory_pool_alloc::gc;
#endif
//--------------------------------------------------------- 
} // namespace container 
} // namespace zhou_yb
//=========================================================