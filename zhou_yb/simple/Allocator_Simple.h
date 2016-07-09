//========================================================= 
/**@file Allocator_Simple.h
 * @brief 
 * 
 * @date 2016-07-09   11:49:31
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ALLOCATOR_SIMPLE_H_
#define _LIBZHOUYB_ALLOCATOR_SIMPLE_H_
//--------------------------------------------------------- 
#include <include/Base.h>
#include <include/Container.h>
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct Allocator_Simple
{
    static void Main(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        int count = 10000000;
        int size = 8;
        {
            Timer timer;
            srand(0);
            void* p = NULL;
            for(int i = 0; i < count; ++i)
            {
                p = memory_pool_alloc::allocate(size);
                memory_pool_alloc::deallocate(p, size);
            }
            _log << timer.Elapsed() << "ms" << endl;
        }
        {
            Timer timer;
            srand(0);
            void* p = NULL;
            for(int i = 0; i < count; ++i)
            {
                p = malloc_alloc::allocate(size);
                malloc_alloc::deallocate(p, size);
            }
            _log << timer.Elapsed() << "ms" << endl;
        }
    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ALLOCATOR_SIMPLE_H_
//========================================================= 