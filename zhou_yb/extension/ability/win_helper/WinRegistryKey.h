//========================================================= 
/**@file WinRegistryKey.h 
 * @brief Windows下类似C# RegistryKey类接口的辅助类 
 * 
 * @date 2013-09-04   21:53:24 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Container.h"
#include "../../../include/Base.h"

#include "../../../base_device/win32/WinHandler.h"
using zhou_yb::base_device::env_win32::WinLastErrBehavior;

#include <Winreg.h>

#pragma comment(lib,"Advapi32.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
// XP及其以下版本还没有 RegGetValue 函数 
#if WINVER <= 0x0501
#define RegGetValue SHRegGetValue
#endif
/// 操作注册表的默认权限(只读,可查询) 
#define KEY_DEFAULT_ACCESS (KEY_READ | KEY_WOW64_32KEY | KEY_QUERY_VALUE)
/// XP下 HKEY_CURRENT_USER_LOCAL_SETTINGS
#ifndef HKEY_CURRENT_USER_LOCAL_SETTINGS
#   define HKEY_CURRENT_USER_LOCAL_SETTINGS (( HKEY ) (ULONG_PTR)((LONG)0x80000007) )
#endif
//--------------------------------------------------------- 
/// 注册表值 
struct RegValue
{
    DWORD dwType;
    ByteBuilder Value;

    string_t ToString() const
    {
        CharConverter cvt;
        string_t sVal;
        if(Value.IsEmpty())
            return sVal;

        ByteBuilder val = Value;
        LPDWORD pDWORD = NULL;
        switch(dwType)
        {
        case REG_EXPAND_SZ:
        case REG_SZ:
            break;
        case REG_BINARY:
            val.Clear();
            ByteConvert::ToAscii(Value, val);
            break;
        case REG_DWORD_BIG_ENDIAN:
        case REG_DWORD:
            val.Clear();
            if(!Value.IsEmpty())
            {
                pDWORD = reinterpret_cast<LPDWORD>(const_cast<byte*>(Value.GetBuffer()));
                val.Format("%d", *(pDWORD));
            }
            else
            {
                val = "0";
            }
            break;
        case REG_MULTI_SZ:
        case REG_NONE:
        default:
            break;
        }
        sVal = cvt.to_char_t(val.GetString());
        return sVal;
    }
};
/// 注册表值项
struct RegValueItem
{
    string_t Name;
    RegValue Item;
};
//--------------------------------------------------------- 
/// 注册表项 
class RegistryKey : public LoggerBehavior
{
public:
    /* 系统中默认的注册表项 */
    static RegistryKey NullRegistryKey;
    static RegistryKey CurrentUser;
    static RegistryKey CurrentUserLocalSetting;
    static RegistryKey ClassesRoot;
    static RegistryKey LocalMachine;
    static RegistryKey Users;
    static RegistryKey CurrentConfig;
protected:
    //----------------------------------------------------- 
    /// 注册表句柄 
    shared_obj<HKEY> _hKey;
    /// 注册表的路径位置 
    shared_obj<string_t> _sName;
    //----------------------------------------------------- 
    // 内部的构造函数,用于作为返回值(不需要关闭注册表) 
    RegistryKey(const HKEY hKey, const shared_obj<string_t>& sName)
    {
        _hKey.obj() = hKey;
        _sName = sName;
    }
    RegistryKey(const HKEY hKey, const char_t* sName)
    {
        _hKey.obj() = hKey;
        _sName.obj() = sName;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 是否以指定的字符串开始 
    static bool StartWith(const char_t* src, size_t srclen, const char* substr, size_t sublen)
    {
        char c = 0;
        for(size_t i = 0; i < sublen && i < srclen; ++i)
        {
            c = src[i] & 0x0FF;
            if(_get_lower(substr[i]) != _get_lower(c))
                return false;
        }
        return true;
    }
    /// 打开完整的注册表路径 
    static RegistryKey OpenKey(const char_t* key, REGSAM regAccess = KEY_DEFAULT_ACCESS)
    {
        /* 查找输入的是否是系统项 */
        RegistryKey* KeyValue[] = 
        {
            &CurrentUser,
            &ClassesRoot,
            &LocalMachine,
            &Users,
            &CurrentConfig,
            &CurrentUserLocalSetting,
            &NullRegistryKey
        };
        const char KeyName[][40] = 
        {
            "HKEY_CURRENT_USER",
            "HKEY_CLASSES_ROOT",
            "HKEY_LOCAL_MACHINE",
            "HKEY_USERS",
            "HKEY_CURRENT_CONFIG",
            "HKEY_CURRENT_USER_LOCAL_SETTINGS",
            "NULL"
        };
        RegistryKey* pKey = &NullRegistryKey;
        bool rlt = false;
        int offset = 0;

        size_t keyLen = 0;
        size_t keyNameLen = 0;

        for(int i = 0;i < SizeOfArray(KeyValue); ++i)
        {
            keyLen = _strlen_t(key);
            keyNameLen = strlen(KeyName[i]);

            rlt = StartWith(key, keyLen, KeyName[i], keyNameLen);
            // 不匹配 
            if(!rlt)
                continue;
            
            pKey = KeyValue[i];
            // 完全匹配 
            if(keyLen == keyNameLen)
                return (*pKey);
            // 部分匹配 
            offset = strlen(KeyName[i]);
            if(key[offset] == '\\')
            {
                if(strlen_t(key + offset) < 1)
                    return (*pKey);
                return (*pKey).OpenSubKey(key + offset + 1, regAccess);
            }
        }
        return NullRegistryKey;
    }
    /// 打开注册表的父键 
    static RegistryKey OpenParentKey(const RegistryKey& reg, REGSAM regAccess = KEY_DEFAULT_ACCESS)
    {
        int index = -1;
        size_t namelen = reg.Name().length();
        const char_t* pName = reg.Name().c_str();
        for(size_t i = namelen - 1; i >= 0; --i)
        {
            if(pName[i] == static_cast<char_t>('\\'))
            {
                index = i;
                break;
            }
        }
        // 没有 "\"
        if(index < 0)
            return reg;

        string_t sName = reg.Name();
        sName[index] = 0;

        RegistryKey regKey = OpenKey(sName.c_str(), regAccess);
        sName[index] = '\\';

        return regKey;
    }
    //----------------------------------------------------- 
    RegistryKey()
    { 
        _hKey.obj() = NULL;
        _sName.obj() = _T("NULL");
    }
    RegistryKey(const RegistryKey& obj)
    {
        _hKey = obj._hKey;
        _sName = obj._sName;
    }
    virtual ~RegistryKey()
    {
        Close();
    }
    //--------------------------------------------------------- 
    /// 打开子项注册表 
    RegistryKey OpenSubKey(const char_t* key, REGSAM regAccess = KEY_DEFAULT_ACCESS)
    {
        LOG_FUNC_NAME();
        LOGGER(_log<<"父表:<"<<_sName.obj()<<">\n");
        LOGGER(
        _log << "打开子表:<" << _strput_t(key) << ">\n");

        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));

            return NullRegistryKey;
        }

        if(_is_empty_or_null_t(key))
        {
            LOGGER(_logRetValue(false));
            return NullRegistryKey;
        }
         
        HKEY hSubKey = NULL;
        if(ERROR_SUCCESS != RegOpenKeyEx(_hKey, key, 0, regAccess, &hSubKey))
        {
            LOGGER(WinLastErrBehavior lastErr;
            _log << "打开注册表失败,错误码:<" << lastErr.GetLastErr() << "," << lastErr.GetErrMessage() << ">\n");
            LOGGER(_logRetValue(false));
            return NullRegistryKey;
        }
        LOGGER(_logRetValue(true));

        shared_obj<string_t> subName;
        subName.obj() = _sName.obj();
        subName.obj() += _T("\\");
        subName.obj() += key;

        return RegistryKey(hSubKey, subName);
    }
    /// 创建子项 
    RegistryKey CreateSubKey(const char_t* subKeyName, bool* pIsExist = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log<<"父表:<"<<_sName.obj()<<">\n");
        LOGGER(_log<<"创建子表:<"<<_strput_t(subKeyName)<<">\n");

        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));

            return NullRegistryKey;
        }

        if(_is_empty_or_null_t(subKeyName))
        {
            LOGGER(_logRetValue(false));
            return NullRegistryKey;
        }

        HKEY hSubKey = NULL;
        DWORD dwDisposition = 0;
        if(ERROR_SUCCESS != RegCreateKeyEx(_hKey, subKeyName, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisposition))
        {
            LOGGER(WinLastErrBehavior lastErr;
            _log << "创建注册表失败,错误码:<" << lastErr.GetLastErr() << "," << lastErr.GetErrMessage() << ">\n");
            LOGGER(_logRetValue(false));
            return NullRegistryKey;
        }
        LOGGER(_log<<"是否创建:<"<<(dwDisposition==REG_CREATED_NEW_KEY)<<">\n");
        LOGGER(_logRetValue(true));

        if(pIsExist != NULL)
        {
            // REG_CREATED_NEW_KEY
            (*pIsExist) = (dwDisposition == REG_OPENED_EXISTING_KEY);
        }

        shared_obj<string_t> subName(_sName);
        subName.obj() += _T("\\");
        subName.obj() += subKeyName;

        return RegistryKey(hSubKey, subName);
    }
    /// 删除子项,""则删除本身 
    bool DeleteSubKey(const char_t* subKeyName = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log<<"父表:<"<<_sName.obj()<<">\n");
        LOGGER(_log<<"删除子表:<"<<_strput_t(subKeyName)<<">\n");

        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));
            return false;
        }

        if(ERROR_SUCCESS != RegDeleteKey(_hKey, subKeyName))
        {
            LOGGER(WinLastErrBehavior lastErr;
            _log << "删除子表失败,错误码:<" << lastErr.GetLastErr() << "," << lastErr.GetErrMessage() << ">\n");
            LOGGER(_logRetValue(false));
            return false;
        }

        LOGGER(_logRetValue(true));
        return true;
    }
    /// 删除值,为""表示删除默认值  
    bool DeleteValue(const char_t* valName)
    {
        LOG_FUNC_NAME();
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));
            return false;
        }

        LONG lRet = RegDeleteValue(_hKey, valName);

        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 删除该注册表的所有键(递归删除所有子键),只删除部分也返回false 
    bool Delete()
    {
        LOG_FUNC_NAME();
        // 底层没有键 
        list<string_t> subKeyNames;
        if(GetSubKeyNames(subKeyNames) < 1)
        {
            DeleteValue(_T(""));
            LOGGER(_logRetValue(true));
            return true;
        }

        bool isDeleteAll = false;
        list<string_t>::iterator itr;
        for(itr = subKeyNames.begin();itr != subKeyNames.end(); ++itr)
        {
            RegistryKey subKey = OpenSubKey(itr->c_str());
            isDeleteAll = subKey.Delete();
        }

        LOGGER(_logRetValue(isDeleteAll));
        return isDeleteAll;
    }
    /// 获取子项的键值名称 
    size_t GetSubKeyNames(list<string_t>& subNames)
    {
        LOG_FUNC_NAME();
        size_t count = 0;

        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(count));
            return count;
        }
        
        char_t itemName[_MAX_PATH]= {0};
        LONG lRet = 0;

        for(int i = 0;; ++i)
        {
            lRet = RegEnumKey(_hKey, i, itemName, sizeof(itemName));
            LOGGER(_log<<"Ret:<"<<lRet<<">\n");

            if(lRet == ERROR_NO_MORE_ITEMS)
                break;
            subNames.push_back(string_t());
            subNames.back() = itemName;

            LOGGER(_log<<"Name:<"<<itemName<<">\n");

            ++count;
        }

        LOGGER(_logRetValue(count));
        return count;
    }
    /// 获取该项的数据值名称 
    size_t EnumValue(list<RegValueItem>& vals)
    {
        LOG_FUNC_NAME();
        size_t count = 0;

        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(count));
            return count;
        }
        
        char_t sName[_MAX_PATH];
        BYTE cdData[8];
        DWORD dwNameSize = _MAX_PATH;
        DWORD dwSize = 0;
        LONG lRet = 0;

        for(int i = 0;; ++i)
        {
            LOGGER(_log<<"Enum:<"<<i<<"> ");

            dwNameSize = _MAX_PATH;
            dwSize = 0;

            lRet = RegEnumValue(_hKey, i, sName, &dwNameSize, 0, 
                NULL, cdData, &dwSize);

            vals.push_back(RegValueItem());
            RegValueItem& item = vals.back();
            item.Item.Value.Append(static_cast<byte>(0x00), dwSize);

            lRet = RegEnumValue(_hKey, i, sName, &dwNameSize, 0, 
                &(item.Item.dwType), reinterpret_cast<LPBYTE>(const_cast<byte*>(item.Item.Value.GetBuffer())), &dwSize);

            if(lRet != ERROR_SUCCESS)
            {
                vals.pop_back();
                break;
            }

            item.Name = sName;

            LOGGER(
            _log<<"Name:<"<<sName<<">,Type:<"<<_hex(item.Item.dwType)<<">\n"
                <<"Value:<"<<item.Item.Value<<endl);

            ++count;
        }

        LOGGER(_logRetValue(count));
        return count;
    }
    /// 获取子键的值 
    bool GetSubValue(const char_t* subKey, const char_t* valName, RegValue& val)
    {
        LOG_FUNC_NAME();

        BYTE cbData[8];
        DWORD dwSize = 0;
        // 先查询出空间 
        LONG lRet = RegGetValue(_hKey, subKey, valName, RRF_RT_ANY, &(val.dwType), cbData, &dwSize);

        val.Value.Resize(dwSize);
        val.Value.Append(static_cast<byte>(0x00), dwSize);
        lRet = RegGetValue(_hKey, subKey, valName, RRF_RT_ANY, &(val.dwType), 
            reinterpret_cast<LPVOID>(const_cast<byte*>(val.Value.GetBuffer())), &dwSize);

        if(lRet != ERROR_SUCCESS)
            val.Value.Clear();

        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 获取子键的值 
    bool GetSubValue(const char_t* subKey, const char_t* valName, DWORD* pdwType, PVOID pData, DWORD* pDataLen)
    {
        LOG_FUNC_NAME();
        
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));
            return false;
        }

        LONG lRet = RegGetValue(_hKey, subKey, valName, RRF_RT_ANY, pdwType, pData, pDataLen);

        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 获取注册表值 
    bool GetValue(const char_t* valName, RegValue& val)
    {
        LOG_FUNC_NAME();

        DWORD dwSize = 0;
        // 先查询出空间 
        LONG lRet = RegQueryValueEx(_hKey, valName, 0, NULL, NULL, &dwSize);
        if(lRet != ERROR_SUCCESS)
        {
            LOGGER(_logRetValue(false));
            return false;
        }

        val.Value.Resize(dwSize);
        val.Value.Append(static_cast<byte>(0x00), dwSize);
        lRet = RegQueryValueEx(_hKey, valName, 0, &(val.dwType), 
            reinterpret_cast<LPBYTE>(const_cast<byte*>(val.Value.GetBuffer())), &dwSize);

        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 获取注册表值 
    bool GetValue(const char_t* valName, DWORD* pdwType, LPBYTE pData, DWORD* pDataLen)
    {
        LOG_FUNC_NAME();
        
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));
            return false;
        }

        LONG lRet = RegQueryValueEx(_hKey, valName, 0, pdwType, pData, pDataLen);

        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 设置注册表值 
    bool SetValue(const char_t* valName, const RegValue& val)
    {
        LOG_FUNC_NAME();
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));
            return false;
        }

        LONG lRet = RegSetValueEx(_hKey, valName, 0, val.dwType, 
            reinterpret_cast<LPBYTE>(const_cast<byte*>(val.Value.GetBuffer())), static_cast<DWORD>(val.Value.GetLength()));
        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 设置注册表值 
    bool SetValue(const RegValueItem& item)
    {
        const char_t* sName = item.Name.c_str();
        if(item.Name.length() < 1)
            sName = _T("");
        return SetValue(sName, item.Item);
    }
    /// 设置注册表值 
    bool SetValue(const char_t* valName, DWORD dwType, PVOID val, DWORD dwSize)
    {
        LOG_FUNC_NAME();
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("当前注册表无效"));
            LOGGER(_logRetValue(false));
            return false;
        }

        LONG lRet = RegSetValueEx(_hKey, valName, 0, dwType, 
            reinterpret_cast<const BYTE*>(val), dwSize);
        LOGGER(_logRetValue(lRet == ERROR_SUCCESS));
        return lRet == ERROR_SUCCESS;
    }
    /// 只读句柄 
    inline HKEY Key() const
    {
        return _hKey;
    }
    /// 只读注册表路径  
    inline const string_t& Name() const
    {
        return _sName;
    }
    /// 返回当前注册表项是否有效 
    bool IsValid() const 
    {
        return (NULL != _hKey);
    }
    /// 关闭注册表 
    void Close()
    {
        LOG_FUNC_NAME();
        // 只有该对象持有_hKey
        if(_hKey.ref_count() < 2)
        {
            if(IsValid())
            {
                LOGGER(_log.WriteLine("Close Key."));
                RegCloseKey(_hKey);
            }
        }
        _hKey.reset() = NULL;
    }
    /// 比较操作符 
    bool operator ==(const RegistryKey& other)
    {
        return _hKey.obj() == other._hKey.obj();
    }
    bool operator !=(const RegistryKey& other)
    {
        return !(operator ==(other));
    }
    /// 相同类型对象之间的引用赋值 
    RegistryKey& operator =(const RegistryKey& other)
    {
        // 先关闭当前的 
        if(this != &other)
        {
            Close();

            _hKey = other._hKey;
            _sName = other._sName;
        }

        return (*this);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension 
} // namespace zhou_yb
//========================================================= 