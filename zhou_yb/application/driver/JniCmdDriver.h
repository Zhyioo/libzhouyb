//========================================================= 
/**@file JniCmdDriver.h
 * @brief 
 * 
 * @date 2016-06-14   19:53:57
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_JNICMDDRIVER_H_
#define _LIBZHOUYB_JNICMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommandDriver.h"

#include <extension/ability/JniInvoker.h>
using zhou_yb::extension::ability::JniConverter;
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
    public JniInvokerDevice
{
public:
    JniEnvCmdDriver()
    {
        ArraySize = DEV_BUFFER_SIZE;
    }
    /// 初始化Jni调用
    virtual bool JniEnvCreate(JNIEnv* env, jobject obj)
    {
        bool bCreate = JniInvoker::Create(env, obj);
        if(bCreate)
        {
            bCreate = JniInvokerDevice::Open(*this, ArraySize);
            if(!bCreate)
            {
                JniInvoker::Dispose();
            }
        }
        return bCreate;
    }
    /// 缓冲区大小
    size_t ArraySize;
    /// 释放资源
    virtual void JniEnvDispose()
    {
        JniInvoker::Dispose();
        JniInvokerDevice::Close();
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
        drv.JniEnvCreate(env, obj);
        recv.Clear();
        bool bCommand = drv.TransmitCommand(cmd.c_str(), ByteArray(arg.c_str(), arg.length()), recv);
        drv.JniEnvDispose();
        if(bCommand)
        {
            cvt.set_jbyteArray(recv, sRecv);
            return JNI_TRUE;
        }
        return JNI_FALSE;
    }
};
//--------------------------------------------------------- 
/// Jni设备
template<class TArgParser, class TCmdDriver>
class JniDrvier : public CommandDriver<TArgParser>
{
protected:
    LOGGER(FolderLogger _folder);

    JniEnvCmdDriver _dev;
    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;

    TCmdDriver<TArgParser> _driver;
public:
    JniDrvier() : CommandDriver<TArgParser>()
    {
        _driver.SelectDevice(_dev);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_dev, "Jnidev");
        _lastErr.Select(_driver, "CmdDriver");
        _objErr.Invoke(this->_lasterr, this->_errinfo);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker), _dev, _driver;

        this->_Registe("NativeInit", (*this), &AndroidH002Driver::NativeInit);
        this->_Registe("NativeDestory", (*this), &AndroidH002Driver::NativeDestory);

        this->_Registe("EnumCommand", (*this), &AndroidH002Driver::EnumCommand);
        this->_Registe("LastError", (*this), &AndroidH002Driver::LastError);

        list<Ref<ComplexCommand> > cmds = _h002.GetCommand("");
        this->Registe(cmds);
    }
    LC_CMD_LASTERR(_lastErr);
    /**
     * @brief JNI初始化
     * @date 2016-06-14 20:38
     * 
     * @param [in] JNIEnv : pointer JniEnv虚拟器地址
     * @param [in] jobject : pointer Java上层对象地址
     */
    LC_CMD_METHOD(JniEnvCreate)
    {
        string sEnv = arg["JNIEnv"].To<string>();
        string sObj = arg["jobject"].To<string>();

        JNIEnv* env = reinterpret_cast<JNIEnv*>(ArgConvert::FromString<pointer>(sEnv));
        jobject obj = reinterpret_cast<jobject>(ArgConvert::FromString<pointer>(sObj));

        return _dev.JniEnvCreate(env, obj);
    }
    LC_CMD_METHOD(JniEnvDispose)
    {
        _dev.JniEnvDispose();
        return true;
    }
    /**
     * @brief 初始化JNI调用
     * @date 2016-06-09 10:57
     *
     * @param [in] Path : string 需要设置的日志目录
     */
    LC_CMD_METHOD(NativeInit)
    {
        LOGGER(string dir = arg["Path"].To<string>();
        _folder.Open(dir.c_str(), "driver", 2, FILE_K(256));
        CommandDriver<TArgParser>::_log.Select(_folder);
        _logInvoker.SelectLogger(CommandDriver<TArgParser>::_log));

        return true;
    }
    LC_CMD_METHOD(NativeDestory)
    {
        LOGGER(_folder.Close();
        CommandDriver<TArgParser>::_log.Release();
        _logInvoker.ReleaseLogger());

        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_JNICMDDRIVER_H_
//========================================================= 