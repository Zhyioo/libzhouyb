//========================================================= 
/**@file CCID_TestLinker.h 
 * @brief CCID设备连接器 
 * 
 * @date 2015-01-24   19:26:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_TESTLINKER_H_
#define _LIBZHOUYB_CCID_TESTLINKER_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// CCID读卡器的连接器 
struct CCID_DeviceLinker : public TestLinker<CCID_Device>
{
    /**
     * @brief 查找是否有指定的读卡器
     * @param [in] dev 需要连接的读卡器
     * @param [in] arg 参数列表
     * - 参数
     *  - Name 需要连接的读卡器名称
     * .
     * @param [in] printer 输出器   
     */
    virtual bool Link(CCID_Device& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        list<string> devlist;
        list<string>::iterator itr;

        dev.EnumDevice(devlist);
        LOGGER(dev.SelectLogger(printer.GetLogger()));

        bool bLink = false;
        string ccidName = arg["Name"].To<string>();
        for(itr = devlist.begin(); itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), ccidName.c_str(), true))
            {
                bLink = true;
                break;
            }
        }

        LOGGER(dev.ReleaseLogger(&printer.GetLogger()));

        return bLink;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_TESTLINKER_H_
//========================================================= 