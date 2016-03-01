//========================================================= 
/**@file WinDllHelper.h
 * @brief DLL辅助函数  
 * 
 * @date 2015-05-30   15:16:07
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// DLL文件辅助函数 
class WinDllHelper
{
public:
    WinDllHelper() {}
public:
    /**
     * @brief 枚举DLL中的导出函数 
     * 
     * @param [in] dllPath DLL文件路径 
     * @param [out] dllNames 枚举到的DLL导出函数列表 
     * 
     * @return size_t 枚举到的函数数目 
     */
    static size_t EnumFunction(const char_t* dllPath, list<string_t>& dllNames)
    {
        size_t count = 0;

        WIN32_FIND_DATA wfd;
        memset(&wfd, 0, sizeof(wfd));

        if(FindFirstFile(dllPath, &wfd) == NULL)
            return count;

        HANDLE hFile = CreateFile(dllPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, wfd.dwFileAttributes, NULL);
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

                dllNames.push_back(funcName);
            }
        }

        CloseHandle(hFileMap);
        CloseHandle(hFile);

        return count;
    }
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension 
} // namespace zhou_yb
//========================================================= 