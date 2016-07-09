//========================================================= 
/**@file ByteBuilder_Simple.h
 * @brief 
 * 
 * @date 2016-07-09   15:05:44
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BYTEBUILDER_SIMPLE_H_
#define _LIBZHOUYB_BYTEBUILDER_SIMPLE_H_
//--------------------------------------------------------- 
#include <include/Base.h>
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct ByteBuilder_Simple
{
    static void Main(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        ByteBuilder buff;
        
        _log << "Size:" << buff.GetSize() << endl;
        DevCommand::FromAscii("11 22 33 44 55 66 77 88 99", buff);

        _log << "Length:<" << buff.GetLength() << endl;

        byte cmd[] = { 0x11, 0x22, 0x33, 0x44 };
        ByteArray cmdArray(cmd, sizeof(cmd));

        ByteArray subBuff = buff.SubArray(0, 4);
        _log << "IsStartWith:" << StringConvert::StartWith(cmdArray, subBuff) << endl;
    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BYTEBUILDER_SIMPLE_H_
//========================================================= 