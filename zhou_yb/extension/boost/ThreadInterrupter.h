//========================================================= 
/**@file ThreadInterrupter.h
 * @brief BOOST线程库中断器  
 * 
 * @date 2015-04-16   22:04:02
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include <boost/thread/thread.hpp>

#include "../../include/Base.h"
#include "../../include/Container.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace boost_helper {
//--------------------------------------------------------- 
/// BOOST中用于中断线程的中断器  
class ThreadInterrupter : public IInterrupter, public RefObject
{
private:
    /// 中断状态 
    bool _interrupt;
public:
    ThreadInterrupter()
    {
        _interrupt = false;
    }
    /// 中断点 
    virtual bool InterruptionPoint()
    {
        if(_interrupt) return _interrupt;
        try
        {
            boost::this_thread::interruption_point();
        }
        catch(boost::thread_interrupted&)
        {
            _interrupt = true;
        }
        return _interrupt;
    }
    /**
     * @brief 返回是否成功中断
     * @warning 中断由线程的函数 ->interrupt() 触发 
     */
    virtual bool Interrupt()
    {
        return _interrupt;
    }
    /// 重置中断为初始状态 
    inline bool Reset()
    {
        _interrupt = false;
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace boost_helper
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 