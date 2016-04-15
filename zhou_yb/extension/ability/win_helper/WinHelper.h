//========================================================= 
/**@file WinHelper.h 
 * @brief Windows环境下的一些辅助接口函数
 * 
 * @date 2013-07-31 21:58:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINHELPER_H_
#define _LIBZHOUYB_WINHELPER_H_
//--------------------------------------------------------- 
#include "../dev_helper/DevHelper.h"
#include "WinRegistryKey.h"
using zhou_yb::extension::ability::RegistryKey;

#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")

#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// Windows下常用功能 
class WinHelper
{
protected:
    WinHelper() {}
public:
    /// 获取系统路径(默认为桌面的路径)
    static const char* GetSystemPath(UINT pathType = CSIDL_DESKTOP)
    {
        static char_t dirPath[_MAX_PATH] = {0};
        static CharConverter cvt;
        SHGetSpecialFolderPath(NULL, dirPath, pathType, 0);
        return cvt.to_char(dirPath);
    }
    /// 获取模块完整路径名称 
    static const char* GetModulePath(const char* moduleName = NULL)
    {
        static char_t path[_MAX_PATH] = { 0 };
        static CharConverter cvt;

        HMODULE hModule = NULL;
        if(_is_empty_or_null(moduleName))
        {
            hModule = GetModuleHandle(cvt.to_char_t(moduleName));
            if(hModule == NULL)
                return "";
        }

        size_t len = static_cast<size_t>(GetModuleFileName(hModule, path, _MAX_PATH));
        if(len < 1)
            return "";
        return cvt.to_char(path);
    }
    /**
     * @brief 获取模块目录路径
     * 
     * @param [in] moduleName 需要获取的模块目录路径,路径后面不带 '\' 
     * 
     * @return const char* ""表示获取失败 
     */
    static const char* GetModuleDirectory(const char* moduleName = NULL)
    {
        char* path = const_cast<char*>(GetModulePath(moduleName));
        size_t len = strlen(path);
        while(path[len] != L'\\')
            --len;
        path[len] = 0;

        return path;
    }
    /**
     * @brief 枚举DLL中的导出函数 
     * 
     * @param [in] dllPath DLL文件路径 
     * @param [out] dllNames 枚举到的DLL导出函数列表 
     * 
     * @return size_t 枚举到的函数数目 
     */
    static size_t EnumFunction(const char* dllPath, list<string>& dllNames)
    {
        size_t count = 0;

        WIN32_FIND_DATA wfd;
        CharConverter cvt;
        memset(&wfd, 0, sizeof(wfd));

        if(FindFirstFile(cvt.to_char_t(dllPath), &wfd) == NULL)
            return count;

        HANDLE hFile = CreateFile(cvt.to_char_t(dllPath), GENERIC_READ, 0, NULL, OPEN_EXISTING, wfd.dwFileAttributes, NULL);
        if(hFile == NULL || hFile == INVALID_HANDLE_VALUE)
            return count;

        HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if(hFileMap == NULL || hFileMap == INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            return count;
        }

        void* modeBase = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
        if(modeBase != NULL)
        {
            IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)modeBase;
            IMAGE_NT_HEADERS* pNtHeader = (IMAGE_NT_HEADERS*)((byte*)modeBase + pDosHeader->e_lfanew);
            IMAGE_OPTIONAL_HEADER* pOptHeader = (IMAGE_OPTIONAL_HEADER*)((byte*)modeBase + pDosHeader->e_lfanew + 24);
            IMAGE_EXPORT_DIRECTORY* pExportDesc = (IMAGE_EXPORT_DIRECTORY*)ImageRvaToVa(pNtHeader, modeBase, pOptHeader->DataDirectory[0].VirtualAddress, 0);
            PDWORD nameAdr = (PDWORD)ImageRvaToVa(pNtHeader, modeBase, pExportDesc->AddressOfNames, 0);
            PTCHAR funcName = NULL;
            for(DWORD i = 0;i < pExportDesc->NumberOfNames; ++i)
            {
                funcName = (PTCHAR)ImageRvaToVa(pNtHeader, modeBase, (DWORD)nameAdr[i], 0);
                ++count;

                dllNames.push_back(cvt.to_char(funcName));
            }
        }

        CloseHandle(hFileMap);
        CloseHandle(hFile);

        return count;
    }
    /// 判断系统版本 
    static bool IsWinVersion(DWORD majorVer, DWORD minorVer)
    {
        OSVERSIONINFOEX osvi;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osvi, sizeof(osvi));
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        osvi.dwMajorVersion = majorVer;
        osvi.dwMinorVersion = minorVer;

        VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
        VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);
        return Tobool(VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask));
    }
    /// 枚举当前目录下的文件和文件夹信息,如果需要枚举多层的,手动处理递归  
    static size_t EnumFiles(const char* rootFolder, list<string>* pFiles, list<string>* pFolders, const char* ext = "*.*")
    {
        size_t count = 0;
        if(pFolders == NULL && pFiles == NULL)
            return count;

        char file[MAX_PATH];
        CharConverter cvt;
        file[0] = 0;
        strcpy(file, rootFolder);
        strcat(file, "\\"); 
        strcat(file, ext);

        WIN32_FIND_DATA wfd; 
        HANDLE hFind = FindFirstFile(cvt.to_char_t(file), &wfd);
        if (hFind == INVALID_HANDLE_VALUE)  
            return count;

        do
        {
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if(pFolders != NULL)
                {
                    (*pFolders).push_back("");
                    string& str = (*pFiles).back();
                    str += rootFolder;
                    str += "\\";
                    str += cvt.to_char(wfd.cFileName);

                    ++count;
                }

                continue;
            }

            if(pFiles != NULL)
            {
                (*pFiles).push_back("");
                string& str = (*pFiles).back();
                str += rootFolder;
                str += "\\";
                str += cvt.to_char(wfd.cFileName);           

                ++count;
            }
        } while(FindNextFile(hFind, &wfd));
        FindClose(hFind);

        return count;
    }
    /// 复制字符串到剪贴板 
    static bool CopyTextToClipboard(HWND hWnd, const char* text)
    {
        BOOL result;
        result = OpenClipboard(hWnd);
        if(!result)
            return false;

        EmptyClipboard();
        
        size_t len = _strlen(text);
        if(len > 0)
        {
            HANDLE hClip = NULL;
            char* pBuf = NULL;

            hClip = GlobalAlloc(GMEM_MOVEABLE, len + 1);
            pBuf = reinterpret_cast<char*>(GlobalLock(hClip));

            memcpy(pBuf, text, len);
            pBuf[len] = 0;

            GlobalUnlock(hClip);
            SetClipboardData(CF_TEXT, hClip);
        }
        CloseClipboard();
        
        return true;
    }
    /// 模拟按键 
    static bool KeyEventText(const char* str, HWND hWnd = NULL)
    {
        if(hWnd == NULL)
            hWnd = GetForegroundWindow();

        DWORD dwFormThreadID = GetCurrentThreadId();
        DWORD dwWindowThredID = GetWindowThreadProcessId(hWnd, NULL);
        if(!AttachThreadInput(dwWindowThredID, dwFormThreadID, TRUE))
            return false;

        hWnd = GetFocus();
        if(INVALID_HANDLE_VALUE == hWnd)
        {
            AttachThreadInput(dwWindowThredID, dwFormThreadID, FALSE);
            return false;
        }

        size_t len = _strlen(str);
        bool isShift = false;
        byte bVK = 0;
        for(size_t i = 0;i < len; ++i)
        {
            isShift = false;

            bVK = str[i];
            switch(str[i])
            {
            case ':':
                isShift = true;
            case ';':
                bVK = VK_OEM_1;
                break;
            case '?':
                isShift = true;
            case '/':
                bVK = VK_OEM_2;
                break;
            case '~':
                isShift = true;
            case '`':
                bVK = VK_OEM_3;
                break;
            case '{':
                isShift = true;
            case '[':
                bVK = VK_OEM_4;
                break;
            case '|':
                isShift = true;
            case '\\':
                bVK = VK_OEM_5;
                break;
            case '}':
                isShift = true;
            case ']':
                bVK = VK_OEM_6;
                break;
            case '\"':
                isShift = true;
            case '\'':
                bVK = VK_OEM_7;
                break;
            case '+':
                bVK = VK_OEM_PLUS;
                break;
            case ',':
                bVK = VK_OEM_COMMA;
                break;
            case '-':
                bVK = VK_OEM_MINUS;
                break;
            case '.':
                bVK = VK_OEM_PERIOD;
                break;
            default:
                break;
            }
            if(isShift) keybd_event(VK_SHIFT, 0, 0, NULL);
            keybd_event(bVK, 0, 0, NULL);
            keybd_event(bVK, 0, KEYEVENTF_KEYUP, NULL);
            if(isShift) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, NULL);
        }

        AttachThreadInput(dwWindowThredID, dwFormThreadID, FALSE);
        return true;
    }
    /// 设置程序开机自启动 
    static bool ExeAutoRun(const char* exePath, bool isAutoRun = true)
    {
        size_t len = _strlen(exePath);
        RegistryKey rootKey = RegistryKey::LocalMachine.OpenSubKey("Software\\Microsoft\\Windows\\CurrentVersion\\Run", KEY_READ | KEY_WRITE);
        RegistryKey appKey = RegistryKey::NullRegistryKey;
        ByteBuilder appName(8);

        // 传入的路径为空则自动获取当前程序 
        if(len < 1)
        {
            exePath = GetModulePath();
            len = strlen(exePath);
            if(len < 1)
                return false;
        }

        size_t index = StringConvert::LastIndexOf(ByteArray(exePath, len), '\\');
        if(index == SIZE_EOF)
            index = 0;
        appName = exePath + index;

        // 过滤 ".exe" 后缀 
        index = StringConvert::LastIndexOf(appName, '.');
        if(index >= 0)
        {
            len = appName.GetLength() - index;
            appName.RemoveTail(len);
        }

        bool bRet = false;
        if(isAutoRun)
        {
            PVOID sArg = reinterpret_cast<PVOID>(const_cast<char*>(exePath));
            bRet = rootKey.SetValue(appName.GetString(), REG_SZ, sArg, _strlen(exePath));
        }
        else
        {
            // 删除注册表键值 
            bRet = rootKey.DeleteValue(exePath);
        }
        return bRet;
    }
    /// 重置系统自动追加的串口序号 
    static bool ResetComDB()
    {
        RegistryKey key = RegistryKey::LocalMachine.OpenSubKey("SYSTEM\\CurrentControlSet\\Control\\COM Name Arbiter", KEY_READ | KEY_WRITE);
        return key.DeleteValue("ComDB");
    }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINHELPER_H_
//========================================================= 
