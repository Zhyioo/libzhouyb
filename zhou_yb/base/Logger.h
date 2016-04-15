//========================================================= 
/**@file Logger.h 
 * @brief 日志
 * 
 * 提供对日志的基本操作,封装基本的文件日志,以及日志适配器 
 * 
 * @date 2011-10-17   15:23:26 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LOGGER_H_
#define _LIBZHOUYB_LOGGER_H_
//--------------------------------------------------------- 
#include <fstream>
using std::char_traits;
using std::basic_ostream;
using std::ostream;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::endl;
using std::ios;
using std::ios_base;

#include "Convert.h"
#include "DevDefine.h"
#include "ArgParser.h"

#include "../container/object.h"
using zhou_yb::container::Ref;
using zhou_yb::container::RefObject;

#include "../container/container_helper.h"
using zhou_yb::container::container_helper;
//---------------------------------------------------------
namespace zhou_yb {
namespace base {
//---------------------------------------------------------
/// 获取当前系统时间 
extern size_t get_current_systime(ByteBuilder& timebuff);
//--------------------------------------------------------- 
/// 输出系统时间，类似于endl; cout<<current_systime<<endl;
template<class _Elem, class _Traits>
basic_ostream<_Elem, _Traits>& current_systime(basic_ostream<_Elem, _Traits>& _Ostr)
{
    ByteBuilder timeBuf(32);
    size_t len = get_current_systime(timeBuf);

    _Ostr.write(timeBuf.GetString(), len);
    return (_Ostr);
}
//--------------------------------------------------------- 
/* ---日志记录器接口--- */
//---------------------------------------------------------
/**
 * @brief 流数据日志(把该类日志也当作一个设备来使用)
 *
 * 内部封装ostream,支持从流的引用中创建(意味着可以使用cout作为日志记录的输出)
 * 对模板的函数不能设置为 virtual = 0，在LoggerAdapter中被定义为 ILogger
 */
class OstreamLogger : public IBaseDevice, public RefObject
{
public:
    //----------------------------------------------------- 
    /// OstreamLogger.Open方法传入的配置信息主键 Ptr
    static const char PointerKey[5];

    typedef basic_ostream<char, char_traits<char> > char_ostream;
    typedef char_ostream & (*ILogger_Pfn)(char_ostream&);
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 输出的设备指针
    ostream* _os;
    /* 由于operator<<(T)不能为虚函数，只能够用额外的虚函数通知子类 */
    /// 输出前触发的动作
    virtual void _buf_before_changed() {}
    /// 输出及输出后触发的动作
    virtual void _buf_after_changed() {}
    //----------------------------------------------------- 
public:
    OstreamLogger(ostream& os)
        : _os(&os) {}
    OstreamLogger() : _os(NULL) {}
    virtual ~OstreamLogger()
    {
        Close();
        _os = NULL;
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief IBaseDevice成员
     */
    /// 打开流
    /**
     * @brief 打开流
     * @param [in] sArg [default:NULL] 
     * - 参数格式:
     *  - NULL 打开上次的流数据 
     *  - pointer PointerKey 指定的流地址配置项参数 
     * .
     */ 
    virtual bool Open(const char* sArg = NULL)
    {
        if(!_is_empty_or_null(sArg))
        {
            ArgParser cfg;
            cfg.Parse(sArg);
            pointer ptr = NULL;
            ArgConvert::FromConfig(cfg, PointerKey, ptr);

            ostream* pOs = reinterpret_cast<ostream*>(ptr);
            if(pOs != NULL) Open(*pOs);
        }

        return IsOpen();
    }
    /// 打开os对象
    bool Open(ostream& os)
    {
        Close();
        _os = &os;

        return true;
    }
    /// 判断是否打开流
    virtual bool IsOpen()
    {
        return _os != NULL;
    }
    /// 关闭流
    virtual void Close()
    {
        Flush();
        _os = NULL;
    }
    //@}
    //-----------------------------------------------------
    /// 刷新缓冲区
    virtual void Flush()
    {
        if(IsOpen())
        {
            _os->flush();
        }
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 输出操作
     * os<<param1<<param2<<param3...
     */
    template<class T>
    OstreamLogger& operator<<(const T& obj)
    {
        if(IsOpen())
        {
            _buf_before_changed();
            (*_os)<<obj;
            _buf_after_changed();
        }
        return (*this);
    }
    /// 对endl等的类似函数的支持
    virtual OstreamLogger& operator<<(ILogger_Pfn _Pfn)
    {
        if(IsOpen())
        {
            _buf_before_changed();
            _Pfn((*_os));
            _buf_after_changed();
        }
        return (*this);
    }
    //@}
    //-----------------------------------------------------
};
//---------------------------------------------------------
/**
 * @brief 将日志记录到文件的日志记录器 
 */
class FileLogger : public OstreamLogger
{
public:
    //----------------------------------------------------- 
    /// FileLogger.Open配置项参数主键 FilePath
    static const char FilePathKey[10];
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 日志文件名
    ByteBuilder _fileName;
    /// 输出流 
    ofstream _fout;
    //----------------------------------------------------- 
    /// 打开日志文件
    void _openlog(bool append = true)
    {
        if(!OstreamLogger::Open(_fout))
            return;
        ios_base::openmode mode = (append ? (ios_base::app | ios_base::out) : ios_base::out);
        _fout.open(_fileName.GetString(), mode);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    FileLogger(bool openlog = false, const char* fileName = "log.txt")
        : _fileName(fileName), OstreamLogger()
    {
        _os = &_fout;
        if(openlog)
            Open();
    }
    virtual ~FileLogger() { Close(); }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief IBaseDevice成员 
     */
    /**
     * @brief 开启日志记录功能
     * @param [in] sArg [default:NULL]
     * - 参数格式:
     *  - NULL 打开上次的文件
     *  - 直接的文件路径 
     *  - string FilePathKey 指定的文件路径配置项参数
     * .
     */ 
    virtual bool Open(const char* sArg = NULL)
    {
        if(sArg != NULL)
        {
            ArgParser cfg;
            cfg.Parse(sArg);
            if(!cfg.Contains(FilePathKey))
                _fileName = sArg;
            else
                _fileName = cfg.GetValue(FilePathKey);
            // 如果_fileName以'/'结尾或者没有扩展名则认为路径名,自动加上log.txt
            if(_fileName.IsEmpty())
            {
                _fileName = "log.txt";
            }
            else
            {
                size_t index = StringConvert::IndexOf(_fileName, '.');
                // 没有文件扩展名
                if(index == SIZE_EOF)
                {
                    if(_fileName[_fileName.GetLength() - 1] != PATH_SEPARATOR)
                        _fileName += PATH_SEPARATOR;
                    _fileName += "log.txt";
                }
            }
        }

        Close();
        if(!_fileName.IsEmpty())
        {
            _openlog();
        }

        return IsOpen();
    }
    /// 是否打开文件
    virtual bool IsOpen()
    {
        return _fout.is_open() && OstreamLogger::IsOpen();
    }
    /// 关闭日志记录功能
    virtual void Close()
    {
        if(IsOpen())
        {
            OstreamLogger::Close();
            _fout.close();
        }
    }
    //@}
    //-----------------------------------------------------
    /// 清空日志数据 
    void Clear()
    {
        // 关闭再重新打开、关闭
        bool last = IsOpen();
        Close();
        // 文件不存在则放弃 
        if(rename(_fileName.GetString(), _fileName.GetString()) != 0)
            return ;

        _openlog(false);
        if(!last)
        {
            Close();
        }
    }
    //----------------------------------------------------- 
    /// 获取当前日志文件 
    const char* const GetFile() const
    {
        return _fileName.GetString();
    }
    //-----------------------------------------------------
};
//---------------------------------------------------------
/**
 * @brief 日志适配器
 * 
 * 对日志记录器的封装，使得日志记录在设备间按层次可嵌套的共享使用
 * 支持同时将日志输出到多个日志记录器中 
 * 
 */
class LoggerAdapter : public RefObject
{
public:
    //-----------------------------------------------------
    typedef OstreamLogger ILogger;
    typedef ILogger::char_ostream char_ostream;
    typedef ILogger::ILogger_Pfn ILogger_Pfn;
    static const LoggerAdapter Null;
    //-----------------------------------------------------
protected:
    //-----------------------------------------------------
    /// 日志对象引用指针
    list<Ref<ILogger> > _logList;
    list<Ref<LoggerAdapter> > _adapterList;
    //----------------------------------------------------- 
    /// 初始化私有数据
    inline void _init()
    {
        IsEnable = true;
        _logList.clear();
        _adapterList.clear();
    }
    /// 清理无效的日志对象 
    void _clean()
    {
        Ref<ILogger> null_logger;
        container_helper<list<Ref<ILogger> > >::remove(_logList, null_logger, true);

        Ref<LoggerAdapter> null_adapter;
        container_helper<list<Ref<LoggerAdapter> > >::remove(_adapterList, null_adapter, true);
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    LoggerAdapter(ILogger& log)
    {
        _init();

        Select(log);
    }
    LoggerAdapter(const LoggerAdapter& other)
    {
        _init();

        Select(other);
    }
    LoggerAdapter() { _init(); }

    virtual ~LoggerAdapter()
    {
        Flush();
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 输出操作
     * os<<param1<<param2<<param3...
     */
    template<class T>
    LoggerAdapter& operator<<(const T& obj)
    {
        if(!IsEnable) return (*this);

        list<Ref<ILogger> >::iterator itr;
        for(itr = _logList.begin(); itr != _logList.end();++itr)
        {
            if(itr->IsNull())
            {
                list<Ref<ILogger> >::iterator itrErase = itr;
                --itr;
                _logList.erase(itrErase);
            }
            else
            {
                (*itr)->operator<< (obj);
            }
        }
        list<Ref<LoggerAdapter> >::iterator itrAdapter;
        for(itrAdapter = _adapterList.begin();
            itrAdapter != _adapterList.end();
            ++itrAdapter)
        {
            if(itrAdapter->IsNull())
            {
                list<Ref<LoggerAdapter> >::iterator itrAdapterErase = itrAdapter;
                --itrAdapter;
                _adapterList.erase(itrAdapterErase);
            }
            else
            {
                (*itrAdapter)->operator<< (obj);
            }
        }
        return (*this);
    }
    LoggerAdapter& operator<<(const bool& obj)
    {
        if(!IsEnable) return (*this);

        (*this)<<(obj ? "True" : "False");
        return (*this);
    }
    LoggerAdapter& operator<<(const std::wstring& obj)
    {
        if(!IsEnable) return (*this);

        CharConverter cvt;
        (*this) << cvt.to_char(obj.c_str());
        return (*this);
    }
    LoggerAdapter& operator<<(const wchar_t* wstr)
    {
        if(!IsEnable) return (*this);

        CharConverter cvt;
        (*this) << cvt.to_char(wstr);
        return (*this);
    }
    inline LoggerAdapter& operator<<(const ByteArray& obj)
    {
        return WriteHex(obj);
    }
    inline LoggerAdapter& operator<<(const ByteBuilder& obj)
    {
        return WriteHex(obj);
    }
    /// 对endl等的类似函数的支持
    LoggerAdapter& operator<<(ILogger_Pfn _Pfn)
    {
        if(!IsEnable) return (*this);

        list<Ref<ILogger> >::iterator itr;
        for(itr = _logList.begin(); itr != _logList.end(); ++itr)
        {
            if(itr->IsNull())
            {
                list<Ref<ILogger> >::iterator itrErase = itr;
                --itr;
                _logList.erase(itrErase);
            }
            else
            {
                (*itr)->operator<< (_Pfn);
            }
        }
        list<Ref<LoggerAdapter> >::iterator itrAdapter;
        for(itrAdapter = _adapterList.begin();
            itrAdapter != _adapterList.end();
            ++itrAdapter)
        {
            if(itrAdapter->IsNull())
            {
                list<Ref<LoggerAdapter> >::iterator itrAdapterErase = itrAdapter;
                --itrAdapter;
                _adapterList.erase(itrAdapterErase);
            }
            else
            {
                (*itrAdapter)->operator<< (_Pfn);
            }
        }
        return (*this);
    }
    //@}
    //-----------------------------------------------------
    //@{
    /**@name 
     * @brief 赋值运算符 
     */ 
    inline LoggerAdapter& operator +=(const ILogger& log)
    {
        return Select(log);
    }
    inline LoggerAdapter& operator +=(const LoggerAdapter& logAdapter)
    {
        return Select(logAdapter);
    }
    inline LoggerAdapter& operator -=(const ILogger& log)
    {
        return Release(log);
    }
    inline LoggerAdapter& operator -=(const LoggerAdapter& logAdapter)
    {
        return Release(logAdapter);
    }
    //@}
    //----------------------------------------------------- 
    /// 是否允许该适配器记录日志
    bool IsEnable;
    //----------------------------------------------------- 
    /// 采用指定的日志器记录日志
    LoggerAdapter& Select(const ILogger& log)
    {
        if(!Contains(log))
        {
            _logList.push_back(log);
        }
        return (*this);
    }
    /// 采用指定的日志器记录日志
    LoggerAdapter& Select(const LoggerAdapter& logAdapter)
    {
        if(this != &logAdapter)
        {
            if(!Contains(logAdapter))
            {
                _adapterList.push_back(logAdapter);
            }
        }
        return (*this);
    }
    /// 移除对日志的引用 
    LoggerAdapter& Release(const ILogger& log)
    {
        list<Ref<ILogger> >::iterator itr;
        bool isFind = false;
        for(itr = _logList.begin(); itr != _logList.end(); ++itr)
        {
            isFind = ((*itr) == log);
            if(itr->IsNull() || isFind)
            {
                list<Ref<ILogger> >::iterator itrErase = itr;
                --itr;
                _logList.erase(itrErase);
            }
            if(isFind)
                break;
        }
        return (*this);
    }
    /// 移除对日志的引用 
    LoggerAdapter& Release(const LoggerAdapter& log)
    {
        bool isFind = false;
        list<Ref<LoggerAdapter> >::iterator itrAdapter;
        for(itrAdapter = _adapterList.begin();
            itrAdapter != _adapterList.end();
            ++itrAdapter)
        {
            isFind = ((*itrAdapter) == log);
            if(itrAdapter->IsNull() || isFind)
            {
                list<Ref<LoggerAdapter> >::iterator itrAdapterErase = itrAdapter;
                --itrAdapter;
                _adapterList.erase(itrAdapterErase);
            }
            if(isFind)
                break;
        }
        return (*this);
    }
    /// 删除所有日志 
    void Release()
    {
        _logList.clear();
        _adapterList.clear();
    }
    /// 返回是否包含指定的日志记录器 
    bool Contains(const ILogger& log)
    {
        list<Ref<ILogger> >::iterator itr;
        for(itr = _logList.begin(); itr != _logList.end(); ++itr)
        {
            if(itr->IsNull())
            {
                list<Ref<ILogger> >::iterator itrErase = itr;
                --itr;
                _logList.erase(itrErase);
            }
            else
            {
                if((*itr) == log)
                    return true;
            }
        }
        return false;
    }
    bool Contains(const LoggerAdapter& log)
    {
        list<Ref<LoggerAdapter> >::iterator itrAdapter;
        for(itrAdapter = _adapterList.begin();
            itrAdapter != _adapterList.end();
            ++itrAdapter)
        {
            if(itrAdapter->IsNull())
            {
                list<Ref<LoggerAdapter> >::iterator itrAdapterErase = itrAdapter;
                --itrAdapter;
                _adapterList.erase(itrAdapterErase);
            }
            else
            {
                if((*itrAdapter) == log)
                    return true;
            }
        }
        return false;
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 写入行
     */
    template<class T>
    LoggerAdapter& WriteLine(const T& obj)
    {
        if(!IsEnable) return (*this);

        (*this)<<obj<<endl;
        return (*this);
    }
    /// 输出空行并刷新缓冲区  
    LoggerAdapter& WriteLine()
    {
        if(!IsEnable) return (*this);

        (*this) << '\n';
        (*this).Flush();
        return (*this);
    }
    /// 输出时间 
    LoggerAdapter& WriteTime()
    {
        if(!IsEnable) return (*this);

        (*this)<<current_systime<<endl;
        return (*this);
    }
    //@}
    //-----------------------------------------------------
    /// 输出字节数据
    LoggerAdapter& WriteHex(const ByteArray& data)
    {
        if(!IsEnable) return (*this);

        ByteBuilder buf;
        ByteConvert::ToHex(data, buf);
        // 每隔10个Hex换行
        const char* pBuf = buf.GetString();
        (*this) << "Len[" << data.GetLength() << "]\n";
        for(size_t i = 1;i <= buf.GetLength(); ++i)
        {
            (*this)<<pBuf[i - 1];
            if((i % 30) == 0)
                (*this)<<endl;
        }
        return (*this);
    }
    /// 以16进制格式输出数据(中间不带空格)
    LoggerAdapter& WriteStream(const ByteArray& data)
    {
        if(!IsEnable) return (*this);

        // 以16为一组转换数据 
        const size_t PKG_SIZE = 16;
        ByteBuilder buff(PKG_SIZE);
        size_t count = data.GetLength() / PKG_SIZE;
        for(size_t i = 0;i <= count; ++i)
        {
            buff.Clear();
            ByteConvert::ToAscii(data.SubArray(i * PKG_SIZE, PKG_SIZE), buff);
            (*this) << buff.GetString();
        }

        return (*this);
    }
    //----------------------------------------------------- 
    /// 刷新缓冲区
    void Flush()
    {
        list<Ref<ILogger> >::iterator itr;
        for(itr = _logList.begin(); itr != _logList.end(); ++itr)
        {
            if(itr->IsNull())
            {
                list<Ref<ILogger> >::iterator itrErase = itr;
                --itr;
                _logList.erase(itrErase);
            }
            else
            {
                (*itr)->Flush();
            }
        }
        list<Ref<LoggerAdapter> >::iterator itrAdapter;
        for(itrAdapter = _adapterList.begin();
            itrAdapter != _adapterList.end();
            ++itrAdapter)
        {
            if(itrAdapter->IsNull())
            {
                list<Ref<LoggerAdapter> >::iterator itrAdapterErase = itrAdapter;
                --itrAdapter;
                _adapterList.erase(itrAdapterErase);
            }
            else
            {
                (*itrAdapter)->Flush();
            }
        }
    }
    //----------------------------------------------------- 
};
//---------------------------------------------------------
} // namespace base
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LOGGER_H_
//=========================================================
