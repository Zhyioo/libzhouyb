//========================================================= 
/**@file ThreadInterrupter.h
 * @brief BOOST线程库中断器  
 * 
 * @date 2015-04-16   22:04:02
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_THREADINTERRUPTER_H_
#define _LIBZHOUYB_THREADINTERRUPTER_H_
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
    /// 中断点状态
    bool _isBreakout;
public:
    ThreadInterrupter()
    {
        _interrupt = false;
        _isBreakout = true;
    }
    /// 中断点 
    virtual bool InterruptionPoint()
    {
        _isBreakout = false;
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
    /// 设置中断器已中断(在中断点中处理)
    virtual void Breakout()
    {
        _isBreakout = true;
    }
    /// 返回是否已经处理完中断点
    virtual bool IsBreakout()
    {
        return _isBreakout;
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
#endif // _LIBZHOUYB_THREADINTERRUPTER_H_
//========================================================= 