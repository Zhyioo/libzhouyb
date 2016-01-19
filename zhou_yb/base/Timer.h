//========================================================= 
/**@file Timer.h 
 * @brief 在Windows下和Linux下兼容的计时器(以ms为单位)
 * 
 * @date 2013-11-04   22:14:34 
 * @author Zhyioo
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "Define.h"

/* 头文件包含 */
#ifdef _WIN32
#   include <Windows.h>
#   pragma comment(lib, "User32.lib")
#else
#   include <sys/time.h>
#   include <unistd.h>
#endif
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base {
//--------------------------------------------------------- 
/// 简易的计时器
class Timer
{
public:
    //----------------------------------------------------- 
#ifdef _WIN32
    /// Windows下获取时差
    static double TimeTick()
    {
        return static_cast<double>(::GetTickCount());
    }
    /// Windows下当前线程消息处理
    static void DoEvent()
    {
        MSG msg;
        while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    /// Windows下延时毫秒(每隔DEV_OPERATOR_INTERVAL做一次DoEvent)
    static void Wait(uint timeMs)
    {
        // 将timeMs拆分成DEV_OPERATOR_INTERVAL进行延时 
        uint time = timeMs / DEV_OPERATOR_INTERVAL;
        for(uint i = 0;i < time; ++i)
        {
            Sleep(DEV_OPERATOR_INTERVAL);
            DoEvent();
        }
        time = timeMs % DEV_OPERATOR_INTERVAL;
        if(time > 0)
        {
            Sleep(time);
        }
    }
#else
    /// Linux下获取时差
    static double TimeTick()
    {
        struct timeval _tv;
        gettimeofday(&_tv, NULL);
        double t = (1000.0*_tv.tv_sec + (_tv.tv_usec/1000.0));

        return t;
    }
    /// Linux下延时毫秒 
    static void Wait(uint timeMs)
    {
    	if(timeMs < 1)
			return;
        usleep(timeMs * 1000);
    }
#endif
    //----------------------------------------------------- 
protected:
    /// 已经流逝的时间(ms)
    double _tickCount;
public:
    //----------------------------------------------------- 
    Timer()
    {
        _tickCount = TimeTick();
    }
    //----------------------------------------------------- 
    /// 重新启动时间 
    inline void Restart()
    {
        _tickCount = TimeTick();
    }
    /// 计算到当前的耗时(ms)
    double Elapsed() const
    {
        double t = static_cast<double>(TimeTick() - _tickCount);
        return t;
    }
    /// 返回耗时,并自动进入下一个计时状态
    double TickCount()
    {
        double tick = Elapsed();
        Restart();
        return tick;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace base
} // namespace zhou_yb
//========================================================= 
