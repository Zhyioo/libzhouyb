//========================================================= 
/**@file WmiInvoker.h 
 * @brief WMI封装 
 * 
 * @date 2013-12-30   19:57:01 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WMIINVOKER_H_
#define _LIBZHOUYB_WMIINVOKER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include "../../../container/container_helper.h"
using zhou_yb::container::list_helper;

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <atlbase.h>
#include <atlcomcli.h>

#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "wbemuuid.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// WMI属性 
struct WmiProperty
{
    /// 空的属性 
    static WmiProperty NullWmiProperty;

    /// 属性名称 
    string Name;
    /// 属性值类型 
    CIMTYPE Type;
    /// 属性特性 
    long Flavor;
    /// 属性值  
    CComVariant Value;
};
/// WMI对象 
struct WmiObject
{
    string ClsName;
    list<WmiProperty> Properties;

    const WmiProperty& operator[](const char* name)
    {
        list<WmiProperty>::iterator itr;
        for(itr = Properties.begin();itr != Properties.end(); ++itr)
        {
            if(itr->Name == name)
                return (*itr);
        }
        return WmiProperty::NullWmiProperty;
    }
};
//--------------------------------------------------------- 
/// WMI辅助类 
class WmiHelper
{
protected:
    WmiHelper() {}
public:
    /// 命名空间名
    static char WMI_SPACE_NAME[][32];
    /// 服务属性连接名
    static char WMI_SERVICE_NAME[][64];
    /// WMI支持的命名空间(将'_'转成'\'转成字符串即为对应的参数namespace) 
    enum NamespaceKey
    {
        /**
         * @brief Active Directory提供程序 
         * dsprov.dll
         * 将Active Directory 对象映射到 WMI
         * 
         * "ROOT\\DIRECTORY\\LDAP"
         */
        DSPROV = 0,
        /**
         * @brief 事件日志提供程序 
         * ntevt.dll
         * 管理 Windows 事件日志，例如，读取、备份、清除、复制、删除、监视、重命名、压缩、解压缩和更改事件日志设置
         * 
         * "ROOT\\CIMV2"
         */
        NTEVT,
        /**
         * @brief 注册表提供程序 
         * stdprov.dll
         * 读取、写入、枚举、监视、创建、删除注册表项和值
         * 
         * "ROOT\\DEFAULT"
         */
        STDPROV,
        /**
         * @brief Win32 提供程序 
         * cimwin32.dll
         * 提供关于计算机、磁盘、外围设备、文件、文件夹、文件系统、网络组件、操作系统、打印机、进程、安全性、服务、共享、SAM 用户及组，以及更多资源的信息
         * 
         * "ROOT\\CIMV2"
         */ 
         CIMWIN32,
         /**
          * @brief Windows 安装程序提供程序
          * msiprov.dll
          * 提供对已安装软件信息的访问
          * 
          * "ROOT\\CIMV2"
          */ 
        MSIPROV,
        /* 枚举的最大值 */
        MAX_NamespaceKey
    };
    /// WMI支持的列表  
    enum ServiceKey
    {
        /* 硬件 */
        /// CPU 处理器
        Win32_Processor, 
        /// 物理内存条
        Win32_PhysicalMemory, 
        /// 键盘
        Win32_Keyboard, 
        /// 点输入设备(包括鼠标)
        Win32_PointingDevice, 
        /// 软盘驱动器
        Win32_FloppyDrive, 
        /// 硬盘驱动器
        Win32_DiskDrive, 
        /// 光盘驱动器
        Win32_CDROMDrive,
        /// 主板
        Win32_BaseBoard, 
        /// BIOS芯片
        Win32_BIOS, 
        /// 并口
        Win32_ParallelPort, 
        /// 串口
        Win32_SerialPort, 
        /// 串口配置
        Win32_SerialPortConfiguration, 
        /// 多媒体设置，一般指声卡
        Win32_SoundDevice,
        /// 主板插槽 (ISA & PCI & AGP)
        Win32_SystemSlot, 
        /// USB控制器
        Win32_USBController, 
        /// 网络适配器
        Win32_NetworkAdapter, 
        /// 网络适配器设置
        Win32_NetworkAdapterConfiguration, 
        /// 打印机
        Win32_Printer, 
        /// 打印机设置
        Win32_PrinterConfiguration, 
        /// 打印机任务
        Win32_PrintJob, 
        /// 打印机端口
        Win32_TCPIPPrinterPort, 
        /// MODEM
        Win32_POTSModem, 
        /// MODEM 端口
        Win32_POTSModemToSerialPort, 
        /// 显示器
        Win32_DesktopMonitor, 
        /// 显卡
        Win32_DisplayConfiguration, 
        /// 显卡设置
        Win32_DisplayControllerConfiguration, 
        /// 显卡细节
        Win32_VideoController, 
        /// 显卡支持的显示模式。
        Win32_VideoSettings, 

        // 操作系统
        /// 时区
        Win32_TimeZone, 
        /// 驱动程序
        Win32_SystemDriver, 
        /// 磁盘分区
        Win32_DiskPartition, 
        /// 逻辑磁盘
        Win32_LogicalDisk, 
        /// 逻辑磁盘所在分区及始末位置。
        Win32_LogicalDiskToPartition, 
        /// 逻辑内存配置
        Win32_LogicalMemoryConfiguration, 
        /// 系统页文件信息
        Win32_PageFile, 
        /// 页文件设置
        Win32_PageFileSetting, 
        /// 系统启动配置
        Win32_BootConfiguration, 
        /// 计算机信息简要
        Win32_ComputerSystem, 
        /// 操作系统信息
        Win32_OperatingSystem, 
        /// 系统自动启动程序
        Win32_StartupCommand, 
        /// 系统安装的服务
        Win32_Service, 
        /// 系统管理组
        Win32_Group, 
        /// 系统组帐号
        Win32_GroupUser, 
        /// 用户帐号
        Win32_UserAccount, 
        /// 系统进程
        Win32_Process, 
        /// 系统线程
        Win32_Thread, 
        /// 共享
        Win32_Share, 
        /// 已安装的网络客户端
        Win32_NetworkClient, 
        /// 已安装的网络协议
        Win32_NetworkProtocol, 

        /* 枚举的最大值 */
        MAX_ServiceKey
    };
    /// 将对应的命名空间转换成对应的空间名 
    static const char* NamespaceToString(NamespaceKey key)
    {
        int keyIndex = static_cast<int>(key);
        if(keyIndex >= MAX_NamespaceKey)
            keyIndex = MAX_NamespaceKey;
        return WMI_SPACE_NAME[keyIndex];
    }
    /// 将对应的路径名转成字符串  
    static const char* ServiceToString(ServiceKey key)
    {
        int keyIndex = static_cast<int>(key);
        if(keyIndex >= MAX_ServiceKey)
            keyIndex = MAX_ServiceKey;
        return WMI_SERVICE_NAME[keyIndex];
    }
    /// 将CComVariant转为字符转
    static string VariantToString(const CComVariant& val)
    {
        string sVal = "";
        char buff[128] = {0};
        char* pStr = NULL;

        switch (val.vt)
        {
        case VT_BSTR:
            pStr = _com_util::ConvertBSTRToString(val.bstrVal);
            sVal = pStr;
            free(pStr);
            break;
        case VT_NULL:
            sVal = "NULL";
            break;
        case VT_I1:
        case VT_I2:
        case VT_I4:
        case VT_I8:
        case VT_INT:
            sprintf(buff, "%d", static_cast<int>(val.intVal));
            sVal = buff;
            break; 
        case VT_UI8:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_UINT:
            sprintf(buff, "%u", static_cast<uint>(val.uintVal));
            sVal = buff;
            break;
        case VT_BOOL:
            sVal = (val.boolVal ? "True" : "False");
            break;
        case VT_UNKNOWN:
            sVal = "IUnknown";
            break;
        }
        return sVal;
    }
};
//--------------------------------------------------------- 
/// WMI接口操作对象 
class WmiInvoker : public LoggerBehavior
{
    /// 禁用拷贝构造函数 
    UnenableObjConsturctCopyBehavior(WmiInvoker);
protected:
    /// 对象引用计数  
    static size_t objCount;
    /// 是否成功初始化相关组件 
    static bool hasInitCOM;
    /// WMI服务对象 
    static IWbemLocator* pWmiLocator;
    /// 初始化WMI资源 
    static bool _Initialize()
    {
        if(hasInitCOM)
            return true;

        hasInitCOM = false;
        pWmiLocator = NULL;

        HRESULT hres = S_FALSE;
        // 初始化COM组件 
        hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
        if(FAILED(hres))
            return false;
        
        // 设置安全属性 
        hres =  CoInitializeSecurity(
            NULL, 
            -1,                          // COM authentication
            NULL,                        // Authentication services
            NULL,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
            NULL,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities 
            NULL                         // Reserved
            );
        if(FAILED(hres))
        {
            CoUninitialize();
            return false;
        }

        // 连接WMI服务 
        hres = CoCreateInstance(
            CLSID_WbemLocator,             
            0, 
            CLSCTX_INPROC_SERVER, 
            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pWmiLocator));

        if (FAILED(hres))
        {
            pWmiLocator = NULL;
            CoUninitialize();
            return false;
        }

        hasInitCOM = true;
        return true;
    }
    /// 释放所暂用的WMI资源 
    static void _Uninitialize()
    {
        if(pWmiLocator != NULL)
        {
            pWmiLocator->Release();
        }
        pWmiLocator = NULL;

        if(hasInitCOM)
        {
            CoUninitialize();
        }
        hasInitCOM = false;
    }
protected:
    /// 是否已经连接到服务 
    bool _hasConnect;
    /// 连接的WMI  
    IWbemServices* _pWmiSvr;
    /// 查询到的对象 
    list<IWbemClassObject*> _clsObjList;
    /// 初始化数据 
    inline void _init()
    {
        ++objCount;
        // 第一个对象 
        if(objCount < 2)
            _Initialize();

        _hasConnect = false;
        _pWmiSvr = NULL;
        _clsObjList.clear();
    }
    /// 获取对象列表 
    size_t _getIWbemClassObject(IEnumWbemClassObject* pEnumerator, list<IWbemClassObject*>& objList)
    {
        LOGGER(_log.WriteLine("Enumerator To IWbemClassObjectList..."));

        ULONG uReturn = 0;
        size_t count = 0;
        while(pEnumerator)
        {
            IWbemClassObject* pClsObj = NULL;
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn);
            if(0 == uReturn)
                break;
            ++count;

            if(SUCCEEDED(hr))
            {
                objList.push_back(pClsObj);
            }
        }
        LOGGER(_log<<"IWbemClassObject Count:<"<<count<<">\n");
        return count;
    }
    /// 获取属性 
    bool _get(IWbemClassObject* pClsObj, WmiProperty& val)
    {
        LOGGER(_log<<"Property:<"<<val.Name<<">\n");

        CComBSTR sName;
        sName = val.Name.c_str();

        HRESULT hr = pClsObj->Get(sName, 0, &val.Value, &val.Type, &val.Flavor);
        LOGGER(
        if(SUCCEEDED(hr))
            _log<<"Value:<"<<WmiHelper::VariantToString(val.Value)<<">,Type:<"<<_hex(val.Type)<<">\n";
        else
            _log.WriteLine("Get Failed..."));

        return SUCCEEDED(hr);
    }
    /// 设置属性 
    bool _put(IWbemClassObject* pClsObj, WmiProperty& val)
    {
        LOGGER(_log<<"Property:<"<<val.Name<<">\n");

        CComBSTR sName;
        sName = val.Name.c_str();

        HRESULT hr = pClsObj->Put(sName, 0, &val.Value, val.Type);
        LOGGER(
        if(SUCCEEDED(hr))
            _log<<"Value:<"<<WmiHelper::VariantToString(val.Value)<<">,Type:<"<<_hex(val.Type)<<">\n";
        else
            _log.WriteLine("Put Failed..."));

        return SUCCEEDED(hr);
    }
    /// 清空链表 
    void _clear_list(list<IWbemClassObject*>& _list)
    {
        list<IWbemClassObject*>::iterator itr;
        for(itr = _list.begin();itr != _list.end(); ++itr)
            (*itr)->Release();
    }
public:
    WmiInvoker()
    {
        _init();
    }
    WmiInvoker(const char* spaceName)
    {
        _init();
        Connect(spaceName);
    }
    virtual ~WmiInvoker()
    {
        Release();

        --objCount;
        // 最后一个对象负责释放COM资源  
        if(objCount < 1)
        {
            _Uninitialize();
        }
    }
    /// 连接服务 
    bool Connect(const char* spaceName)
    {
        LOG_FUNC_NAME();
        LOGGER(_log<<"Connect To Server:<"<<_strput(spaceName)<<">...\n");
        // 参数完整性校验 
        if(_is_empty_or_null(spaceName))
            return false;
        // 处理WMI服务是否有效 
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("Restart WMI COM..."));
            // 重启 
            _Uninitialize();
            _Initialize();
        }
        if(!IsValid())
        {
            LOGGER(_log.WriteLine("Init WMI COM Failed ..."));
            return false;
        }
        /// 已经连接过另外一个命名空间则断开连接  
        Release();
        // 连接命名空间  
        LOGGER(_log.WriteLine("ConnectServer..."));
        HRESULT hres = pWmiLocator->ConnectServer(
            _com_util::ConvertStringToBSTR(spaceName), // Object path of WMI namespace
            NULL,                    // User name. NULL = current user
            NULL,                    // User password. NULL = current
            0,                       // Locale. NULL indicates current
            NULL,                    // Security flags.
            0,                       // Authority (e.g. Kerberos)
            0,                       // Context object 
            &_pWmiSvr                // pointer to IWbemServices proxy
            );
        if(FAILED(hres))
        {
            LOGGER(_log.WriteLine("ConnectServer Failed..."));
            return false;
        }

        LOGGER(_log.WriteLine("CoSetProxyBlanket..."));
        hres = CoSetProxyBlanket(
            _pWmiSvr,                    // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
            NULL,                        // Server principal name 
            RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
            NULL,                        // client identity
            EOAC_NONE                    // proxy capabilities 
            );
        if(FAILED(hres))
        {
            LOGGER(_log.WriteLine("CoSetProxyBlanket Failed..."));
            Release();
            return false;
        }
        LOGGER(_log.WriteLine("Connect To Server Successed ."));
        return true;
    }
    /// 查询数据 
    bool ExecQuery(const char* wql)
    {
        LOG_FUNC_NAME();
        LOGGER(_log<<"WQL:<"<<_strput(wql)<<">...\n");

        if(!CanQuery() || _is_empty_or_null(wql))
            return false;

        _clear_list(_clsObjList);
        _clsObjList.clear();

        CComBSTR queryStr;
        queryStr = wql;

        LOGGER(_log.WriteLine("ExecQuery..."));
        IEnumWbemClassObject* pEnumerator = NULL;
        HRESULT hres = _pWmiSvr->ExecQuery(
            bstr_t("WQL"), 
            queryStr,
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator);

        if (FAILED(hres))
        {
            LOGGER(_log.WriteLine("ExecQuery Failed..."));
            return false;
        }

        _getIWbemClassObject(pEnumerator, _clsObjList);
        pEnumerator->Release();

        return true;
    }
    bool ExecQuery(const char* spaceName, const char* wql)
    {
        if(!Connect(spaceName))
            return false;
        return ExecQuery(wql);
    }
    /// 获取所有属性值 
    bool Get(const char* propertyName, list<CComVariant>& propertyVals)
    {
        LOG_FUNC_NAME();

        if(_is_empty_or_null(propertyName))
            return false;
        bool isGet = false;
        list<IWbemClassObject*>::iterator itr;
        WmiProperty val;
        val.Name = propertyName;

        for(itr = _clsObjList.begin();itr != _clsObjList.end(); ++itr)
        {
            if(_get(*itr, val) && val.Value.vt != VT_NULL)
            {
                propertyVals.push_back(val.Value);
                isGet = true;
            }
        }
        return isGet;
    }
    /// 获取第一个属性值 
    bool Get(const char* propertyName, CComVariant& val)
    {
        LOG_FUNC_NAME();

        if(_is_empty_or_null(propertyName))
            return false;

        bool isGet = false;
        list<IWbemClassObject*>::iterator itr;
        WmiProperty wmiVal;
        wmiVal.Name = propertyName;

        for(itr = _clsObjList.begin();itr != _clsObjList.end(); ++itr)
        {
            if(_get(*itr, wmiVal))
            {
                val = wmiVal.Value;
                isGet = true;
                break;
            }
        }
        return isGet;
    }
    /// 获取第一个属性值的字符串表示形式,获取失败则返回"" 
    string operator [](const char* propertyName)
    {
        CComVariant val;
        if(Get(propertyName, val))
            return WmiHelper::VariantToString(val);
        return "";
    }
    /// 获取所有WQL查询内容 
    size_t EnumObjects(list<WmiObject>& objs)
    {
        LOG_FUNC_NAME();

        size_t objCount = 0;
        list<IWbemClassObject*>::iterator itr;
        for(itr = _clsObjList.begin();itr != _clsObjList.end(); ++itr)
        {
            LOGGER(_log<<"Get CLASS, Index=["<<(objCount)<<"]\n");

            CComVariant vtProp;
            HRESULT hr = (*itr)->Get(L"__CLASS", 0, &vtProp, NULL, NULL);
            if(FAILED(hr))
            {
                LOGGER(_log.WriteLine("Failed..."));
                continue;
            }

            ++objCount;
            objs.push_back(WmiObject());
            WmiObject& newObj = objs.back();

            newObj.ClsName = WmiHelper::VariantToString(vtProp);
            LOGGER(_log.WriteLine(newObj.ClsName));

            hr = (*itr)->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);
            do
            {
                CComBSTR objName;
                CComVariant objVal;
                CIMTYPE objType;
                long lFlavor = 0;

                hr = (*itr)->Next(0, &objName, &objVal, &objType, &lFlavor);
                // 枚举完成或者属性名称为空则结束 
                if(FAILED(hr) || objName.Length() < 1)
                    break;

                newObj.Properties.push_back(WmiProperty());
                WmiProperty& newObjProerty = newObj.Properties.back();

                newObjProerty.Name = WmiHelper::VariantToString(objName);
                hr = (*itr)->Get(objName, 0, &(newObjProerty.Value), &(newObjProerty.Type), &(newObjProerty.Flavor));

                LOGGER(
                if(SUCCEEDED(hr))
                    _log<<" "<<newObjProerty.Name<<"=\""
                        <<WmiHelper::VariantToString(newObjProerty.Value)<<"\"\n");
            } while (SUCCEEDED(hr));
            hr = (*itr)->EndEnumeration();
        }

        return objCount;
    }
    /// 获取原始的IWbemClassObject
    IWbemClassObject* GetClassObject(size_t index)
    {
        if(index >= _clsObjList.size())
            return NULL;
        return *(list_helper<IWbemClassObject*>::index_of(_clsObjList, index));
    }
    /// 释放资源 
    void Release()
    {
        _clear_list(_clsObjList);
        _clsObjList.clear();

        if(_pWmiSvr != NULL)
            _pWmiSvr->Release();
        _pWmiSvr = NULL;
    }
    /// 是否可用于连接 
    inline bool IsValid() const
    {
        return (hasInitCOM && pWmiLocator != NULL);
    }
    /// 是否可用于查询 
    inline bool CanQuery() const
    {
        return (_pWmiSvr != NULL);
    }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WMIINVOKER_H_
//========================================================= 