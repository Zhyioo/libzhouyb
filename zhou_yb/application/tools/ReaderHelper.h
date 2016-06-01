//========================================================= 
/**@file ReaderHelper.h
 * @brief 读卡器操作辅助函数
 * 
 * @date 2016-04-16   10:17:09
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_READERHELPER_H_
#define _LIBZHOUYB_READERHELPER_H_
//--------------------------------------------------------- 
#include <thread>
using std::thread;

#include "../../include/Base.h"
#include "../../include/BaseDevice.h"
#include "../../include/Extension.h"

#include "../device/ComIC_ReaderDevAdapter.h"
using zhou_yb::application::device::ComIC_ReaderDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace tools {
//--------------------------------------------------------- 
/// 线程函数
struct ComScanerTask
{
    bool IsConnected;
    uint Port;
    uint Baud;
    uint TimeoutMs;

    void operator()()
    {
        IsConnected = false;
        ComDevice dev;
        if(ComDeviceHelper::OpenDevice<ComDevice>(dev, Port, Baud) != DevHelper::EnumSUCCESS)
            return;
        
        dev.SetWaitTimeout(TimeoutMs);
        dev.Write(DevCommand::FromAscii("1B 24 49"));
        IsConnected = ComIC_ReaderDevAdapter::HasContactCard(dev);
    }
};
/// 串口IC卡读卡器辅助函数
class ComIC_ReaderHelper
{
protected:
    ComIC_ReaderHelper() {}
public:
    /// 返回设备是否连接
    inline static bool IsConnected(Ref<IInteractiveTrans> comDev)
    {
        return ComIC_ReaderDevAdapter::HasContactCard(comDev);
    }
    /**
     * @brief 扫描设备所连接的端口号
     * 
     * @param [in] baud 设备的波特率
     * @param [in] timeoutMs 等待响应的超时时间(ms)
     */
    static uint ScanReader(uint baud, uint timeoutMs)
    {
        list<uint> comlist;
        ComDevice().EnumDevice(comlist);

        list<ComScanerTask> tasklist;
        list<ComScanerTask>::iterator taskItr;
        list<thread*> threadlist;
        for(uint port : comlist)
        {
            taskItr = tasklist.push_back();
            taskItr->Baud = baud;
            taskItr->Port = port;
            taskItr->TimeoutMs = timeoutMs;

            threadlist.push_back(new thread(*taskItr));
        }
        for(thread* t : threadlist)
        {
            t->join();
            delete t;
        }
        for(ComScanerTask& task : tasklist)
        {
            if(task.IsConnected)
                return task.Port;
        }
        return 0;
    }
};
//--------------------------------------------------------- 
} // namespace zhou_yb
} // namespace application
} // tools
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_READERHELPER_H_
//========================================================= 