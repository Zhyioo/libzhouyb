//========================================================= 
/**@file LC_JniDriver.h
 * @brief LC Java Native接口驱动
 * 
 * @date 2016-04-02   11:12:31
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_JNIDRIVER_H_
#define _LIBZHOUYB_LC_JNIDRIVER_H_
//--------------------------------------------------------- 
#include "../driver/CommandDriver.h"

#include <extension/ability/JniInvoker.h>
using zhou_yb::extension::ability::JniInvoker;

#include <extension/ability/JniInvokerDevice.h>
using zhou_yb::extension::ability::JniInvokerDevice;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// JniEnv操作命令
class JniEnvCmdDriver : 
    public JniInvoker, 
    public JniInvokerDevice,
    public CommandCollection
{
public:
    JniEnvCmdDriver()
    {
        _Registe("JniEnvCreate", (*this), &JniEnvCmdDriver::JniEnvCreate);
        _Registe("JniEnvDispose", (*this), &JniEnvCmdDriver::JniEnvDispose);
    }
    
    /**
     * @brief 初始化JniEnv
     * 
     * @param [in] arglist 参数列表
     * - 参数:
     *  - JNIEnv 参数
     *  - jobject 参数
     * .
     */
    LC_CMD_METHOD(JniEnvCreate)
    {
        JNIEnv* env = reinterpret_cast<JNIEnv*>(arg["JNIEnv"].To<pointer>(NULL));
        jobject obj = reinterpret_cast<jobject>(arg["jobject"].To<pointer>(NULL));
        bool bCreate = JniInvoker::Create(env, obj);
        if(bCreate)
        {
            bCreate = JniInvokerDevice::Open(*this);
            if(!bCreate)
            {
                JniInvoker::Dispose();
            }
        }
        return bCreate;
    }
    /**
     * @brief 释放JniEnv
     * 
     * @param [in] arglist 参数列表(无)
     */
    LC_CMD_METHOD(JniEnvDispose)
    {
        JniInvoker::Dispose();
        JniInvokerDevice::Close();

        return true;
    }
};
//--------------------------------------------------------- 
/// Jni导出驱动辅助函数
struct JniDriverHelper
{
    /// 发送数据
    template<class TJniDriver>
    static jboolean Jni_Write(TJniDriver& drv, JNIEnv *env, jobject obj, jbyteArray sCmd, jint sLen)
    {
        JniConverter cvt(env);
        ByteBuilder sBuff(32);
        cvt.get_jbyteArray(sCmd, sLen, sBuff);
        return drv.Write(sBuff) ? JNI_TRUE : JNI_FALSE;
    }
    /// 接收数据
    template<class TJniDriver>
    static jboolean Jni_Read(TJniDriver& drv, JNIEnv *env, jobject obj, jbyteArray rCmd, jintArray rLen)
    {
        JniConverter cvt(env);
        ByteBuilder rBuff(32);
        if(!drv.Read(rBuff))
            return JNI_FALSE;
        cvt.set_jbyteArray(rBuff, rCmd);
        int len = static_cast<int>(rBuff.GetLength());
        cvt.set_jintArray(&len, 1, rLen);
        return JNI_TRUE;
    }
    /// 发送指令
    template<class TJniDriver>
    static jboolean Jni_TransmitCommand(TJniDriver& drv, JNIEnv *env, jobject obj, jstring sCmd, jstring sArg, jbyteArray sRecv)
    {
        JniConverter cvt(env);
        string cmd = cvt.get_string(sCmd);
        string arg = cvt.get_string(sArg);
        ByteBuilder recv(32);
        ByteBuilder sJniEnv(32);
        typename TJniDriver::ArgParserType argParser;
        argParser.PushValue("JNIEnv", ArgConvert::ToString<pointer>(env));
        argParser.PushValue("jboject", ArgConvert::ToString<pointer>(obj));
        argParser.ToString(sJniEnv);
        drv.TransmitCommand("JniEnvCreate", sJniEnv.GetString(), recv);
        recv.Clear();
        bool bCommand = drv.TransmitCommand(cmd.c_str(), ByteArray(arg.c_str(), arg.length()), recv);
        drv.TransmitCommand("JniEnvDispose", sJniEnv.GetString(), recv);
        if(bCommand)
        {
            cvt.set_jbyteArray(recv, sRecv);
            return JNI_TRUE;
        }
        return JNI_FALSE;
    }
};
//--------------------------------------------------------- 
/* 对应Java类声明如下:
package com.lc.driver;

public class LC_DriverInvoker {
    /// 初始化Native调用
    public native boolean nativeInit(String arg);
    /// 结束Native调用
    public native void nativeDestory();
    /// 与外设交互指令
    public native boolean TransmitCommand(String cmd, String send, byte[] recv);
    /// 获取上次失败的错误信息
    public native String getLastMessage();
    /// 获取上次失败的错误码
    public native int getLastErr();
    /// 打开设备
    public native boolean Open(String sArg);
    /// 返回设备是否已经打开
    public native boolean IsOpen();
    /// 关闭设备
    public native void Close();
    /// 发送指令
    public native boolean Write(byte[] sCmd, int sLen);
    /// 接收指令
    public native boolean Read(byte[] rCmd, int[] rLen);

    static {
        try {
            String projectPath = System.getProperty("user.dir");
            String libPath = projectPath + "/lcdriver.dll";
            System.load(libPath);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
*/
//--------------------------------------------------------- 
/// LC Driver的全局变量名
#define LC_JNI_ID(jnidriver) Java_com_lc_driver_##jnidriver
/// 输出返回结果
#define return_JBool(val) \
    {jboolean b=(val);_log<<"返回:<"<<(b==JNI_TRUE?"JNI_TRUE":"JNI_FALSE")<<">\n";return (b);}
/// 导出IBaseDevice接口
#define LC_EXPORT_IBaseDevice(jnidriver) \
    EXTERN_C JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_Open \
        (JNIEnv *env, jobject obj, jstring sArg) \
    { \
        LOG_FUNC_NAME(); \
        JniConverter cvt(env); \
        bool isOpen = LC_JNI_ID(jnidriver).Open(cvt.get_string(sArg)); \
        return_JBool(isOpen ? JNI_TRUE : JNI_FALSE); \
    } \
    EXTERN_C JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_IsOpen \
        (JNIEnv *env, jobject obj) \
    { \
        LOG_FUNC_NAME(); \
        return_JBool(LC_JNI_ID(jnidriver).IsOpen() ? JNI_TRUE : JNI_FALSE;) \
    } \
    EXTERN_C JNIEXPORT void JNICALL Java_com_lc_driver_LC_1DriverInvoker_Close \
        (JNIEnv *env, jobject obj) \
    { \
        LOG_FUNC_NAME(); \
        LC_JNI_ID(jnidriver).Close(); \
    }
/// 导出IInteractiveTrans接口
#define LC_EXPORT_IInteractiveTrans(jnidriver) \
    EXTERN_C JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_Write \
        (JNIEnv *env, jobject obj, jbyteArray sCmd, jint sLen) \
    { \
        LOG_FUNC_NAME(); \
        return_JBool(zhou_yb::application::driver::JniDriverHelper::Jni_Write<jnidriver>(LC_JNI_ID(jnidriver), env, obj, sCmd, sLen)); \
    } \
    EXTERN_C JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_Read \
        (JNIEnv *env, jobject obj, jbyteArray rCmd, jintArray rLen) \
    { \
        LOG_FUNC_NAME(); \
        return_JBool(zhou_yb::application::driver::JniDriverHelper::Jni_Read<jnidriver>(LC_JNI_ID(jnidriver), env, obj, rCmd, rLen)); \
    }
/// 导出指定的类为Jni形式的LC_Driver驱动
#define LC_JNI_EXPORT_DRIVER(jnidriver) \
    jnidriver LC_JNI_ID(jnidriver); \
    LOGGER(LoggerAdapter _log); \
    EXTERN_C JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_nativeInit \
        (JNIEnv *env, jobject obj, jstring sArg) \
    { \
        JniConverter cvt(env); \
        ByteBuilder recv(8); \
        bool bInit = LC_JNI_ID(jnidriver).TransmitCommand("NativeInit", cvt.get_string(sArg), recv); \
        LOGGER(_log = LC_JNI_ID(jnidriver).GetLogger()); \
        { LOG_FUNC_NAME(); } \
        return_JBool(bInit); \
    } \
    EXTERN_C JNIEXPORT void JNICALL Java_com_lc_driver_LC_1DriverInvoker_nativeDestory \
        (JNIEnv *, jobject) \
    { \
        { LOG_FUNC_NAME(); } \
        ByteBuilder recv(8); \
        LC_JNI_ID(jnidriver).TransmitCommand("NativeDestory", "", recv); \
    } \
    EXTERN_C JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_TransmitCommand \
        (JNIEnv *env, jobject obj, jstring sCmd, jstring sArg, jbyteArray sRecv) \
    { \
        LOG_FUNC_NAME(); \
        return_JBool(zhou_yb::application::driver::JniDriverHelper::Jni_TransmitCommand<jnidriver>(LC_JNI_ID(jnidriver), env, obj, sCmd, sArg, sRecv)); \
    } \
    EXTERN_C JNIEXPORT jstring JNICALL Java_com_lc_driver_LC_1DriverInvoker_getLastMessage \
        (JNIEnv *env, jobject) \
    { \
        JniConverter cvt(env); \
        return cvt.get_string(LC_JNI_ID(jnidriver).GetErrMessage()); \
    } \
    EXTERN_C JNIEXPORT jint JNICALL Java_com_lc_driver_LC_1DriverInvoker_getLastErr \
        (JNIEnv *, jobject) \
    { \
        return LC_JNI_ID(jnidriver).GetLastErr(); \
    }
//--------------------------------------------------------- 
} // namespace driver
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_JNIDRIVER_H_
//========================================================= 