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
#include "CommandDriver.h"

#include <extension/ability/JniInvoker.h>
using zhou_yb::extension::ability::JniInvoker;

#include <extension/ability/JavaIInteractiveTransInvoker.h>
using zhou_yb::extension::ability::JavaIInteractiveTransInvoker;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// JniEnv操作命令
class JniEnvCmdDriver : 
    public JniInvoker, 
    public JavaIInteractiveTransInvoker,
    public CommandCollection
{
public:
    JniEnvCmdDriver()
    {
        _Bind("JniEnvCreate", (*this), &JniEnvCmdDriver::JniEnvCreate);
        _Bind("JniEnvDispose", (*this), &JniEnvCmdDriver::JniEnvDispose);
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
            bCreate = JavaIInteractiveTransInvoker::Create(*this);
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
        JavaIInteractiveTransInvoker::Dispose();

        return true;
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
}
*/
//--------------------------------------------------------- 
/// LC Driver的全局变量名
#define LC_JNI_ID(jnidriver) Java_com_lc_driver_##jnidriver
/// 导出IBaseDevice接口
#define LC_EXPORT_IBaseDevice(jnidriver) \
    JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_Open \
        (JNIEnv *env, jobject obj, jstring sArg) \
    { \
        LOG_FUNC_NAME(); \
        JniConverter cvt(env); \
        bool isOpen = LC_JNI_ID(jnidriver).Open(cvt.get_string(sArg)); \
        return isOpen ? JNI_TRUE : JNI_FALSE; \
    } \
    JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_IsOpen \
        (JNIEnv *env, jobject obj) \
    { \
        LOG_FUNC_NAME(); \
        return LC_JNI_ID(jnidriver).IsOpen() ? JNI_TRUE : JNI_FALSE; \
    } \
    JNIEXPORT void JNICALL Java_com_lc_driver_LC_1DriverInvoker_Close \
        (JNIEnv *env, jobject obj) \
    { \
        LOG_FUNC_NAME(); \
        LC_JNI_ID(jnidriver).Close(); \
    }
/// 导出IInteractiveTrans接口
#define LC_EXPORT_IInteractiveTrans(jnidriver) \
    JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_Write \
        (JNIEnv *env, jobject obj, jbyteArray sCmd, jint sLen) \
    { \
        LOG_FUNC_NAME(); \
        JniConverter cvt(env); \
        ByteBuilder sBuff(32); \
        cvt.get_jbyteArray(sCmd, sLen, sBuff); \
        return LC_JNI_ID(jnidriver).Write(sBuff) ? JNI_TRUE : JNI_FALSE; \
    } \
    JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_Read \
        (JNIEnv *env, jobject obj, jbyteArray rCmd, jintArray rLen) \
    { \
        LOG_FUNC_NAME(); \
        JniConverter cvt(env); \
        ByteBuilder rBuff(32); \
        if(!LC_JNI_ID(jnidriver).Read(rBuff)) \
            return JNI_FALSE; \
        cvt.set_jbyteArray(rBuff, rCmd); \
        int len = static_cast<int>(rBuff.GetLength()); \
        cvt.set_jintArray(&len, 1, rLen); \
        return JNI_TRUE; \
    }
/// 导出指定的类为Jni形式的LC_Driver驱动
#define LC_EXPORT_DRIVER(jnidriver) \
    jnidriver LC_JNI_ID(jnidriver); \
    LOGGER(LoggerAdapter _log); \
    JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_nativeInit \
        (JNIEnv *env, jobject obj, jstring sArg) \
    { \
        JniConverter cvt(env); \
        ByteBuilder recv(8); \
        bool bInit = LC_JNI_ID(jnidriver).TransmitCommand("NativeInit", cvt.get_string(sArg), recv); \
        LOGGER(_log = LC_JNI_ID(jnidriver).GetLogger()); \
        { LOG_FUNC_NAME(); } \
        return bInit; \
    } \
    JNIEXPORT void JNICALL Java_com_lc_driver_LC_1DriverInvoker_nativeDestory \
        (JNIEnv *, jobject) \
    { \
        { LOG_FUNC_NAME(); } \
        ByteBuilder recv(8); \
        LC_JNI_ID(jnidriver).TransmitCommand("NativeDestory", "", recv); \
    } \
    JNIEXPORT jboolean JNICALL Java_com_lc_driver_LC_1DriverInvoker_TransmitCommand \
        (JNIEnv *env, jobject obj, jstring sCmd, jstring sArg, jbyteArray sRecv) \
    { \
        LOG_FUNC_NAME(); \
        JniConverter cvt(env); \
        string cmd = cvt.get_string(sCmd); \
        string arg = cvt.get_string(sArg); \
        LOGGER(_log << "Command:<" << cmd << ">\n" << "Arg:<" << arg << ">\n"); \
        ByteBuilder recv(32); \
        ByteBuilder sJniEnv(32); \
        sJniEnv += ArgConvert::ToString<pointer>(env).c_str(); \
        sJniEnv += SPLIT_STRING; \
        sJniEnv += ArgConvert::ToString<pointer>(obj).c_str(); \
        LC_JNI_ID(jnidriver).TransmitCommand("JniEnvCreate", sJniEnv.GetString(), recv); \
        recv.Clear(); \
        bool bCommand = LC_JNI_ID(jnidriver).TransmitCommand(cmd.c_str(), ByteArray(arg.c_str(), arg.length()), recv); \
        LC_JNI_ID(jnidriver).TransmitCommand("JniEnvDispose", sJniEnv.GetString(), recv); \
        LOGGER(_log << "Recv:<" << recv.GetString() << ">\n"); \
        LOGGER(_log << (bCommand ? "RET:JNI_TRUE" : "RET:JNI_FALSE") << endl); \
        if(bCommand) \
        { \
            cvt.set_jbyteArray(recv, sRecv); \
            return JNI_TRUE; \
        } \
        return JNI_FALSE; \
    } \
    JNIEXPORT jstring JNICALL Java_com_lc_driver_LC_1DriverInvoker_getLastMessage \
        (JNIEnv *env, jobject) \
    { \
        JniConverter cvt(env); \
        return cvt.get_string(LC_JNI_ID(jnidriver).GetErrMessage()); \
    } \
    JNIEXPORT jint JNICALL Java_com_lc_driver_LC_1DriverInvoker_getLastErr \
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