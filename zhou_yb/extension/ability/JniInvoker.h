//========================================================= 
/**@file JniInvoker.h
 * @brief Java Jni辅助对象 
 * 
 * @date 2015-07-09   15:03:17
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include <jni.h>

 #include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// Java Env生成器
class JniEnvInvoker
{
protected:
    shared_obj<JavaVM*> _jVM;
    shared_obj<JNIEnv*> _env;

    void _init()
    {
        _jVM.obj() = NULL;
        _env.obj() = NULL;
    }
public:
    JniEnvInvoker()
    {
        _init();
    }
    JniEnvInvoker(JNIEnv* env)
    {
        _init();
        Create(env);
    }
    virtual ~JniEnvInvoker()
    {
        Dispose();
    }

    bool Create(JNIEnv* env)
    {
        if(env == NULL)
            return false;
        /*
        JavaVM* jVM = NULL;
        env->GetJavaVM(&jVM);
        if(jVM == NULL)
            return false;

        #ifdef _WIN32
        void* pEnv = NULL;
        #else
        JNIEnv* pEnv = NULL;
        #endif
        
        jVM->AttachCurrentThread(&pEnv, NULL);
        if(pEnv == NULL)
            return false;

        _jVM.obj() = jVM;
        _env.obj() = reinterpret_cast<JNIEnv*>(pEnv);
        */
        _env.obj() = env;

        return true;
    }
    inline bool IsValid() const
    {
        //return _env.obj() != NULL && _jVM.obj() != NULL;
        return _env.obj() != NULL;
    }
    void Dispose()
    {
        // 只有自身拥有对象 
        if(IsValid() && _jVM.ref_count() < 2)
        {
            //_jVM.obj()->DetachCurrentThread();
            _jVM.obj() = NULL;
            _env.obj() = NULL;
        }
    }

    inline operator JNIEnv* () { return _env.obj(); }
    inline operator const JNIEnv* () const { return _env.obj(); }

    inline JNIEnv& operator* () { return *(_env.obj()); }
    inline JNIEnv* operator->() { return &(operator *()); }
};
/// Java数据格式转换器 
class JniConverter : public JniEnvInvoker
{
public:
    JniConverter() : JniEnvInvoker() {}
    JniConverter(JNIEnv* env) : JniEnvInvoker(env) {}

    size_t get_jbyteArray(jbyteArray _jbyteArray, size_t len, ByteBuilder& _cbyteArray)
    {
        size_t lastLen = _cbyteArray.GetLength();
        _cbyteArray.Append(static_cast<byte>(0x00), len);

        jbyte* jArray = reinterpret_cast<jbyte*>(const_cast<byte*>(_cbyteArray.GetBuffer(lastLen)));
        _env.obj()->GetByteArrayRegion(_jbyteArray, 0, len, jArray);
        return len;
    }
    inline size_t get_jbyteArray(jbyteArray _jbyteArray, ByteBuilder& _cbyteArray)
    {
        jsize len = _env.obj()->GetArrayLength(_jbyteArray);
        return get_jbyteArray(_jbyteArray, static_cast<size_t>(len), _cbyteArray);
    }
    size_t set_jbyteArray(const ByteArray& _cbyteArray, jbyteArray _jbyteArray)
    {
        jbyte* cArray = reinterpret_cast<jbyte*>(const_cast<byte*>(_cbyteArray.GetBuffer()));
        jsize clen = static_cast<jsize>(_cbyteArray.GetLength());
        jsize jlen = _env.obj()->GetArrayLength(_jbyteArray);
        clen = _min(clen, jlen);
        _env.obj()->SetByteArrayRegion(_jbyteArray, 0, clen, cArray);

        return clen;
    }
    size_t set_jintArray(const int* _cintArray, size_t _cintArrayLength, jintArray _jintArray)
    {
        jint* cArray = reinterpret_cast<jint*>(const_cast<int*>(_cintArray));
        jsize clen = static_cast<jsize>(_cintArrayLength);
        jsize jlen = _env.obj()->GetArrayLength(_jintArray);
        clen = _min(clen, jlen);
        _env.obj()->SetIntArrayRegion(_jintArray, 0, clen, cArray);

        return clen;
    }
    /// 获取UTF格式的字符串 
    inline jstring get_string(const char* str)
    {
        return _env.obj()->NewStringUTF(str);
    }
    /// 获取默认格式的字符串(Unicode)
    inline jstring get_string(const wchar_t* wstr)
    {
        wchar_t* p = const_cast<wchar_t*>(wstr);
        return _env.obj()->NewString(reinterpret_cast<const jchar*>(p), _wcslen(wstr));
    }
    /// 获取UTF格式的字符串 
    inline const char* get_string(jstring str)
    {
        jboolean isCopy = JNI_FALSE;
        return _env.obj()->GetStringUTFChars(str, &isCopy);
    }
    /// 获取默认格式的字符串(Unicode) 
    inline const wchar_t* get_wstring(jstring str)
    {
        jboolean isCopy = JNI_FALSE;
        jchar* p = const_cast<jchar*>(_env.obj()->GetStringChars(str, &isCopy));
        return reinterpret_cast<const wchar_t*>(p);
    }
    /// 将byte[]转换为字符串 
    inline const char* get_string(jbyteArray str)
    {
        jboolean isCopy = JNI_FALSE;
        jbyte* jArray = _env.obj()->GetByteArrayElements(str, &isCopy);
        return reinterpret_cast<const char*>(jArray);
    }
};
//--------------------------------------------------------- 
/// Java对象
class JniInvoker : public JniEnvInvoker
{
protected:
    /// Jni上层Java类对象
    jobject _obj;
    /// Jni上层类
    jclass _jcls;

    /// 初始化函数 
    inline void _init()
    {
        _obj = NULL;
        _jcls = NULL;
    }
public:
    JniInvoker() : JniEnvInvoker() { _init(); }
    JniInvoker(JNIEnv* env, jobject obj) : JniEnvInvoker()
    {
        _init();
        Create(env, obj);
    }
    virtual ~JniInvoker() { Dispose(); }

    bool Create(JNIEnv* env, jobject obj)
    {
        if(obj == NULL)
            return false;

        Dispose();

        if(!JniEnvInvoker::Create(env))
            return false;

        jobject refObj = _env.obj()->NewGlobalRef(obj);
        if(refObj == NULL)
        {
            JniEnvInvoker::Dispose();
            return false;
        }
        jclass jcls = _env.obj()->GetObjectClass(refObj);;
        if(jcls == NULL)
        {
            _env.obj()->DeleteGlobalRef(refObj);
            JniEnvInvoker::Dispose();
            return false;
        }
        
        _obj = refObj;
        _jcls = jcls;

        return true;
    }
    void Dispose()
    {
        // 只有自身拥有对象 
        if(IsValid() && _env.ref_count() < 2)
        {
            _env.obj()->DeleteGlobalRef(_obj);
            JniEnvInvoker::Dispose();
            
            _obj = NULL;
            _jcls = NULL;
        }
    }
    
    inline operator jclass () { return _jcls; }
    inline operator jobject () { return _obj; }
    inline operator const jclass () const { return _jcls; }
    inline operator const jobject () const { return _obj; }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension
} // namespace zhou_yb
//========================================================= 
