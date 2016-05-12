//========================================================= 
/**@file HealthCard_Simple.h
 * @brief ���񽡿���ʾ��
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
struct HealthCard_Simple
{
    static void Main(LoggerAdapter _log, LoggerAdapter _devlog)
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
            _log.WriteLine("�����û���ʧ��");
            return;
        }
        if(CCID_DeviceHelper::PowerOn(samdev, "", &samAtr, SIZE_EOF, &ccidlist) != DevHelper::EnumSUCCESS)
        {
            _log.WriteLine("����SAM��ʧ��");
            return;
        }

        if(!icAdapter.InitSAM(samdev))
        {
            _log.WriteLine("��ʼ��SAM��ʧ��");
            return;
        }
        byte cityCode;
        if(!icAdapter.GetCityCode(cityCode))
        {
            _log.WriteLine("��ȡ�û������д���ʧ��");
            return;
        }
        if(!icAdapter.SelectAid("DF01|EF05"))
        {
            _log.WriteLine("ѡ���û���AIDʧ��");
            return;
        }
        if(!icAdapter.AuthenticateFID(samdev, "DF01|EF05", HealthCardAppAdapter::UpdateKey, atrSession, cityCode))
        {
            _log.WriteLine("��֤ʧ��");
            return;
        }
    }
};
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HEALTHCARD_SIMPLE_H_
//========================================================= 