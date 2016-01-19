//========================================================= 
/**@file BluetoothDevice.h 
 * @brief Windows下蓝牙设备操作逻辑 
 * 
 * 时间: 2014-10-10   19:07 
 * 作者: Zhyioo  
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "SocketDevice.h"

#include <BluetoothAPIs.h>
#include <Ws2bth.h>
#pragma comment(lib, "Bthprops.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_win32 {
//--------------------------------------------------------- 
/// 蓝牙设备的一些信息 
struct BluetoothInfo
{
    /// 蓝牙名称 
    string Name;
    /// 蓝牙地址 
    BTH_ADDR Address;
    /// 句柄 
    HANDLE hRadio;
    /// 是否允许被发现 
    bool IsDiscoverable;
    /// 是否配对 
    bool IsAuthenticated;
    /// 是否连接 
    bool IsConnected;
    /// 是否保存 
    bool IsRemembered;
};
//--------------------------------------------------------- 
/// 蓝牙设备配置参数(蓝牙类为模板类,无法初始化单独提取出来共用)
struct BluetoothParam
{
    //----------------------------------------------------- 
    /// BluetoothDevice生成参数配置项时的主键 RemoteNameKey
    static const char RemoteNameKey[16];
    /// BluetoothDevice生成参数配置项时的主键 PasswordKey
    static const char PasswordKey[16];
    //----------------------------------------------------- 
    /// 将数据转换为配置参数 
    static string ToConfig(const char* remoteName, const char* password)
    {
        string tmp = "";
        tmp += ArgConvert::ToConfig<const char*>(RemoteNameKey, _strput(remoteName));
        tmp += ArgConvert::ToConfig<const char*>(PasswordKey, _strput(password));

        return tmp;
    }
    /// 从配置参数中解析 
    static void FromConfig(const ArgParser& cfg, string& remoteName, string& pwd)
    {
        ArgConvert::FromConfig<string>(cfg, RemoteNameKey, remoteName);
        ArgConvert::FromConfig<string>(cfg, PasswordKey, pwd);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 蓝牙操作扩展器 
template<class TSocketDevice>
class BluetoothHandlerAppender : public TSocketDevice
{
public:
    /// 蓝牙设备类型 
    typedef BluetoothInfo device_info;
    //----------------------------------------------------- 
    /// 将地址转换为字符串 
    static string AddressToString(BTH_ADDR& adr)
    {
        string sAdr;
        char buff[8] = { 0 };
        BLUETOOTH_ADDRESS address;
        address.ullLong = adr;
        for(int i = sizeof(address.rgBytes) - 1;i >= 0; --i)
        {
            sprintf(buff, "%02X:", address.rgBytes[i]);
            sAdr += buff;
        }
        sAdr.pop_back();

        return sAdr;
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 地址 
    BTH_ADDR _bthAdr;
    /// 配对密码
    string _bthPwd;
    /// 本地句柄
    HANDLE _hLocalRadio;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 枚举本地蓝牙设备
     *
     * @param [out] devlist 枚举到的本地设备
     * @param [in] isFindFirst [default:false] 是否只查找第一个
     *
     * @return size_t 枚举到的设备数目
     */
    size_t EnumLocalDevice(list<device_info>& devlist, bool isFindFirst = false)
    {
        LOG_FUNC_NAME();

        HANDLE hRadio = NULL;
        BLUETOOTH_FIND_RADIO_PARAMS bfrp = { sizeof(bfrp) };
        HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&bfrp, &hRadio);
        BLUETOOTH_RADIO_INFO bri = { sizeof(BLUETOOTH_RADIO_INFO) };

        size_t count = 0;
        CharConvert convert;
        if(!hFind)
        {
            _logErr(DeviceError::OperatorNoSupportErr, "BluetoothFindFirstRadio失败");
            return _logRetValue(count);
        }

        do{
            if(hRadio && BluetoothGetRadioInfo(hRadio, &bri) == ERROR_SUCCESS)
            {
                devlist.push_back(device_info());
                ++count;

                device_info& dev = devlist.back();

                dev.hRadio = hRadio;

                dev.Name = convert.to_char(bri.szName);
                dev.Address = bri.address.ullLong;
                dev.IsDiscoverable = Tobool(BluetoothIsDiscoverable(hRadio));

                LOGGER(_log << "Find Device:<" << count << ">\n";
                _log << "Name:<" << dev.Name << ">\n"
                    << "Address:<" << AddressToString(dev.Address).c_str() << ">\n"
                    << "IsDiscoverable:<" << dev.IsDiscoverable << ">\n");

                if(isFindFirst) break;
            }
        } while(BluetoothFindNextRadio(hFind, &hRadio));
        BluetoothFindRadioClose(hFind);

        return _logRetValue(count);
    }
    /**
     * @brief 枚举本机外的其他蓝牙设备
     * 
     * @param [out] devlist 枚举到的设备列表 
     * @param [in] hLocalRadio [default:INVALID_HANDLE_VALUE] 使用的本地蓝牙
     * @param [in] timeoutMS [default:DEV_WAIT_TIMEOUT] 枚举的超时时间
     * @param [in] pBDSP [default:NULL] 枚举时的属性,为空表示使用默认属性
     *
     * @return size_t 枚举到的设备数目
     */
    size_t EnumRemoteDevice(list<device_info>& devlist,
        HANDLE hLocalRadio = INVALID_HANDLE_VALUE,
        uint timeoutMS = DEV_WAIT_TIMEOUT,
        BLUETOOTH_DEVICE_SEARCH_PARAMS* pBDSP = NULL)
    {
        LOG_FUNC_NAME();

        BLUETOOTH_DEVICE_INFO bdi = { sizeof(BLUETOOTH_DEVICE_INFO) };
        BLUETOOTH_DEVICE_SEARCH_PARAMS  bdsp;
        size_t count = 0;

        if(pBDSP != NULL)
        {
            memcpy(&bdsp, pBDSP, sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS));
        }
        else
        {
            // 枚举第一个本地蓝牙 
            if(hLocalRadio == INVALID_HANDLE_VALUE)
            {
                list<device_info> bthDevice;
                // 只枚举第一个  
                if(EnumLocalDevice(bthDevice, true) < 1)
                    return _logRetValue(count);

                hLocalRadio = bthDevice.front().hRadio;
            }

            ZeroMemory(&bdsp, sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS));
            bdsp.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
            bdsp.hRadio = hLocalRadio;
            bdsp.fReturnAuthenticated = TRUE;
            bdsp.fReturnRemembered = TRUE;
            bdsp.fReturnUnknown = TRUE;
            bdsp.fReturnConnected = TRUE;
            bdsp.fIssueInquiry = TRUE;
            // 最小一秒的超时时间 
            bdsp.cTimeoutMultiplier = _itobyte(_min(timeoutMS / 1000, 1));
        }

        HBLUETOOTH_DEVICE_FIND hbf = BluetoothFindFirstDevice(&bdsp, &bdi);
        if(hbf == NULL)
        {
            _logErr(DeviceError::OperatorNoSupportErr, "BluetoothFindFirstRadio失败");
            return _logRetValue(count);
        }
        CharConvert convert;
        do
        {
            devlist.push_back(device_info());
            ++count;
            device_info& dev = devlist.back();
            dev.Address = bdi.Address.ullLong;
            dev.Name = convert.to_char(bdi.szName);
            dev.IsDiscoverable = true;
            dev.IsAuthenticated = Tobool(bdi.fAuthenticated);
            dev.IsConnected = Tobool(bdi.fConnected);
            dev.IsRemembered = Tobool(bdi.fRemembered);

            LOGGER(_log << "Find Device:<" << count << ">\n";
            _log << "Name:<" << dev.Name << ">\n"
                << "Address:<" << AddressToString(dev.Address).c_str() << ">\n"
                << "IsAuthenticated:<" << dev.IsAuthenticated << ">\n"
                << "IsConnected:<" << dev.IsConnected << ">\n"
                << "IsRemembered:<" << dev.IsRemembered << ">\n");
        } while(BluetoothFindNextDevice(hbf, &bdi));
        BluetoothFindDeviceClose(hbf);

        return _logRetValue(count);
    }
    //----------------------------------------------------- 
    /**
     * @brief 根据名称获取远程蓝牙地址
     *
     * @param [in] remoteName 需要获取的名称(部分名称)
     * @param [out] adr 获取到的远程地址
     * @param [out] pLocalAdr [default:NULL] 获取到的本地地址(NULL表示不需要)
     * @param [in] ignoreCase [default:true] 在匹配名称时是否忽略大小写
     *
     * @return bool 是否匹配上
     */
    bool NameToAddress(const char* remoteName, BTH_ADDR& adr,
        BTH_ADDR* pLocalAdr = NULL, bool ignoreCase = true)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Name:<" << _strput(remoteName) << ">,IgnoreCase:<" << ignoreCase << ">\n");
        ASSERT_FuncErrRet(!_is_empty_or_null(remoteName), DeviceError::ArgIsNullErr);

        INT iResult = 0;
        ULONG ulFlags = 0;
        ULONG ulPQSSize = sizeof(WSAQUERYSET);
        HANDLE hLookup = 0;
        PWSAQUERYSET pWSAQuerySet = NULL;

        LOGGER(_log.WriteLine("HeapAlloc"));
        pWSAQuerySet = reinterpret_cast<PWSAQUERYSET>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulPQSSize));
        ASSERT_FuncErrInfoRet(pWSAQuerySet != NULL, DeviceError::OperatorStatusErr,
            "Unable to allocate memory for WSAQUERYSET!");

        ulFlags = LUP_CONTAINERS;
        ulFlags |= LUP_RETURN_NAME;
        ulFlags |= LUP_RETURN_ADDR;

        iResult = 0;
        hLookup = 0;
        ZeroMemory(pWSAQuerySet, ulPQSSize);
        pWSAQuerySet->dwNameSpace = NS_BTH;
        pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);

        LOGGER(_log.WriteLine("WSAStartup"));
        if(!SocketHandlerFactory::WsaStart())
        {
            _logErr(DeviceError::DevInitErr, "WSAStartup失败");
            return _logRetValue(false);
        }
        LOGGER(_log.WriteLine("WSAStartup Success!"));

        LOGGER(_log.WriteLine("WSALookupServiceBegin"));
        iResult = WSALookupServiceBegin(pWSAQuerySet, ulFlags, &hLookup);
        if((iResult != NO_ERROR) || (hLookup == NULL))
        {
            LOGGER(_log.WriteLine("HeapFree"));
            HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
            _logErr(DeviceError::OperatorErr, "WSALookupServiceBegin失败");
            return _logRetValue(false);
        }
        LOGGER(_log.WriteLine("WSALookupServiceBegin Success!"));

        CharConvert convert;
        bool isFound = false;
        bool isContinue = true;
        while(isContinue)
        {
            LOGGER(_log.WriteLine("WSALookupServiceNext"));
            iResult = WSALookupServiceNext(hLookup, ulFlags, &ulPQSSize, pWSAQuerySet);
            if(iResult != NO_ERROR)
                iResult = WSAGetLastError();

            LOGGER(_log << "RET:<" << iResult << ">,Hex:<" << _hex(iResult) << ">\n");
            switch(iResult)
            {
            case NO_ERROR:
                if(pWSAQuerySet->lpszServiceInstanceName != NULL)
                {
                    const char* bthName = convert.to_char(pWSAQuerySet->lpszServiceInstanceName);
                    LOGGER(_log << "FindName:<" << bthName << ">\n");
                    if(StringConvert::Contains(bthName, remoteName, ignoreCase))
                    {
                        CopyMemory(&adr, &((PSOCKADDR_BTH)pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr,
                            sizeof(BTH_ADDR));
                        LOGGER(_log << "远程地址:<" << AddressToString(adr) << ">\n");
                        if(pLocalAdr != NULL)
                        {
                            CopyMemory(pLocalAdr, &((PSOCKADDR_BTH)pWSAQuerySet->lpcsaBuffer->LocalAddr.lpSockaddr)->btAddr,
                                sizeof(BTH_ADDR));
                            LOGGER(_log << "本地地址:<" << AddressToString(*pLocalAdr) << ">\n");
                        }

                        isContinue = false;
                    }
                }
                isFound = true;
                break;
            case WSAEFAULT:
                ulFlags |= LUP_FLUSHCACHE;

                HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
                pWSAQuerySet = NULL;
                LOGGER(_log.WriteLine("HeapAlloc"));
                pWSAQuerySet = reinterpret_cast<PWSAQUERYSET>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulPQSSize));
                isContinue = (pWSAQuerySet != NULL);
                // 内存分配失败 
                if(!isContinue)
                {
                    _logErr(DeviceError::OperatorErr, "Allocate memory for WSAQUERYSET失败");
                }
                break;
            case WSA_E_NO_MORE:
                LOGGER(_log.WriteLine("WSALookupServiceNext WSA_E_NO_MORE"));
                isContinue = false;
                break;
            default:
                // 已经枚举到则退出 
                if(isFound) isContinue = false;
                break;
            }
        }
        LOGGER(_log.WriteLine("WSALookupServiceEnd"));
        WSALookupServiceEnd(hLookup);

        if(pWSAQuerySet != NULL)
        {
            LOGGER(_log.WriteLine("HeapFree"));
            HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
        }

        return _logRetValue(isFound);
    }
    //----------------------------------------------------- 
    /**
     * @brief 通过第一个本地蓝牙打开指定名称的外围蓝牙设备
     * @param [in] sArg [default:NULL] 蓝牙设备参数配置项主键:[RemoteName][PWD(可选)](为NULL表示打开上次的设备)
     */
    virtual bool Open(const char* sArg = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "sArg:<" << _strput(sArg) << ">\n");

        BTH_ADDR adr = _bthAdr;
        HANDLE hLocalRadio = _hLocalRadio;
        string pwd = _bthPwd;

        if(!_is_empty_or_null(sArg))
        {
            // 自动查找第一个本地蓝牙hRadio
            list<device_info> devlist;
            ASSERT_FuncErrInfoRet(EnumLocalDevice(devlist, true) > 0, DeviceError::DevNotExistErr, "没有识别到本地蓝牙设备");

            hLocalRadio = devlist.front().hRadio;

            string name;
            ArgParser cfg;
            if(cfg.Parse(sArg))
            {
                BluetoothParam::FromConfig(cfg, name, pwd);
            }
            else
            {
                name = sArg;
            }
            // 查找外围设备 
            ASSERT_FuncErrInfoRet(NameToAddress(name.c_str(), adr), DeviceError::DevInitErr, "查找远程蓝牙设备失败");
        }

        return _logRetValue(Open(hLocalRadio, adr, pwd.c_str()));
    }
    /// 根据指定的地址打开
    bool Open(HANDLE hLocalRadio, BTH_ADDR adr, const char* pwd = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "配对地址:<" << AddressToString(adr) << ">\n"
            << "配对密码:<" << _strput(pwd) << ">\n");

        Close();

        BLUETOOTH_DEVICE_INFO bdi;
        memset(&bdi, 0, sizeof(BLUETOOTH_DEVICE_INFO));
        bdi.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
        bdi.Address.ullLong = adr;

        LOGGER(_log.WriteLine("BluetoothGetDeviceInfo"));
        DWORD dwRet = BluetoothUpdateDeviceRecord(&bdi);
        if(dwRet != ERROR_SUCCESS)
            _logErr(DeviceError::DevStateErr, "BluetoothUpdateDeviceRecord失败");

        dwRet = BluetoothGetDeviceInfo(hLocalRadio, &bdi);
        ASSERT_FuncErrInfoRet(dwRet == ERROR_SUCCESS,
            DeviceError::DevConnectErr, "BluetoothGetDeviceInfo获取远程设备信息失败");
        // 没有配对 
        if(!bdi.fAuthenticated)
        {
            LOGGER(_log.WriteLine("未配对,BluetoothAuthenticateDevice"));
            CharConvert convert;
            dwRet = BluetoothAuthenticateDevice(NULL, hLocalRadio, &bdi, const_cast<PWSTR>(convert.to_wchar(pwd)), _strlen(pwd));
            ASSERT_FuncErrInfoRet(dwRet == ERROR_SUCCESS, DeviceError::DevVerifyErr, "密码配对失败");

            dwRet = BluetoothUpdateDeviceRecord(&bdi);
            if(dwRet != ERROR_SUCCESS)
                _logErr(DeviceError::DevStateErr, "BluetoothUpdateDeviceRecord失败");
        }

        string socketArg = SocketHandlerFactory::ToArg(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        ASSERT_FuncErrRet(TSocketDevice::Open(socketArg.c_str()), DeviceError::DevOpenErr);

        /// 蓝牙服务GUID 
        const GUID GUID_BLUETOOTH_SERVICE_CLASS = { 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74 };

        SOCKADDR_BTH SockAddrBthServer = { 0 };
        SockAddrBthServer.addressFamily = AF_BTH;
        SockAddrBthServer.btAddr = adr;
        SockAddrBthServer.serviceClassId = GUID_BLUETOOTH_SERVICE_CLASS;
        SockAddrBthServer.port = 1;

        if(!TSocketDevice::Connect(reinterpret_cast<sockaddr*>(&SockAddrBthServer), sizeof(SOCKADDR_BTH)))
        {
            TSocketDevice::Close();
            return _logRetValue(false);
        }
        
        _hLocalRadio = hLocalRadio;
        _bthPwd = _strput(pwd);
        _bthAdr = adr;

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /// 获取地址 
    operator const BTH_ADDR&() const { return _bthAdr; }
    /// 获取本地句柄
    operator const HANDLE&() const { return _hLocalRadio; }
    //----------------------------------------------------- 
    /// 删除配对的设备 
    static bool Remove(const BTH_ADDR& adr)
    {
        BLUETOOTH_ADDRESS address;
        address.ullLong = adr;
        bool bRet = (BluetoothRemoveDevice(&address) == ERROR_SUCCESS);
        return bRet;
    }
    /// 删除已配对指定名称的设备 
    static size_t RemoveAll(const char* sName, list<device_info>* pList = NULL)
    {
        list<device_info> devlist;
        if(pList == NULL)
        {
            BluetoothHandlerAppender<TSocketDevice> dev;
            dev.EnumRemoteDevice(devlist);
            pList = &devlist;
        }

        size_t count = 0;
        list<device_info>::iterator itr;
        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if(StringConvert::Contains(itr->Name.c_str(), sName))
            {
                Remove(itr->Address);
                ++count;
            }
        }

        return count;
    }
    /// 设置可见性 
    static bool EnableDiscovery(HANDLE hLocalRadio, bool isEnable = true)
    {
        // 设置可见 
        BOOL bRet = BluetoothEnableDiscovery(hLocalRadio, ToBOOL(isEnable));
        BluetoothEnableIncomingConnections(hLocalRadio, ToBOOL(isEnable));

        return Tobool(bRet);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 默认的蓝牙设备类 
typedef BluetoothHandlerAppender<AsyncSocketDevice> BluetoothDevice;
//--------------------------------------------------------- 
} // namespace env_win32
} // namespace base_device
} // namespace zhou_yb
//========================================================= 
