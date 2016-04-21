//========================================================= 
/**@file JniInvokerDevice.h
 * @brief 回调Java类实现 IInteractiveTrans 接口的封装类 
 * 
 * class XXInvoker {
        public native boolean Write(byte[] sCmd, int sLen);
        public native boolean Read(byte[] rCmd, int[] rLen);
    }
 *
 * 
 * @date 2015-08-18   20:36:04
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_JNIINVOKERDEVICE_H_
#define _LIBZHOUYB_JNIINVOKERDEVICE_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/BaseDevice.h"

#include "JniInvoker.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// Java回调句柄
struct JniInvokerHandler : public Handler<int>
{
    /// JniInvoker回调
    JniInvoker Invoker;
    /// Java类的接收函数
    jmethodID RecvCallback;
    /// Java类的发送函数
    jmethodID SendCallback;
    /// 发送缓冲区
    jbyteArray SendArray;
    /// 接收缓冲区
    jbyteArray RecvArray;
    /// 接收长度
    jintArray RecvLenArray;
};
//--------------------------------------------------------- 
/// JniInvokerHandler工厂类
class JniInvokerHandlerFactory : public IFactory<JniInvokerHandler>, public LoggerBehavior
{
public:
    //----------------------------------------------------- 
    /// JniInvokerHandlerFactory生成参数配置项时的主键 JniInvokerKey
    static const char JniInvokerKey[16];
    /// JniInvokerHandlerFactory生成参数配置项时的主键 ArraySizeKey
    static const char ArraySizeKey[16];
    //----------------------------------------------------- 
    /// 创建句柄
    virtual bool Create(JniInvokerHandler& handle, const char* sArg = NULL)
    {
        ArgParser cfg;
        cfg.Parse(sArg);
        pointer jniPtr = NULL;
        if(!ArgConvert::FromConfig<pointer>(cfg, JniInvokerHandlerFactory::JniInvokerKey, jniPtr))
            return false;
        if(jniPtr == NULL)
            return false;

        size_t arraySize = DEV_BUFFER_SIZE;
        ArgConvert::FromConfig<size_t>(cfg, JniInvokerHandlerFactory::ArraySizeKey, arraySize);

        JniInvoker* pJniInvoker = reinterpret_cast<JniInvoker*>(jniPtr);
        return Create(handle, (*pJniInvoker), arraySize);
    }
    /// 创建句柄
    bool Create(JniInvokerHandler& handle, JniInvoker& jniInvoker, size_t arraySize = DEV_BUFFER_SIZE)
    {
        if(!jniInvoker.IsValid())
        {
            LOGGER(_log.WriteLine("JniInvoker无效"));
            return false;
        }
        LOGGER(_log.WriteLine("GetMethodID..."));
        jmethodID sendMethod = jniInvoker->GetMethodID(jniInvoker, "Write", "([BI)Z");
        jmethodID recvMethod = jniInvoker->GetMethodID(jniInvoker, "Read", "([B[I)Z");
        if(sendMethod == NULL || recvMethod == NULL)
        {
            LOGGER(_log.WriteLine("回调指针句柄为NULL"));
            return false;
        }

        LOGGER(_log.WriteLine("GetArray..."));
        jbyteArray sendArray = jniInvoker->NewByteArray(arraySize);
        jbyteArray recvArray = jniInvoker->NewByteArray(arraySize);
        jintArray recvLenArray = jniInvoker->NewIntArray(2);

        if(sendArray == NULL || recvArray == NULL || recvLenArray == NULL)
        {
            if(sendArray) jniInvoker->DeleteLocalRef(sendArray);
            if(recvArray) jniInvoker->DeleteLocalRef(recvArray);
            if(recvLenArray) jniInvoker->DeleteLocalRef(recvLenArray);

            LOGGER(_log.WriteLine("申请缓冲区失败"));
            return false;
        }

        handle.Invoker = jniInvoker;
        handle.SendArray = sendArray;
        handle.RecvArray = recvArray;
        handle.RecvLenArray = recvLenArray;
        handle.RecvCallback = NULL;
        handle.SendCallback = NULL;

        return true;
    }
    /// 返回句柄是否有效
    virtual bool IsValid(const JniInvokerHandler& obj)
    {
        return obj.Invoker.IsValid();
    }
    /// 释放句柄
    virtual void Dispose(JniInvokerHandler& obj)
    {
        if(obj.SendArray != NULL)
            obj.Invoker->DeleteLocalRef(obj.SendArray);
        if(obj.RecvArray != NULL)
            obj.Invoker->DeleteLocalRef(obj.RecvArray);
        if(obj.RecvLenArray != NULL)
            obj.Invoker->DeleteLocalRef(obj.RecvLenArray);
        obj.Invoker.Dispose();

        obj.SendArray = NULL;
        obj.RecvArray = NULL;
        obj.RecvLenArray = NULL;
        obj.SendCallback = NULL;
        obj.RecvCallback = NULL;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// Java回调句柄读取器
class JniInvokerHandlerReader : public IHandlerReader
{
protected:
    /// 读取句柄 
    Ref<JniInvokerHandler> _handle;
public:
    JniInvokerHandlerReader(const JniInvokerHandler& handle) { _handle = handle; }
    /// 开始读数据 
    virtual bool Async(uint timeoutMs)
    {
        return true;
    }
    /// 读数据 
    virtual size_t Wait(ByteBuilder& data)
    {
        jvalue args[2];
        args[0].l = _handle->RecvArray;
        args[1].l = _handle->RecvLenArray;

        jobject jObj = _handle->Invoker;
        jboolean bRet = _handle->Invoker->CallBooleanMethodA(jObj, _handle->RecvCallback, args);

        if(bRet != JNI_TRUE)
            return 0;

        jint buf[2];
        _handle->Invoker->GetIntArrayRegion(_handle->RecvLenArray, 0, 2, buf);
        JniConverter(_handle->Invoker).get_jbyteArray(_handle->RecvArray, buf[0], data);

        return SIZE_EOF;
    }
};
//--------------------------------------------------------- 
/// Java回调句柄写入器
class JniInvokerHandlerWriter : public IHandlerWriter
{
protected:
    /// 写入句柄 
    Ref<JniInvokerHandler> _handle;
    /// 待写入数据的长度
    size_t _datalen;
public:
    JniInvokerHandlerWriter(const JniInvokerHandler& handle) { _handle = handle; }
    /// 开始写数据 
    virtual bool Async(const ByteArray& data, uint timeoutMs)
    {
        JniConverter(_handle->Invoker).set_jbyteArray(data, _handle->SendArray);
        _datalen = data.GetLength();
        return true;
    }
    /// 写数据 
    virtual size_t Wait()
    {
        jvalue args[2];
        
        args[0].l = _handle->SendArray;
        args[1].i = _datalen;

        jobject jObj = _handle->Invoker;
        jboolean bRet = _handle->Invoker->CallBooleanMethodA(jObj, _handle->SendCallback, args);
        
        return bRet == JNI_TRUE ? _datalen : 0;
    }
};
//--------------------------------------------------------- 
/// JniInvoker回调设备
class JniInvokerDevice : 
    public HandlerDevice<
        HandlerBaseDevice<JniInvokerHandler, JniInvokerHandlerFactory>,
        JniInvokerHandlerReader,
        JniInvokerHandlerWriter>
{
public:
    //----------------------------------------------------- 
    /// 打开设备
    bool Open(JniInvoker& jniInvoker, size_t arraySize = DEV_BUFFER_SIZE)
    {
        LOG_FUNC_NAME();
        // 关闭之前打开的设备 
        Close();
        ASSERT_FuncErrInfoRet(_hFactory.Create(_hDev, jniInvoker, arraySize), DeviceError::DevOpenErr, "创建设备Handle失败");
        LOGGER(_log << "Handler:<" << _hex_num(_hDev.Handle) << ">\n");
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_JNIINVOKERDEVICE_H_
//========================================================= 
