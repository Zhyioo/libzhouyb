//========================================================= 
/**@file FolderLogger.h 
 * @brief 文件夹日志,将文件夹输出到一个固定的目录,同时控制
 * 日志文件总数及每个日志文件的大小,对于超过数据和大小的,删
 * 除最早的一个文件版本 
 * 
 * @date 2014-06-01   18:09:02 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_FOLDERLOGGER_H_
#define _LIBZHOUYB_FOLDERLOGGER_H_
//--------------------------------------------------------- 
#include "StringLogger.h"
//---------------------------------------------------------
/// 文件大小基数 
#define FILE_SIZE_MODE (1024)
/// n K
#define FILE_K(n) ((n)*FILE_SIZE_MODE)
/// n M
#define FILE_M(n) (FILE_K(n)*FILE_SIZE_MODE)
/// n G
#define FILE_G(n) (FILE_M(n)*FILE_SIZE_MODE)
/// n T
#define FILE_T(n) (FILE_G(n)*FILE_SIZE_MODE)
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
#ifdef _MSC_VER
//--------------------------------------------------------- 
/**
 * @brief 文件夹辅助工具,负责文件夹的创建,枚举子文件,获取子文件大小,删除文件  
 */ 
class FolderHelper
{
protected:
    FolderHelper() {}
public:
    /// 返回文件夹是否存在 
    static bool IsFolderExist(const char* path)
    {
        CharConverter cvt;
        DWORD dw = GetFileAttributes(cvt.to_char_t(path));
        if(dw == INVALID_FILE_ATTRIBUTES)
            return false;

        return (dw & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
    }
    /// 创建文件夹 
    static bool CreateFolder(const char* path)
    {
        size_t len = _strlen(path);
        ByteArray pathArray(path, len);
        size_t splitCount = StringConvert::GetSplitFlagCount(pathArray, PATH_SEPARATOR);
        // 记录第一个开始新建的文件夹,以便没有全部创建成功的时候删除 
        string createDir;
        string dir;
        ByteBuilder tmp(8);
        CharConverter cvt;

        for(size_t i = 0;i <= splitCount; ++i)
        {
            tmp.Clear();
            StringConvert::Split(pathArray, tmp, i, PATH_SEPARATOR);
            dir += tmp.GetString();

            // 不以 \\ 结尾 
            if(!tmp.IsEmpty())
                dir += static_cast<byte>(PATH_SEPARATOR);
            else
                break;

            // 如果文件夹不存在则新建 
            if(!IsFolderExist(dir.c_str()))
            {
                // 是第一个创建的文件夹 
                if(createDir.length() < 1)
                {
                    createDir = dir;
                }
                // 创建失败,删除已经创建的文件夹 
                if(CreateDirectory(cvt.to_char_t(dir.c_str()), NULL) != TRUE)
                {
                    DeleteFolder(createDir.c_str());
                    return false;
                }
            }
        }

        return true;
    }
    /// 删除文件夹 
    static bool DeleteFolder(const char* path)
    {
        char file[MAX_PATH];
        CharConverter cvt;
        file[0] = 0;
        
        size_t len = _strlen(path);
        memcpy(file, path, len);
        file[len] = 0;
        if(file[len - 1] != PATH_SEPARATOR)
            file[len++] = PATH_SEPARATOR;
        file[len++] = '*';
        file[len] = 0;

        WIN32_FIND_DATA wfd; 
        HANDLE hFind = FindFirstFile(cvt.to_char_t(file), &wfd);
        if (hFind == INVALID_HANDLE_VALUE)  
            return false;

        do
        {
            string subpath;
            subpath = path;
            subpath += PATH_SEPARATOR;
            subpath += cvt.to_char(wfd.cFileName);

            if (wfd.cFileName[0] == '.')
            {
                continue;
            }
            // 去除只读属性 
            SetFileAttributes(cvt.to_char_t(subpath.c_str()), FILE_ATTRIBUTE_NORMAL);
            if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 递归删除子文件夹 
                DeleteFolder(subpath.c_str());
                RemoveDirectory(cvt.to_char_t(subpath.c_str()));
                continue;
            }
            // 普通文件直接删除 
            DeleteFile(cvt.to_char_t(subpath.c_str()));
        } while(FindNextFile(hFind, &wfd));
        RemoveDirectory(cvt.to_char_t(path));
        FindClose(hFind);

        return true;
    }
    /// 删除单独的文件 
    static bool EraseFile(const char* path)
    {
        CharConverter cvt;
        return ::DeleteFile(cvt.to_char_t(path)) == TRUE;
    }
    /// 枚举文件夹下的第一层文件 
    static size_t EnumFiles(const char* dir, list<string>& files, list<size_t>* pfilesize = NULL, const char* ext = "*.*")
    {
        char file[MAX_PATH];
        CharConverter cvt;
        file[0] = 0;
        strcpy(file, dir);
        if(_strlen(ext) > 0)
        {
            strcat(file, "\\"); 
            if(ext[0] == '.')
                strcat(file, "*");
            strcat(file, ext);
        }
        else
        {
            strcpy(file, "\\*");
        }
        
        WIN32_FIND_DATA wfd;
        size_t count = 0;

        HANDLE hFind = FindFirstFile(cvt.to_char_t(file), &wfd);
        if (hFind == INVALID_HANDLE_VALUE)  
            return static_cast<size_t>(0);

        do
        {
            // 跳过文件夹 
            if (wfd.cFileName[0] == '.' || (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                continue;
            }
            files.push_back(cvt.to_char(wfd.cFileName));
            if(pfilesize != NULL)
            {
                pfilesize->push_back(static_cast<size_t>(wfd.nFileSizeLow));
            }
            ++count;
        } while(FindNextFile(hFind, &wfd));
        FindClose(hFind);
        return count;
    }
    /// 获取文件大小 
    static size_t GetFileSize(const char* path)
    {
        WIN32_FIND_DATA wfd;
        CharConverter cvt;
        HANDLE hFind = FindFirstFile(cvt.to_char_t(path), &wfd);
        if (hFind == INVALID_HANDLE_VALUE)  
            return static_cast<size_t>(0);
        FindClose(hFind);

        return static_cast<size_t>(wfd.nFileSizeLow);
    }
};
/**
 * @brief 将日志重定向到一个文件夹中,按照日期分组,控制最大日志文件数和文件大小 
 */ 
//--------------------------------------------------------- 
#else
//---------------------------------------------------------
#include<sys/stat.h>
#include<dirent.h>
//---------------------------------------------------------
/**
 * @brief 文件夹辅助工具,负责文件夹的创建,枚举子文件,获取子文件大小,删除文件  
 */ 
class FolderHelper
{
protected:
    FolderHelper() {}
public:
    /// 返回文件夹是否存在
    static bool IsFolderExist(const char* path)
    {
        struct stat st;
        if(stat(path, &st) != 0)
            return false;
        return S_ISDIR(st.st_mode) != 0;
    }
    /// 创建文件夹
    static bool CreateFolder(const char* path)
    {
        ByteArray pathArray(path);
        size_t splitCount = StringConvert::GetSplitFlagCount(pathArray, PATH_SEPARATOR);
        // 记录第一个开始新建的文件夹,以便没有全部创建成功的时候删除
        string createDir;
        string dir;
        ByteBuilder tmp(8);
        for(size_t i = 0;i <= splitCount; ++i)
        {
            tmp.Clear();
            StringConvert::Split(pathArray, tmp, i, PATH_SEPARATOR);
            dir += tmp.GetString();
            dir += static_cast<byte>(PATH_SEPARATOR);

            // 不以 / 结尾
            if(tmp.IsEmpty())
            {
                continue;
            }

            // 如果文件夹不存在则新建
            if(!IsFolderExist(dir.c_str()))
            {
                // 是第一个创建的文件夹
                if(createDir.length() < 1)
                {
                    createDir = dir;
                }
                // 创建失败,删除已经创建的文件夹
                if(mkdir(dir.c_str(), 0766) != 0)
                {
                    DeleteFolder(createDir.c_str());
                    return false;
                }
            }
        }
        return true;
    }
    /// 删除文件夹
    static bool DeleteFolder(const char* path)
    {
        DIR *pDir = NULL;
        struct dirent *pDirent = NULL;
        
        pDir = opendir(path);
        if (pDir == NULL)
            return false;

        while((pDirent = readdir(pDir)) != NULL)
        {
            // 跳过隐藏的目录
            if (pDirent->d_name[0] == '.')
            {
                continue;
            }
            string fullpath = path;
            fullpath += "/";
            fullpath += pDirent->d_name;

            struct stat st;
            if(stat(fullpath.c_str(), &st) == 0)
            {
                // 如果是文件夹则递归删除
                if(S_ISDIR(st.st_mode))
                {
                    DeleteFolder(fullpath.c_str());
                    rmdir(fullpath.c_str());
                    continue;
                }
                // 普通文件直接删除
                EraseFile(fullpath.c_str());
            }
        }
        closedir(pDir);
        return true;
    }
    /// 删除单独的文件
    static bool EraseFile(const char* path)
    {
        return remove(path) == 0;
    }
    /// 枚举文件夹下的第一层文件
    static size_t EnumFiles(const char* dir, list<string>& files, list<size_t>* pfilesize = NULL, const char* ext = "")
    {
        DIR *pDir = NULL;
        struct dirent *pDirent = NULL;
        size_t count = 0;

        pDir = opendir(dir);
        if (pDir == NULL)
            return static_cast<size_t>(0);

        while((pDirent = readdir(pDir)) != NULL)
        {
            // 跳过隐藏的目录
            if (pDirent->d_name[0] == '.')
            {
                continue;
            }
            string fullpath = dir;
            if(fullpath[fullpath.length()-1] != PATH_SEPARATOR)
                fullpath += static_cast<byte>(PATH_SEPARATOR);
            fullpath += pDirent->d_name;

            // 过滤文件
            if(StringConvert::Contains(pDirent->d_name, _strput(ext)) != true)
                continue;

            struct stat st;
            if(stat(fullpath.c_str(), &st) == 0)
            {
                // 如果是文件夹则跳过
                if(S_ISDIR(st.st_mode))
                    continue;
                files.push_back(pDirent->d_name);
                if(pfilesize != NULL)
                {
                    pfilesize->push_back(static_cast<size_t>(st.st_size));
                }
            }
        }
        closedir(pDir);

        return count;
    }
    /// 获取文件大小
    static size_t GetFileSize(const char* path)
    {
        struct stat st;
        if(stat(path, &st) != 0)
            return static_cast<size_t>(0);

        return static_cast<size_t>(st.st_size);
    }
};
//--------------------------------------------------------- 
#endif
//--------------------------------------------------------- 
/**
 * @brief 文件夹日志 
 * 
 * 在指定的文件夹下输出日志,日志由N个子文件组成,每个子文件名称按照 
 * "文件夹" + "\子标题" + "时间".log 组成,按照时间顺序进行排序 
 * 每个日志有固定的大小作为限制,超过数目限制时,自动删除时间最早的日志 
 */ 
class FolderLogger : public StringLogger
{
protected:
    /// 最大输出文件数目 
    size_t _logcount;
    /// 当前输出文件大小 
    size_t _logsize;
    /// 最大输出文件大小  
    size_t _maxsize;
    /// 日志标题 
    string _title;
    /// 输出目录路径 
    string _dir;
    /// 当前输出的文件组(按照时间顺序进行排序) 
    list<string> _logs;
    
    /// 输出文件流 
    ofstream _fout;

    // 缓冲区事件(计算输出的文件大小) 
    virtual void _buf_after_changed()
    {
        string strVal = StringLogger::String();
        StringLogger::Clear();

        size_t len = strVal.length();
        _logsize += len;

        // 如果超过限制大小重新输出到另外一个文件 
        if(_logsize >= _maxsize)
            _reset_file();

        _fout<<strVal;
    }
    /// 初始化路径 
    void _init_dir(const char* dir)
    {
        _dir = _strput(dir);
        if(_dir.length() < 1)
        {
            _dir = ".";
            _dir += PATH_SEPARATOR;
            _dir += "logs";
        }
        if(_dir[_dir.length() - 1] != PATH_SEPARATOR)
            _dir += PATH_SEPARATOR;
    }
    /// 初始化操作 
    void _init(const char* dir, const char* logTitle, size_t logCount, size_t logMaxSize)
    {
        _os = &_ostr;

        _init_dir(dir);

        _logcount = logCount;
        _title = _strput(logTitle);
        // 默认日志最小为1K
        _maxsize = _max(logMaxSize, FILE_K(1));

        _logsize = 0;
    }
    /// 是否是指定的日志文件名 
    bool _is_log(string& path)
    {
        bool isLog = false;
        if(_title.length() < 1)
        {
            isLog = true;
            // 文件名为:"时间.log";
            for(int i = 0;i < 14; ++i)
            {
                if(_is_digital(path[i]))
                {
                    isLog = false;
                    break;
                }
            }
        }
        else
        {
            // 文件名以Title开头
            isLog = StringConvert::StartWith(ByteArray(path.c_str(), path.length()), ByteArray(_title.c_str(), _title.length()));
        }
        return isLog;
    }
    /// 生成一个新的日志文件名到文件日志组中 
    void _new_log()
    {
        ByteBuilder sys_time(32);
        get_current_systime(sys_time);
        /* "2014-05-26 12:32:15" => "20140526123215" */
        StringConvert::Remove(sys_time, ' ');
        StringConvert::Remove(sys_time, ':');
        StringConvert::Remove(sys_time, '-');

        // 如果在同一秒内产生了多个文件,再次按照序号重新命名 
        string str;
        if(_title.length() > 0)
        {
            str += _title;
            str += "_";
        }
        str += sys_time.GetString();

        if(_logs.size() > 0)
        {
            string& logback = _logs.back();
            // 相同时间内需要产生多个日志,文件名重复,在后续的文件名上加上序号
            if(StringConvert::StartWith(ByteArray(logback.c_str(), logback.length()), ByteArray(str.c_str(), str.length())))
            {
                // 第一次重复
                if(logback.length() == str.length() + 4)
                {
                    str += "_00";
                }
                else
                {
                    char sNum[4] = {0};
                    // title + "_" + 14 + "_00" + ".log"
                    memcpy(sNum, logback.c_str() + _title.length() + 16, 2);
                    int num = atoi(sNum);
                    ++num;
                    sprintf(sNum, "%02d", num);

                    str += "_";
                    str += sNum;
                }
            }
        }

        str += ".log";

        _logs.push_back(str);
    }
    /// 重置日志文件,返回是否重置过 
    bool _reset_file()
    {
        if(_logsize < _maxsize)
            return false;
        // 需要重新输出到另外一个文件中 
        _new_log();
        _fout.flush();
        _fout.close();

        string logpath;
        // 删除旧的日志(只保留链表中最新的一个) 
        while(_logs.size() > _logcount)
        {
            logpath = _dir + _logs.front();
            FolderHelper::EraseFile(logpath.c_str());
            _logs.pop_front();
        }
        _logsize = 0;

        logpath = _dir + _logs.back();
        _fout.open(logpath.c_str(), ios_base::out | ios_base::app);
        // 如果创建文件失败则继续输出到之前的文件 
        if(_fout.fail())
        {
            _logs.pop_back();
            logpath = _dir + _logs.back();
            _fout.open(logpath.c_str(), ios_base::out | ios_base::app);
            return false;
        }
        return true;
    }
    /// 开始记录日志(处理输出的文件名)
    bool _open()
    {
        if(!StringLogger::Open())
            return false;

        list<string> logs;
        list<size_t> logsize;

        if(!FolderHelper::IsFolderExist(_dir.c_str()))
        {
            if(!FolderHelper::CreateFolder(_dir.c_str()))
            {
                StringLogger::Close();
                return false;
            }
        }
        else
        {
            FolderHelper::EnumFiles(_dir.c_str(), logs, &logsize, ".log");

            // 剔除不是该分组的文件 
            list<string>::iterator itr;
            list<size_t>::iterator itrSize;

            for(itr = logs.begin(),itrSize = logsize.begin();itr != logs.end();)
            {
                bool isLog = _is_log(*itr);
                if(!isLog)
                {
                    list<string>::iterator tmp = itr;
                    list<size_t>::iterator tmpSize = itrSize;
                    ++tmp;
                    ++tmpSize;

                    logs.erase(itr);
                    itr = tmp;

                    logsize.erase(itrSize);
                    itrSize = tmpSize;
                }
                else
                {
                    ++itr;
                    ++itrSize;
                }
            }

            if(logs.size() > 0)
            {
                // 按照文件名称时间顺序进行排序(直接插入排序,字符串可以直接排序) 
                list<string>::iterator itrInsert;
                for(itr = logs.begin();itr != logs.end(); ++itr)
                {
                    for(itrInsert = _logs.begin();itrInsert != _logs.end(); ++itrInsert)
                    {
                        // 长度小的排在前面 
                        if(itrInsert->length() < itr->length() || (*itrInsert) > (*itr))
                            break;
                    }
                    _logs.insert(itrInsert, (*itr));
                }

                // 设置当前输出的日志文件大小值 
                size_t index = 0;
                for(itr = logs.begin();itr != logs.end(); ++itr)
                {
                    if((*itr) == _logs.back())
                        break;
                    ++index;
                }
                _logsize = *(list_helper<size_t>::index_of(logsize, index));
            }
        }

        // 暂时没有文件或者最后一个日志文件超过设定大小  
        if(_logsize >= _maxsize || logs.size() < 1)
        {
            // 没有文件名称则新建一个日志名称  
            _new_log();
            _logsize = 0;
        }

        string logpath = _dir + _logs.back();
        _fout.open(logpath.c_str(), ios_base::app | ios_base::out);
        if(_fout.fail()) StringLogger::Close();

        return IsOpen();
    }
public:
    /// 默认将日志存放在当前目录logs下,文件名为"时间.log",日志数为5,每个文件大小为256K 
    FolderLogger(bool isOpen = false, const char* dir = "", const char* logTitle = "",
        size_t logCount = 5, size_t logMaxSize = FILE_K(256)) : StringLogger()
    {
        _init(dir, logTitle, logCount, logMaxSize);
        if(isOpen)
            _open();
    }
    virtual ~FolderLogger() { Close(); }
    //----------------------------------------------------- 
    //@{
    /**@name
     * @brief IBaseDevice成员 
     */
    /**
     * @brief 开启日志记录功能
     * @param [in] sArg [default:NULL]
     * - 参数格式:
     *  - NULL 根据上一次配置打开 
     *  - 文件夹路径 
     *  - string FileLogger::FilePathKey 配置项参数 
     * .
     */ 
    virtual bool Open(const char* sArg = NULL)
    {
        if(sArg != NULL)
        {
            ArgParser cfg;
            cfg.Parse(sArg);
            if(!cfg.Contains(FileLogger::FilePathKey))
                _init_dir(sArg);
            else
            {
                string dir = cfg[FileLogger::FilePathKey].To<string>();
                if(dir.length() < 1)
                    _init_dir("");
                else
                    _init_dir(dir.c_str());
            }
        }
        Close();

        return _open();
    }
    /// 开启日志功能 
    bool Open(const char* dir, const char* logTitle, size_t logCount, size_t logMaxSize)
    {
        _init(dir, logTitle, logCount, logMaxSize);
        Close();

        return _open();
    }
    /// 是否打开日志 
    virtual bool IsOpen()
    {
        return _fout.is_open() && StringLogger::IsOpen();
    }
    /// 关闭日志记录功能
    virtual void Close()
    {
        if(IsOpen())
        {
            StringLogger::Close();
            _fout.close();
        }
    }
    //@}
    /// 刷新缓冲区 
    virtual void Flush()
    {
        if(IsOpen())
        {
            StringLogger::Flush();
            _buf_after_changed();

            _fout.flush();
        }
    }
    /// 删除整个日志文件夹 
    void Clear()
    {
        StringLogger::Clear();
        if(_dir.length() > 0 && FolderHelper::IsFolderExist(_dir.c_str()))
        {
            FolderHelper::DeleteFolder(_dir.c_str());
        }
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_FOLDERLOGGER_H_
//========================================================= 
