﻿//========================================================= 
/**@file WinCCID_EscapeCommandHelper.h 
 * @brief Windows下CCID设备EscapeCommand名称编辑
 * 
 * @date 2013-09-02   22:07:25 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINCCID_ESCAPECOMMANDHELPER_H_
#define _LIBZHOUYB_WINCCID_ESCAPECOMMANDHELPER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
#include "WinRegistryKey.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// EscapeCommand注册表名 
#define REG_EscapeCommandName "EscapeCommandEnable"
/// PCSC GUID
#define PCSC_CCID_GUID "{50dd5230-ba8a-11d1-bf5d-0000f805f530}"
//--------------------------------------------------------- 
/// USB设备注册表信息 
struct UsbDevRegInfo
{
    bool IsSelected;
    bool IsSupport;
    string Class;
    string Guid;
    string Info;
    string Mfg;
    string Service;
    string RegeditKey;
    string RootKey;
};
/// USB Reg结构
struct UsbRegNode
{
    string rootInfo;
    list<UsbDevRegInfo> subInfo;
};
/// Windows下CCID设备EscapeCommand功能编辑器 
class WinCCID_EscapeCommandHelper
{
protected:
    WinCCID_EscapeCommandHelper() {}

    /// 获取子设备下面是否支持EscapeCommand
    static bool _GetCCIDEscapeParams(RegistryKey rootKey, string& pararmsName)
    {
        // 查找设备所安装的驱动服务信息
        list<RegValueItem> subItems;
        rootKey.EnumValue(subItems);

        bool isWudfService = false;
        list<RegValueItem>::iterator itemItr;
        for(itemItr = subItems.begin();itemItr != subItems.end(); ++itemItr)
        {
            ByteBuilder tmp = (*itemItr).Name.c_str();
            StringConvert::ToLower(tmp);

            if(tmp == "service")
            {
                tmp.Clear();
                tmp = (*itemItr).Item.ToString().c_str();
                if(StringConvert::StartWith(tmp, "WUDF"))
                {
                    isWudfService = true;
                }
                break;
            }
        }

        list<string> subKeyNames;
        rootKey.GetSubKeyNames(subKeyNames);

        bool isSupport = false;
        list<string>::iterator itr;
        for(itr = subKeyNames.begin();itr != subKeyNames.end(); ++itr)
        {
            if(StringConvert::StartWith(ByteArray(itr->c_str(), itr->length()), "Device Parameters", true))
            {
                RegistryKey pararmsKey = rootKey.OpenSubKey(itr->c_str());
                pararmsName = (*itr);
                
                if(isWudfService)
                {
                    // 检查是否是Win7下的CCID驱动 
                    list<string> pararmsKeyNames;
                    list<string>::iterator pararmsItr;
                    pararmsKey.GetSubKeyNames(pararmsKeyNames);
                    for(pararmsItr = pararmsKeyNames.begin();pararmsItr != pararmsKeyNames.end(); ++pararmsItr)
                    {
                        // WUDFUsbccidDriver
                        if(StringConvert::StartWith(ByteArray(pararmsItr->c_str(), pararmsItr->length()), "WUDFUsbccidDriver", true))
                        {
                            pararmsKey = pararmsKey.OpenSubKey(pararmsItr->c_str());
                            pararmsName += "\\";
                            pararmsName += (*pararmsItr);
                            break;
                        }
                    }
                }

                list<RegValueItem> regItems;
                pararmsKey.EnumValue(regItems);

                list<RegValueItem>::iterator itrItem;
                for(itrItem = regItems.begin();itrItem != regItems.end(); ++itrItem)
                {
                    if(StringConvert::StartWith(ByteArray(itrItem->Name.c_str(), itrItem->Name.length()), "EscapeCommandEnable", true))
                    {
                        string itemVal = itrItem->Item.ToString();
                        const char* escapeVal = itemVal.c_str();
                        DWORD dwEscapeCommand = ArgConvert::FromString<int>(escapeVal);
                        isSupport = (dwEscapeCommand == TRUE);

                        return isSupport;
                    }
                }
            }
        }

        return isSupport;
    }
    /// 设置子设备下面的EscapeCommand值 NULL:删除,TRUE:开启,FALSE:禁用
    static bool _SetCCIDEscapeParams(RegistryKey rootKey, bool* pIsEnable, const char* paramsName)
    {
        // EscapeCommand 路径为 : rootKey(UsbDevRegInfo.RootKey) + \ + paramsName("..\VID_1DFC&PID_8913\6&4817b6d&0&3" "\" "Device Parameters\WUDFUsbccidDriver")
        RegistryKey key = RegistryKey::NullRegistryKey;
        // 如果是要设置成True,则直接创建新的建 
        if(pIsEnable != NULL && (*pIsEnable)) 
        {
            key = rootKey.CreateSubKey(paramsName);
        }
        // 已经有键直接带权限打开即可 
        else
        {
            key = rootKey.OpenSubKey(paramsName, KEY_WRITE | KEY_QUERY_VALUE);
        }

        if(!key.IsValid())
            return false;

        if(pIsEnable != NULL)
        {
            DWORD val = (*pIsEnable) ? 1 : 0;
            return key.SetValue(REG_EscapeCommandName, REG_DWORD, &val, sizeof(DWORD));
        }

        return key.DeleteValue(REG_EscapeCommandName);
    }
    /// 获取子设备下面的USB信息(每个VID,PID的设备不同端口有不用的设置) 
    static size_t _GetSubUsbDevInfo(RegistryKey rootKey, list<UsbDevRegInfo>& subInfo)
    {
        list<string> subKeyNames;
        rootKey.GetSubKeyNames(subKeyNames);
        bool isCCID = false;
        size_t count = 0;

        list<string>::iterator itr;
        for(itr = subKeyNames.begin();itr != subKeyNames.end(); ++itr)
        {
            RegistryKey subKey = rootKey.OpenSubKey(itr->c_str());
            subInfo.push_back();

            isCCID = false;
            
            UsbDevRegInfo* pInfo = &(subInfo.back());
            list<RegValueItem> valItems;
            subKey.EnumValue(valItems);

            list<RegValueItem>::iterator itrVal;
            for(itrVal = valItems.begin();itrVal != valItems.end(); ++itrVal)
            {
                ByteBuilder tmp = (*itrVal).Name.c_str();
                StringConvert::ToLower(tmp);
                
                if(tmp == "service")
                    pInfo->Service = (*itrVal).Item.ToString();
                else if(tmp == "classguid")
                {
                    pInfo->Guid = (*itrVal).Item.ToString();
                    tmp.Clear();
                    tmp = pInfo->Guid.c_str();
                    // 是CCID的设备则判断EscapeCommand的值 
                    if(StringConvert::Compare(tmp, ByteArray(PCSC_CCID_GUID), true))
                    {
                        ++count;
                        isCCID = true;
                        pInfo->IsSupport = _GetCCIDEscapeParams(subKey, pInfo->RegeditKey);
                    }
                }
                else if(tmp == "locationinformation")
                    pInfo->Info = (*itrVal).Item.ToString();
                else if(tmp == "class")
                    pInfo->Class = (*itrVal).Item.ToString();
                else if(tmp == "mfg")
                    pInfo->Mfg = (*itrVal).Item.ToString();
            }
            // 如果不是CCID的设备则移除 
            if(isCCID)
            {
                pInfo->RootKey = subKey.Name();
                pInfo->IsSelected = true;
            }
            else
            {
                subInfo.pop_back();
            }
        }
        return count;
    }
public:
    //----------------------------------------------------- 
    /* 中间层接口 */
    /// 枚举CCID的设备信息 
    static size_t EnumUsbRegInfo(list<UsbRegNode>& ccidInfo)
    {
        RegistryKey usbKey = RegistryKey::LocalMachine.OpenSubKey("SYSTEM\\CurrentControlSet\\Enum\\USB");
        list<string> subKeyNames;
        size_t count = 0;

        usbKey.GetSubKeyNames(subKeyNames);

        list<string>::iterator itr;
        for(itr = subKeyNames.begin();itr != subKeyNames.end(); ++itr)
        {
            if(StringConvert::StartWith(ByteArray(itr->c_str(), itr->length()), "VID", true))
            {
                ccidInfo.push_back();

                UsbRegNode* pSubTree = &(ccidInfo.back());
                pSubTree->rootInfo = (*itr);

                if(_GetSubUsbDevInfo(usbKey.OpenSubKey(itr->c_str()), pSubTree->subInfo) < 1)
                    ccidInfo.pop_back();
                else
                    ++count;
            }
        }

        return count;
    }
    /// 打印UsbRegInfo信息 
    static void PrintUsbRegInfo(LoggerAdapter& logAdapter, list<UsbRegNode>* pCcidInfo = NULL)
    {
        list<UsbRegNode> ccidInfo;
        if(pCcidInfo == NULL)
        {
            EnumUsbRegInfo(ccidInfo);
            pCcidInfo = &ccidInfo;
        }

        list<UsbRegNode>::iterator itr;
        int devIndex = 0;
        int subDevIndex = 0;

        logAdapter<<"Count:<"<<pCcidInfo->size()<<">\n";
        for(itr = pCcidInfo->begin();itr != pCcidInfo->end(); ++itr)
        {
            UsbRegNode* pNode = &(*itr);
            logAdapter<<" "<<++devIndex<<".";
            logAdapter<<"ROOT=\""<<pNode->rootInfo<<"\"\n";
            list<UsbDevRegInfo>::iterator subItr;
            subDevIndex = 0;
            for(subItr = pNode->subInfo.begin();subItr != pNode->subInfo.end(); ++subItr)
            {
                UsbDevRegInfo* pInfo = &(*subItr);
                logAdapter<<"  "<<++subDevIndex<<".\n";
                logAdapter<<"  CLASS=\""<<pInfo->Class<<"\"\n"
                    <<"  EscapeCommand=\""<<pInfo->IsSupport<<"\"\n"
                    <<"  Service=\""<<pInfo->Service<<"\"\n"
                    <<"  GUID=\""<<pInfo->Guid<<"\"\n"
                    <<"  Info=\""<<pInfo->Info<<"\"\n"
                    <<"  MSG=\""<<pInfo->Mfg<<"\"\n"
                    <<"  RootKey=\""<<pInfo->RootKey<<"\"\n"
                    <<"  RegKey=\""<<pInfo->RegeditKey<<"\"\n";
            }
        }
    }
    /// 枚举所有CCID设备 
    static size_t EnumEscapeCommand(list<string>& rootKeys)
    {
        return EnumEscapeCommand("", "", rootKeys);
    }
    /// 枚举指定VID,PID下的所有子设备注册表路径值(Device Parameters的上一层)  
    static size_t EnumEscapeCommand(const char* sVid, const char* sPid, list<string>& rootKeys)
    {
        size_t count = 0;
        RegistryKey usbKey = RegistryKey::LocalMachine.OpenSubKey("SYSTEM\\CurrentControlSet\\Enum\\USB");
        list<string> subKeyNames;

        usbKey.GetSubKeyNames(subKeyNames);
        list<string>::iterator itr;
        for(itr = subKeyNames.begin();itr != subKeyNames.end(); ++itr)
        {
            ByteArray sTmp(itr->c_str(), itr->length());
            /* 过滤VID,PID */
            if(!StringConvert::StartWith(sTmp, "VID", true))
                continue;
            
            size_t indexFlag = StringConvert::IndexOf(sTmp, '&');
            if(!(StringConvert::Contains(sTmp.SubArray(0, indexFlag), sVid, true) &&
                StringConvert::Contains(sTmp.SubArray(indexFlag), sPid, true)))
            {
                continue;
            }
            
            list<string> subKeyNames;
            RegistryKey rootKey = usbKey.OpenSubKey(itr->c_str());

            rootKey.GetSubKeyNames(subKeyNames);
            list<string>::iterator subItr;
            /* 处理同一VID,PID下面的多个子设备 */
            for(subItr = subKeyNames.begin();subItr != subKeyNames.end(); ++subItr)
            {
                RegistryKey subKey = rootKey.OpenSubKey(subItr->c_str());
                list<RegValueItem> valItems;
                subKey.EnumValue(valItems);

                /* 过滤CCID设备 */
                list<RegValueItem>::iterator itrVal;
                for(itrVal = valItems.begin();itrVal != valItems.end(); ++itrVal)
                {
                    ByteBuilder tmp = (*itrVal).Name.c_str();
                    StringConvert::ToLower(tmp);

                    if(tmp == "classguid")
                    {
                        tmp.Clear();
                        tmp = (*itrVal).Item.ToString().c_str();

                        // 是CCID的设备则判断EscapeCommand的值 
                        if(StringConvert::Compare(tmp, PCSC_CCID_GUID, true))
                        {
                            ++count;
                            rootKeys.push_back(subKey.Name());
                            break;
                        }
                    }
                }
            }
        }

        return count;
    }
    /**
     * @brief 获取指定的键值是否支持EscapeCommand
     * @param [in] rootKey VID&PID下面第一层注册表(UsbDevRegInfo.RootKey) 
     * @param [out] pParamsName 注册表键值存在的路径,""则为没有注册表存在
     */ 
    static bool GetEscapeCommand(const char* rootKey, string* pParamsName = NULL)
    {
        string paramsName;
        bool bRet = _GetCCIDEscapeParams(RegistryKey::OpenKey(rootKey), paramsName);
        if(pParamsName != NULL)
            (*pParamsName) = paramsName;

        return bRet;        
    }
    /**
     * @brief 设置最底层路径下对EscapeCommand的值
     * @param [in] rootKey 枚举出的VID&PID下的注册表(UsbDevRegInfo.RootKey)  
     * @param [in] pIsEnable [default:NULL] 是否需要支持,为NULL表示直接删除  
     * @param [in] pParamsName [default:NULL] EscapeCommand值的注册表子路径(可通过GetEscapeCommand获得,为NULL自动获取) 
     */ 
    static bool UpdateEscapeCommand(const char* rootKey, bool* pIsEnable = NULL, const char* pParamsName = NULL)
    {
        string paramsName = "";
        bool bRet = false;
        RegistryKey key = RegistryKey::OpenKey(rootKey);
        if(!key.IsValid())
            return false;

        if(_is_empty_or_null(pParamsName))
        {
            bRet = _GetCCIDEscapeParams(key, paramsName);
            if(!bRet)
                return false;
            pParamsName = paramsName.c_str();
        }

        return _SetCCIDEscapeParams(key, pIsEnable, pParamsName);
    }
    /**
     * @brief 设置最底层路径下对EscapeCommand的值
     * @param [in] rootKey 枚举出的VID&PID下的注册表(UsbDevRegInfo.RootKey) 
     * @param [in] isEnable [default:true] 是否需要支持 
     * @param [in] pParamsName [default:NULL] EscapeCommand值的注册表子路径(可通过GetEscapeCommand获得,为NULL自动获取) 
     */ 
    static bool SetEscapeCommand(const char* rootKey, bool isEnable = false, const char* pParamsName = NULL)
    {
        return UpdateEscapeCommand(rootKey, &isEnable, pParamsName);
    }
    /**
     * @brief 删除最底层路径下对EscapeCommand的值
     * @param [in] rootKey 枚举出的VID&PID下的注册表(UsbDevRegInfo.RootKey) 
     * @param [in] pParamsName [default:NULL] EscapeCommand值的注册表子路径(可通过GetEscapeCommand获得,为NULL自动获取) 
     */ 
    static bool RemoveEscapeCommand(const char* rootKey, const char* pParamsName = NULL)
    {
        return UpdateEscapeCommand(rootKey, NULL, pParamsName);
    }
    /**
     * @brief 设置指定VID&PID下读卡器的EscapeCommand值  
     * @param [in] sVid 设备的VID
     * @param [in] sPid 设备的PID 
     * @param [in] enable [default:true] 需要设置的EscapeCommand值 
     * @param [out] pIsChanged [default:NULL] 是否设置过值
     */ 
    //----------------------------------------------------- 
    /* 对应用最直接的接口 */
    static bool SetEscapeCommandEnable(const char* sVid, const char* sPid, bool enable = true, bool* pIsChanged = NULL)
    {
        list<string> regKeys;
        if(WinCCID_EscapeCommandHelper::EnumEscapeCommand(sVid, sPid, regKeys) < 1)
            return false;

        list<string>::iterator itr;
        string paramer;
        bool is_support = false;
        bool bRet = true;
        for(itr = regKeys.begin();itr != regKeys.end(); ++itr)
        {
            // 对于值相同的不再修改 
            is_support = WinCCID_EscapeCommandHelper::GetEscapeCommand(itr->c_str(), &paramer); 
            if(is_support != enable)
            {
                // 只要有一个不成功则返回失败  
                if(!WinCCID_EscapeCommandHelper::SetEscapeCommand(itr->c_str(), enable, paramer.c_str()))
                {
                    bRet = false;
                }
                else
                {
                    if(pIsChanged != NULL)
                    {
                        (*pIsChanged) = true;
                    }
                }
            }
        }
        return bRet;
    }
    //----------------------------------------------------- 
    /// 枚举注册表下的键值(旧的函数) 
    static size_t EnumRegSubKey(HKEY hMainKey, list<string>& subkeys)
    {
        size_t count = 0;
        char_t itemName[_MAX_PATH]= {0};
        LONG lRet = 0;
        CharConverter cvt;

        for(int i = 0;; ++i)
        {
            lRet = RegEnumKey(hMainKey, i, itemName, sizeof(itemName));
            if(lRet == ERROR_NO_MORE_ITEMS)
                return count;
            subkeys.push_back();
            subkeys.back() = cvt.to_char(itemName);

            ++count;
        }

        return count;
    }
    /// CCID修改注册表EscapeCommand值(旧的函数) 
    static bool CCID_SetEscapeCommandEnable(const char* sVid, const char* sPid, bool enable = true, LoggerAdapter* plog = NULL)
    {
        /* 输出日志 */
        LOGGER(LoggerAdapter log;
        if(NULL != plog)
            log = (*plog));

        /* 枚举USB下面的注册项名称 */
        string path = "SYSTEM\\CurrentControlSet\\Enum\\USB";
        HKEY hKey = NULL;
        LOGGER(log<<"打开注册表:"<<path<<">\n");
        CharConverter cvt;
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cvt.to_char_t(path.c_str()), 0, KEY_READ, &hKey);
        if(lRet != ERROR_SUCCESS)
        {
            LOGGER(log<<"打开设备列表注册表<"<<path<<">失败\n");
            return false;
        }

        list<string> _list;
        list<string>::iterator itr;

        LOGGER(log.WriteLine("枚举子注册表..."));
        EnumRegSubKey(hKey, _list);
        RegCloseKey(hKey);

        /* 读取配置文件中的VID,PID */
        string svid;
        string spid;

        svid = sVid;
        spid = sPid;
        
        LOGGER(log<<"VID:<"<<svid<<">,PID:<"<<spid<<">\n");
        string tmpVidPid;
        list<string> _devList;
        for(itr = _list.begin();itr != _list.end(); ++itr)
        {
            tmpVidPid = itr->c_str();
            if(StringConvert::Contains(tmpVidPid.c_str(), svid.c_str(), true) != NULL &&
                StringConvert::Contains(tmpVidPid.c_str(), spid.c_str(), true) != NULL)
            {
                _devList.push_back();
                _devList.back() = *itr;

                LOGGER(log<<"找到匹配设备:<"<<(*itr)<<">\n");
            }
        }

        if(_devList.size() < 1)
        {
            LOGGER(log<<"在注册表中没有指定的VID<"<<svid<<">,PID<"<<spid<<">的设备\n");
            return false;
        }

        /* 设置查找到的VID,PID设备在注册表中的路径 */
        list<string>::iterator devItr;
        string tmpPath;

        DWORD dwValue = (enable ? 0x01 : 0x00);
        size_t errorCount = 0;
        CharConverter regCvt;
        const char_t* REG_EscapeCommandNamePtr = regCvt.to_char_t(REG_EscapeCommandName);

        string tmp;
        devItr = _devList.begin();
        while(devItr != _devList.end())
        {
            tmpPath = path;
            tmpPath += "\\";
            tmpPath += *devItr;

            ++devItr;
    
            /* 枚举注册表中的每一个子项 */
            hKey = NULL;
            LOGGER(log<<"打开注册表:<"<<tmpPath<<">\n");
            lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cvt.to_char_t(tmpPath.c_str()), 0, KEY_READ, &hKey);
            if(lRet != ERROR_SUCCESS)
            {
                LOGGER(log<<"打开注册表:<"<<tmpPath<<">失败\n");
                return false;
            }
    
            /* 枚举该设备下面的子设备 */
            _list.clear();
            EnumRegSubKey(hKey, _list);
            RegCloseKey(hKey);

            /* 为该VID,PID设备的所有子设备都添加上这一键值 */
            for(itr = _list.begin();itr != _list.end(); ++itr)
            {
                tmp = tmpPath;
                tmp += "\\";
                tmp += *itr;
                tmp += "\\Device Parameters";

                if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, cvt.to_char_t(tmp.c_str()), 
                    0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
                {
                    ++errorCount;
                    continue;
                }
                LOGGER(log<<"设置EscapeCommandEnable:<"<<dwValue<<">\n");
                if(RegSetValueEx(hKey, REG_EscapeCommandNamePtr,
                    0, REG_DWORD, reinterpret_cast<CONST BYTE*>(&dwValue), sizeof(DWORD)) != ERROR_SUCCESS)
                {
                    LOGGER(log<<(*itr)<<"写入注册表EscapeCommand失败"<<endl);
                }
                RegCloseKey(hKey);

                // Win7下WUDF EscapeCommand支持
                tmp += "\\WUDFUsbccidDriver";
                if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, cvt.to_char_t(tmp.c_str()), 
                    0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
                {
                    continue;
                }
                LOGGER(log<<"设置WUDFUsbccidDriver.EscapeCommandEnable:<"<<dwValue<<">\n");
                if(RegSetValueEx(hKey, _T("EscapeCommandEnable"), 
                    0, REG_DWORD, reinterpret_cast<CONST BYTE*>(&dwValue), sizeof(DWORD)) != ERROR_SUCCESS)
                {
                    LOGGER(log<<(*itr)<<"写入WUDF注册表EscapeCommandEnable失败"<<endl);
                }
                RegCloseKey(hKey);
            }
        }
        if(errorCount < 1)
        {
            LOGGER(log<<"EscapeCommand注册表键值写入成功\n");
            return true;
        }
        else if(errorCount >= _devList.size())
        {
            LOGGER(log.WriteLine("写入注册表失败"));
        }
        else
        {
            LOGGER(log.WriteLine("部分数据写入失败,请重新尝试修改"));
        }

        return false;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINCCID_ESCAPECOMMANDHELPER_H_
//========================================================= 