//========================================================= 
/**@file WinComHelper.h
 * @brief Windows下串口号修改工具
 * 
 * @date 2016-05-28   14:30:41
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINCOMHELPER_H_
#define _LIBZHOUYB_WINCOMHELPER_H_
//--------------------------------------------------------- 
#include "../../extension/ability/win_helper/SetupDiHelper.h"
using zhou_yb::extension::ability::SetupDiHelper;

#include "../../extension/ability/win_helper/WinRegistryKey.h"
using zhou_yb::extension::ability::RegistryKey;
using zhou_yb::extension::ability::RegValueItem;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace tools {
//--------------------------------------------------------- 
/// Windows下串口属性值
struct ComProperty
{
    string Description;
};
/// Windows下串口的辅助类
class WinComHelper
{
protected:
    WinComHelper() {}
public:
    static size_t EnumComPort(list<string>& comlist)
    {
        // 通过SetupDi接口枚举串口
        const GUID COM_GUID = { 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 };
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&COM_GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        int i;
        SP_DEVINFO_DATA DeviceInfoData = { sizeof(DeviceInfoData) };
        DWORD DataT;
        DWORD buffersize = 1024;
        DWORD req_bufsize = 0;
        size_t count = 0;

        char_t friendlyName[1024] = { 0 };
        CharConverter cvt;

        ByteArray comMsgArray;
        for(i = 0; i < 10; i++)
        {
            SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData);
            if(!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, &DataT, (LPBYTE)friendlyName, buffersize, &req_bufsize))
                continue;
            comMsgArray = ByteArray(cvt.to_char(friendlyName));
            comlist.push_back(comMsgArray.GetString());

            if(StringConvert::Contains(comMsgArray, "COM48"))
            {
                // 修改注册表
                RegistryKey regKey = RegistryKey::OpenKey("HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM", KEY_READ | KEY_WRITE);
                if(regKey.IsValid())
                {
                    list<RegValueItem> regValues;
                    regKey.EnumValue(regValues);

                    list<RegValueItem>::iterator itr;
                    string sVal;
                    for(itr = regValues.begin();itr != regValues.end(); ++itr)
                    {
                        sVal = itr->Item.Value.GetString();
                        if(sVal == "COM48")
                        {
                            itr->Item.Value = "COM30";
                            regKey.SetValue(*itr);
                            break;
                        }
                    }
                }

                // 禁用设备
                SP_PROPCHANGE_PARAMS params = { sizeof(SP_CLASSINSTALL_HEADER) };
                params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
                params.Scope = DICS_FLAG_GLOBAL;
                params.StateChange = DICS_DISABLE;
                SetupDiSetClassInstallParams(hDevInfo, &DeviceInfoData, (SP_CLASSINSTALL_HEADER*)&params, sizeof(params));
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData);

                // 启用设备
                params.StateChange = DICS_ENABLE;
                SetupDiSetClassInstallParams(hDevInfo, &DeviceInfoData, (SP_CLASSINSTALL_HEADER*)&params, sizeof(params));
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData);

                params.StateChange = DICS_ENABLE;
                SetupDiSetClassInstallParams(hDevInfo, &DeviceInfoData, (SP_CLASSINSTALL_HEADER*)&params, sizeof(params));
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData);
            }
            ++count;
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
        return count;
    }
};
//--------------------------------------------------------- 
} // namespace tools
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINCOMHELPER_H_
//========================================================= 