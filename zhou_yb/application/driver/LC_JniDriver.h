//========================================================= 
/**@file LC_JniDriver.h
 * @brief LC Java Native�ӿ�����
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
/// JniEnv��������
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
     * @brief ��ʼ��JniEnv
     * 
     * @param [in] arglist �����б�
     * - ����:
     *  - JNIEnv ����
     *  - jobject ����
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
     * @brief �ͷ�JniEnv
     * 
     * @param [in] arglist �����б�(��)
     */
    LC_CMD_METHOD(JniEnvDispose)
    {
        JniInvoker::Dispose();
        JavaIInteractiveTransInvoker::Dispose();

        return true;
    }
};
//--------------------------------------------------------- 
/* ��ӦJava����������:
package com.lc.driver;

public class LC_DriverInvoker {
    /// ��ʼ��Native����
    public native boolean nativeInit(String arg);
    /// ����Native����
    public native void nativeDestory();
    /// �����轻��ָ��
    public native boolean TransmitCommand(String cmd, String send, byte[] recv);
    /// ��ȡ�ϴ�ʧ�ܵĴ�����Ϣ
    public native String getLastMessage();
    /// ��ȡ�ϴ�ʧ�ܵĴ�����
    public native int getLastErr();
    /// ���豸
    public native boolean Open(String sArg);
    /// �����豸�Ƿ��Ѿ���
    public native boolean IsOpen();
    /// �ر��豸
    public native void Close();
    /// ����ָ��
    public native boolean Write(byte[] sCmd, int sLen);
    /// ����ָ��
    public native boolean Read(byte[] rCmd, int[] rLen);
}
*/
//--------------------------------------------------------- 
/// LC Driver��ȫ�ֱ�����
#define LC_JNI_ID(jnidriver) Java_com_lc_driver_##jnidriver
/// ����IBaseDevice�ӿ�
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
/// ����IInteractiveTrans�ӿ�
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
/// ����ָ������ΪJni��ʽ��LC_Driver����
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