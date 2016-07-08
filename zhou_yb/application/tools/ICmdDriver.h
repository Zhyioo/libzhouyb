//========================================================= 
/**@file ICmdDriver.h
 * @brief 将CmdDriver导出为DLL驱动
 * 
 * @date 2016-07-03   16:26:39
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICMDDRIVER_H_
#define _LIBZHOUYB_ICMDDRIVER_H_
//--------------------------------------------------------- 
#include "IConverter.h"
//--------------------------------------------------------- 
/* 直接使用IConvert接口来导出 */
#define CMD_DRIVER_EXPORTS(name, className) \
className _##name##_driver; \
bool name##_TransmitCommand(const char* sCmd, ByteBuilder& sRlt, const char* sArg) \
{ \
    return _##name##_driver.TransmitCommand(sCmd, sArg, sRlt); \
} \
CONVERTER_EXPORT(name)
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICMDDRIVER_H_
//========================================================= 