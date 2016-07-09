//========================================================= 
/**@file Tlv_Simple.h
 * @brief 
 * 
 * @date 2016-04-30   09:45:29
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TLV_SIMPLE_H_
#define _LIBZHOUYB_TLV_SIMPLE_H_
//--------------------------------------------------------- 
#include <include/Base.h>
#include <include/BaseDevice.h>
#include <include/Extension.h>
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct Tlv_Simple
{
    static void Main_Tlv(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog, TlvElement::TlvMode mode)
    {
        ByteBuilder buff(32);
        DevCommand::FromAscii(sArg, buff);

        TlvElement tagElement = TlvElement::Parse(buff, mode);
        _log << "Root.IsEmpty=" << tagElement.IsEmpty() << endl;

        TlvElement subElement = tagElement.MoveNext();
        ByteBuilder tmp(8);
        while(!subElement.IsEmpty())
        {
            _log << "HEAD:" << TlvConvert::ToHeaderAscii(subElement.GetHeader()) << ' ';

            tmp.Clear();
            subElement.GetValue(tmp);
            _log.WriteStream(tmp) << endl;

            subElement = tagElement.MoveNext();
        }
    }

    static void Main_Normal(LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        _log.WriteLine("Tlv Normal:");
        Main_Tlv("71153701B42302D8077103617AD1DB5FADAD67012CA75C", _log, _devlog, TlvElement::Normal);
    }

    static void Main_NormalIgnoreFormat(LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        _log.WriteLine("Tlv NormalIgnoreFormat:");
        Main_Tlv("71153701B42302D8077103617AD1DB5FADAD67012CA75C", _log, _devlog, TlvElement::NormalIgnoreFormat);
    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TLV_SIMPLE_H_
//========================================================= 