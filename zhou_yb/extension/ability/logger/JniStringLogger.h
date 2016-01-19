//========================================================= 
/**@file JniStringLogger.h 
 * @brief Java下扩展的日志类(支持将日志打印操作回调到TextView控件中)
 *
 * 输出日志数据时:1.先回调通知Java类有新的日志数据产生 2.Java类调用相关接口获取输出的日志 
 * class XXInvoker {
       // 设置env和obj初始化回调 
       public void native nativeInit();

       public void logUpdate(String str) {
           if(this.textViewLogger != null) {
               this.textViewLogger.append(str);
           }
       }
 * 
 * @date 2013-11-07   22:52:31 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../JniInvoker.h"
#include "StringLogger.h"

#include <sstream>
using std::ostringstream;
//---------------------------------------------------------
namespace zhou_yb {
namespace extension {
namespace ability {
//---------------------------------------------------------
/**
 * @brief 将日志的输出操作通过一个回调函数进行回调  
 */
class JniStringLogger : public StringLogger
{
public:
    /// JniStringLogger.Open配置项参数主键 Env
    static const char EnvKey[4];
    /// JniStringLogger.Open配置项参数主键 Obj
    static const char ObjKey[4];
    /// JniStringLogger.Open配置项参数主键 Method
    static const char MethodKey[8];
protected:
    /// Java类的日志数据接收函数
    jmethodID _jniCallback;
    /// Jni上层Java类对象
    JniInvoker _jniInvoker;

    /// 输出后触发的动作
    virtual void _buf_after_changed()
    {
        // 最后一个字符为换行
        string str = StringLogger::String();
        StringLogger::Clear();

        if(str.length() < 1)
            return;

        jobject jObj = _jniInvoker;
        jstring jstr = _jniInvoker->NewStringUTF(str.c_str());
        _jniInvoker->CallVoidMethod(jObj, _jniCallback, jstr);
    }
    /// 初始化 
    void _init()
    {
        _jniCallback = NULL;
    }
public:
    JniStringLogger(JNIEnv* env, jobject obj, const char* methodID)
    {
        _os = &_ostr;
        _init();

        Open(env, obj, methodID);
    }
    JniStringLogger()
    {
        _os = &_ostr;
        _init();
    }
    virtual ~JniStringLogger()
    {
        Close();
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief IBaseDevice成员
     */
    /**
     * @brief 开启日志记录功能
     * @param [in] sArg [default:NULL]
     * - 参数格式:
     *  - NULL 根据上次的配置打开 
     *  - pointer EnvKey, pointer ObjKey, pointer MethodKey 配置项参数 
     * .
     */ 
    virtual bool Open(const char* sArg = NULL)
    {
        if(!_is_empty_or_null(sArg))
        {
            ArgParser cfg;

            JNIEnv* env = NULL;
            jobject obj = NULL;
            const char* methodID = NULL;

            cfg.Parse(sArg);
            pointer ptr = NULL;
            ArgConvert::FromConfig(cfg, EnvKey, ptr);
            env = reinterpret_cast<JNIEnv*>(ptr == NULL ? const_cast<char*>(sArg) : reinterpret_cast<char*>(ptr));

            ptr = NULL;
            ArgConvert::FromConfig(cfg, ObjKey, ptr);
            obj = reinterpret_cast<jobject>(ptr == NULL ? const_cast<char*>(sArg) : reinterpret_cast<char*>(ptr));

            ptr = NULL;
            ArgConvert::FromConfig(cfg, MethodKey, ptr);
            methodID = (ptr == NULL ? sArg : reinterpret_cast<const char*>(ptr));

            Open(env, obj, methodID);
        }
        
        return IsOpen();
    }
    /// 打开日志 
    bool Open(JNIEnv* env, jobject obj, const char* methodID)
    {
        Close();

        if((env == NULL) || (obj == NULL) || _is_empty_or_null(methodID))
            return false;

        JniInvoker jniInvoker;
        if(!jniInvoker.Create(env, obj))
            return false;

        jmethodID method = jniInvoker->GetMethodID(jniInvoker, methodID, "(Ljava/lang/String;)V");
        if(method == NULL)
            return false;

        _jniInvoker = jniInvoker;
        _jniCallback = method;

        return StringLogger::Open();
    }
    /// 返回是否打开 
    virtual bool IsOpen()
    {
        return _jniInvoker.IsValid() && StringLogger::IsOpen();
    }
    /// 销毁所占用资源  
    virtual void Close()
    {
        StringLogger::Close();
        _jniInvoker.Dispose();
    }
    /// 刷新缓冲区  
    virtual void Flush()
    {
        if(IsOpen())
        {
            StringLogger::Flush();
            _buf_after_changed();
        }
    }
    //@}
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//========================================================= 
