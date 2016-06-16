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
#include "CommonCmdDriver.h"

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
        typename TJniDriver::ArgParserType argParser;
        argParser.PushValue("JniEnv", ArgConvert::ToString<pointer>(reinterpret_cast<pointer>(env)));
        argParser.PushValue("jobject", ArgConvert::ToString<pointer>(reinterpret_cast<pointer>(obj)));
        argParser.ToString(sJniEnv);
        drv.TransmitCommand("JniEnvCreate", sJniEnv, recv);
        recv.Clear();
        bool bCommand = drv.TransmitCommand(cmd.c_str(), ByteArray(arg.c_str(), arg.length()), recv);
        ByteBuilder tmp(8);
        drv.TransmitCommand("JniEnvDispose", sJniEnv, tmp);
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
template<class TCmdDriver>
class JniDriver : public CommandDriver<typename TCmdDriver::ArgParserType>
{
protected:
    LOGGER(FolderLogger _folder);

    JniEnvCmdDriver _dev;
    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;

    TCmdDriver _driver;
public:
    JniDriver() : CommandDriver<typename TCmdDriver::ArgParserType>()
    {
        _dev.ArraySize = 1024;
        _driver.SelectDevice(_dev);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_dev, "Jnidev");
        _lastErr.Select(_driver, "CmdDriver");
        _objErr.Invoke(this->_lasterr, this->_errinfo);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker), _dev, _driver;

        this->_Registe("NativeInit", (*this), &JniDriver::NativeInit);
        this->_Registe("NativeDestory", (*this), &JniDriver::NativeDestory);

        this->_Registe("EnumCommand", (*this), &JniDriver::EnumCommand);
        this->_Registe("LastError", (*this), &JniDriver::LastError);

        this->_Registe("JniEnvCreate", (*this), &JniDriver::JniEnvCreate);
        this->_Registe("JniEnvDispose", (*this), &JniDriver::JniEnvDispose);
        this->_Registe("SetArraySize", (*this), &JniDriver::SetArraySize);
        this->_Registe("SetTimeoutMS", (*this), &JniDriver::SetTimeoutMS);

        list<Ref<ComplexCommand> > cmds = _driver.GetCommand("");
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
     * @brief 设置JNI调用时的缓冲区大小
     * @date 2016-06-14 20:38
     * 
     * @param [in] ArraySize : uint 缓冲区大小
     */
    LC_CMD_METHOD(SetArraySize)
    {
        _dev.ArraySize = arg["ArraySize"].To<uint>(DEV_BUFFER_SIZE);
        return true;
    }
    /**
     * @brief 设置JNI调用时超时时间
     * @date 2016-06-14 20:38
     * 
     * @param [in] Timeout : uint 超时时间(ms)
     */
    LC_CMD_METHOD(SetTimeoutMS)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        _dev.SetWaitTimeout(timeoutMs);
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
        CommandDriver<typename TCmdDriver::ArgParserType>::_log.Select(_folder);
        _logInvoker.SelectLogger(CommandDriver<typename TCmdDriver::ArgParserType>::_log));

        return true;
    }
    LC_CMD_METHOD(NativeDestory)
    {
        LOGGER(_folder.Close();
        CommandDriver<typename TCmdDriver::ArgParserType>::_log.Release();
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
