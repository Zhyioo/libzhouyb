//========================================================= 
/**@file HealthCard_Simple.h
 * @brief 居民健康卡示例
 * 
 * @date 2016-05-05   21:00:54
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HEALTHCARD_SIMPLE_H_
#define _LIBZHOUYB_HEALTHCARD_SIMPLE_H_
//--------------------------------------------------------- 
#include "../zhouyb_lib.h"
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct HealthCard_Simple
{
    static void Main(LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        CCID_Device usrdev;
        CCID_Device samdev;
        ByteBuilder usrAtr(8);
        ByteBuilder samAtr(8);
        ByteBuilder atrSession(8);
        HealthCardAppAdapter icAdapter;
        
        usrdev.SelectLogger(_devlog);
        samdev.SelectLogger(_devlog);
        icAdapter.SelectLogger(_devlog);

        list<string> ccidlist;
        usrdev.EnumDevice(ccidlist);

        list<string>::iterator itr;
        for(itr = ccidlist.begin();itr != ccidlist.end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), "contactless", true))
            {
                if(!usrdev.PowerOn(itr->c_str(), &usrAtr))
                    continue;

                ccidlist.erase(itr);
                HealthCardAppAdapter::AtrToSession(usrAtr, atrSession);
                icAdapter.SelectDevice(usrdev);
                break;
            }
        }
        if(usrAtr.IsEmpty())
        {
            _log.WriteLine("连接用户卡失败");
            return;
        }
        if(CCID_DeviceHelper::PowerOn(samdev, "", &samAtr, SIZE_EOF, &ccidlist) != DevHelper::EnumSUCCESS)
        {
            _log.WriteLine("连接SAM卡失败");
            return;
        }

        if(!icAdapter.InitSAM(samdev))
        {
            _log.WriteLine("初始化SAM卡失败");
            return;
        }
        byte cityCode;
        if(!icAdapter.GetCityCode(cityCode))
        {
            _log.WriteLine("获取用户卡城市代码失败");
            return;
        }
        if(!icAdapter.AuthenticateFID(samdev, "DF01|EF05", HealthCardAppAdapter::UpdateKey, atrSession, cityCode))
        {
            _log.WriteLine("认证失败");
            return;
        }
        if(!icAdapter.UpdateTAG(samdev, "DF01|EF05", atrSession, 0x15, DevCommand::FromAscii("11")))
        {
            _log.WriteLine("修改失败");
            return;
        }
        _log.WriteLine("修改成功");
    }
    static void ReadFID(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {
        CCID_Device usrdev;
        CCID_Device samdev;
        ByteBuilder usrAtr(8);
        ByteBuilder samAtr(8);
        ByteBuilder atrSession(8);
        HealthCardAppAdapter icAdapter;
        
        usrdev.SelectLogger(_devlog);
        samdev.SelectLogger(_devlog);
        icAdapter.SelectLogger(_devlog);

        list<string> ccidlist;
        usrdev.EnumDevice(ccidlist);

        list<string>::iterator itr;
        for(itr = ccidlist.begin();itr != ccidlist.end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), "contactless", true))
            {
                if(!usrdev.PowerOn(itr->c_str(), &usrAtr))
                    continue;

                ccidlist.erase(itr);
                HealthCardAppAdapter::AtrToSession(usrAtr, atrSession);
                icAdapter.SelectDevice(usrdev);
                break;
            }
        }
        if(usrAtr.IsEmpty())
        {
            _log.WriteLine("连接用户卡失败");
            return;
        }
        if(CCID_DeviceHelper::PowerOn(samdev, "", &samAtr, SIZE_EOF, &ccidlist) != DevHelper::EnumSUCCESS)
        {
            _log.WriteLine("连接SAM卡失败");
            return;
        }

        if(!icAdapter.InitSAM(samdev))
        {
            _log.WriteLine("初始化SAM卡失败");
            return;
        }
        byte cityCode;
        if(!icAdapter.GetCityCode(cityCode))
        {
            _log.WriteLine("获取用户卡城市代码失败");
            return;
        }
        if(!icAdapter.AuthenticateFID(samdev, sArg, HealthCardAppAdapter::ReadKey, atrSession, cityCode))
        {
            _log.WriteLine("认证失败");
            return;
        }
        ByteBuilder msg(32);
        if(!icAdapter.ReadFID(sArg, msg))
        {
            _log.WriteLine("读数据失败");
            return;
        }
        _log.WriteLine(msg.GetString());
    }
    static void WriteFID(const char* sArg, LoggerAdapter& _log, LoggerAdapter& _devlog)
    {

    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HEALTHCARD_SIMPLE_H_
//========================================================= 