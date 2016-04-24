//========================================================= 
/**@file JavaIInteractiveTransInvoker.h
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
#ifndef _LIBZHOUYB_JAVAIINTERACTIVETRANSINVOKER_H_
#define _LIBZHOUYB_JAVAIINTERACTIVETRANSINVOKER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/BaseDevice.h"

#include "JniInvoker.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 回调Java类实现IInteractiveTrans接口的封装类(建议当作局部变量来使用)
class JavaIInteractiveTransInvoker :
    public IInteractiveTrans,
    public DeviceBehavior,
    public InterruptBehavior,
    public RefObject
{
protected:
    ByteBuilder _tmpBuffer;

    /// Java类的接收函数
    jmethodID _recvCallback;
    /// Java类的发送函数
    jmethodID _sendCallback;
    /// Jni对象
    JniInvoker _jniInvoker;
    /// 发送缓冲区
    jbyteArray _sendArray;
    /// 接收缓冲区
    jbyteArray _recvArray;
    /// 接收长度
    jintArray _recvLenArray;

    /// 初始化
    inline void _init()
    {
        _recvCallback = NULL;
        _sendCallback = NULL;

        _sendArray = NULL;
        _recvArray = NULL;
        _recvLenArray = NULL;
    }
public:
    JavaIInteractiveTransInvoker()
    {
        _init();
    }
    virtual ~JavaIInteractiveTransInvoker()
    {
        Dispose();
    }
    /// 创建引用
    bool Create(JniInvoker& jniInvoker, size_t arraySize = DEV_BUFFER_SIZE)
    {
        LOG_FUNC_NAME();
        ASSERT_DeviceValid(jniInvoker.IsValid());

        LOGGER(_log.WriteLine("GetMethodID..."));
        jmethodID sendMethod = jniInvoker->GetMethodID(jniInvoker, "Write", "([BI)Z");
        jmethodID recvMethod = jniInvoker->GetMethodID(jniInvoker, "Read", "([B[I)Z");
        if(sendMethod == NULL || recvMethod == NULL)
        {
            _logErr(DeviceError::OperatorStatusErr, "回调指针为NULL");
            return _logRetValue(false);
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

            _logErr(DeviceError::OperatorStatusErr, "申请缓冲区失败");
            return _logRetValue(false);
        }

        Dispose();

        _jniInvoker = jniInvoker;
        _sendCallback = sendMethod;
        _recvCallback = recvMethod;

        _sendArray = sendArray;
        _recvArray = recvArray;
        _recvLenArray = recvLenArray;

        return _logRetValue(true);
    }
    /// 返回对象是否有效
    inline bool IsValid() const
    {
        return _jniInvoker.IsValid();
    }
    /// 释放相关资源
    void Dispose()
    {
        LOG_FUNC_NAME();
        if(IsValid())
        {
            LOGGER(_log.WriteLine("DeleteLocalRef..."));
            if(_sendArray != NULL)
                _jniInvoker->DeleteLocalRef(_sendArray);
            if(_recvArray != NULL)
                _jniInvoker->DeleteLocalRef(_recvArray);
            if(_recvLenArray != NULL)
                _jniInvoker->DeleteLocalRef(_recvLenArray);
            _jniInvoker.Dispose();
            
            _init();
        }
        LOGGER(_logRetValue(true));
    }
    /// 读数据
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        jvalue args[2];
        args[0].l = _recvArray;
        args[1].l = _recvLenArray;

        jobject jObj = _jniInvoker;
        jboolean bRet = _jniInvoker->CallBooleanMethodA(jObj, _recvCallback, args);

        if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
        {
            _logErr(DeviceError::OperatorInterruptErr);
            return _logRetValue(false);
        }
        ASSERT_FuncErrRet(bRet == JNI_TRUE, DeviceError::RecvErr);

        jint buf[2];
        _jniInvoker->GetIntArrayRegion(_recvLenArray, 0, 2, buf);
        JniConverter(_jniInvoker).get_jbyteArray(_recvArray, buf[0], data);

        LOGGER(_log.WriteLine("RECV:").WriteLine(data));
        return _logRetValue(true);
    }
    /// 写数据
    virtual bool Write(const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("SEND:").WriteLine(data));

        jvalue args[2];
        JniConverter(_jniInvoker).set_jbyteArray(data, _sendArray);
        args[0].l = _sendArray;
        args[1].i = data.GetLength();

        jobject jObj = _jniInvoker;
        jboolean bRet = _jniInvoker->CallBooleanMethodA(jObj, _sendCallback, args);

        if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
        {
            _logErr(DeviceError::OperatorInterruptErr);
            return _logRetValue(false);
        }
        ASSERT_FuncErrRet(bRet == JNI_TRUE, DeviceError::SendErr);
        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_JAVAIINTERACTIVETRANSINVOKER_H_
//========================================================= 
