//========================================================= 
/**@file PBOC_Simple.h
 * @brief 
 * 
 * @date 2016-04-22   14:52:47
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_SIMPLE_H_
#define _LIBZHOUYB_PBOC_SIMPLE_H_
//--------------------------------------------------------- 
#include <include/Base.h>
#include <include/BaseDevice.h>
#include <include/Extension.h>

#include <application/pboc/pboc_app.h>
#include <application/pboc/pboc_app.cpp>
using namespace zhou_yb::application::pboc;
//--------------------------------------------------------- 
struct PBOC_Simple
{
    static void GetInformation(Ref<IICCardDevice> dev, const char* sAid, const char* sTag, const char* sTerminal, LoggerAdapter& _log)
    {
        ByteBuilder aid(8);
        ByteBuilder tag(32);
        ByteBuilder infoTag(32);

        ByteBuilder _iccTerminalVal;
        DevCommand::FromAscii(sAid, aid);
        DevCommand::FromAscii(sTag, tag);
        DevCommand::FromAscii(sTerminal, _iccTerminalVal);

        PBOC_v2_0_AppAdapter _icAdapter;
        _icAdapter.SelectDevice(dev);

        LOGGER(_icAdapter.SelectLogger(_log));
        _icAdapter.TerminalValue.Clear();
        _icAdapter.TerminalValue += _iccTerminalVal;
        // 添加默认的时间
        ByteBuilder localDate(4);
        ByteBuilder localTime(3);
        PBOC_Library::GetLocalTime(&localDate, &localTime);
        TlvConvert::MakeTLV(0x9A, localDate.SubArray(1), _iccTerminalVal);
        TlvConvert::MakeTLV(0x9F21, localTime, _iccTerminalVal);
        DevCommand::FromAscii(SYS_PBOC_V2_0_TERMINAL_TAGVALUE, _icAdapter.TerminalValue);

        // GAC标签8C 8D
        infoTag = tag;
        PBOC_AppHelper::assertTagHeaderAscii(tag, "82 8C 8D");
        _icAdapter.Random();

        ByteBuilder _iccAppData(256);
        _icAdapter.Random();
        if(!_icAdapter.GetInformation(aid, tag, "", _iccAppData))
            return ;

        ByteBuilder arqc(32);
        if(!_icAdapter.GenArqc(_iccTerminalVal, arqc, _iccAppData))
            return ;

        // 提取标签
        list<TlvHeader> taglist;
        PBOC_Library::MakeTagList(infoTag, taglist);

        ByteBuilder infoBuff(32);
        ByteBuilder icData(256);
        icData += arqc;
        icData += _iccAppData;
        icData += _icAdapter.TerminalValue;
        PBOC_Library::SelectTagToBuffer(taglist, icData, infoBuff);

        _log.WriteLine("INFO:");
        _log.WriteStream(infoBuff) << endl;

        char arpc[] = "910A11223344556677883030 72189F180400000001860F04DA9F790A0000000002007C0DE18C";
        ByteBuilder df31(8);
        _icAdapter.RunArpc(_iccTerminalVal, DevCommand::FromAscii(arpc), _iccAppData, df31);

        _log.WriteStream(df31) << endl;

        return;
    }
};
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_SIMPLE_H_
//========================================================= 
