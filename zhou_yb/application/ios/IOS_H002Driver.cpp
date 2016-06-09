//========================================================= 
/**@file IOS_H002Driver.cpp
 * @brief IOS下H002命令驱动资源文件
 * 
 * @date 2016-06-09   11:40:28
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IOS_H002DRIVER_CPP_
#define _LIBZHOUYB_IOS_H002DRIVER_CPP_
//--------------------------------------------------------- 
#include "IOS_H002Driver.h"
//--------------------------------------------------------- 
/// IOS下H002命令驱动
template<class TArgParser>
class IOS_H002Driver : public CommandDriver<TArgParser>
{
protected:
    LOGGER(FolderLogger _folder);
    LoggerAdapter _log;
    id<BluetoothDeviceDelegate> _dev;
    BluetoothDeviceInvoker _devInvoker;
    LC_ComStreamCmdAdapter _streamCmdAdapter;

    H002CmdDriver<ArgParser> _h002;
public:
    IOS_H002Driver()
    {

    }
};
//--------------------------------------------------------- 
@implementation H002Driver : NSObject
{
    IOS_H002Driver _h002;
}
-(id)init{
    if(self = [super init]) {
        
    }
    return self;
}
/// 蓝牙操作
- (BOOL)TransmitCommand : (NSString*)sCmd, (NSString*)sArg, (NSMutableString*)sRecv{

}
@end
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IOS_H002DRIVER_CPP_
//========================================================= 