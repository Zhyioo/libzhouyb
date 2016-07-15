//========================================================= 
/**@file Handler.h
 * @brief 句柄托管 
 * 
 * @date 2015-10-06   16:07:07
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HANDLER_H_
#define _LIBZHOUYB_HANDLER_H_
//--------------------------------------------------------- 
#include "../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
//---------------------------------------------------------
/// 句柄封装,提供基本的操作 
template<class T>
struct Handler : public RefObject
{
    /// 句柄数据类型 
    typedef T HandlerType;
    /// 句柄数据 
    T Handle;
    /// 将句柄结构初始化为 0
    Handler() { memset(&Handle, 0, sizeof(T)); }
};
//--------------------------------------------------------- 
/// 句柄读接口 
struct IHandlerReader
{
    /**
     * @brief 开始读
     * 
     * @param [in] timeoutMs 超时时间(ms)
     */
    virtual bool Async(uint timeoutMs) = 0;
    /**
     * @brief 等待读取数据
     * 
     * @param [in] data 获取到的数据 
     * @param [in] rlen 期望读取的数据长度 
     * 
     * @return size_t 每次读取到的数据长度
     * @retval 0 读取失败 
     * @retval SIZE_EOF 已经读取完毕 
     */
    virtual size_t Wait(ByteBuilder& data) = 0;
};
/// 句柄写接口 
struct IHandlerWriter
{
    /**
     * @brief 开始写
     * 
     * @param [in] data 需要发送的数据
     * @param [in] timeoutMs 操作的超时时间
     */
    virtual bool Async(const ByteArray& data, uint timeoutMs) = 0;
    /**
     * @brief 等待写入完成
     * 
     * @return size_t 每次成功写入的数据长度 
     */
    virtual size_t Wait() = 0;
};
//--------------------------------------------------------- 
/**
 * @brief 基于句柄的基础设备
 *
 * 通过HandlerFactory封装Handler实现IBaseDevice接口
 *
 * @param THandler 设备句柄类型
 * @param THandlerFactory 创建句柄的工厂类型
 */
template<class THandler, class THandlerFactory>
class HandlerBaseDevice :
    public IBaseDevice,
    public DeviceBehavior,
    public TimeoutBehavior,
    public InterruptBehavior,
    public RefObject
{
    /// 禁用拷贝构造函数 
    UnenableObjConsturctCopyBehavior(HandlerBaseDevice);
protected:
    //----------------------------------------------------- 
    /// 设备句柄 
    THandler _hDev;
    /// 设备句柄工厂类 
    THandlerFactory _hFactory;
    //----------------------------------------------------- 
    /// 读取数据
    bool Read(IHandlerReader& reader, ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Handler:<" << _hex_num(_hDev.Handle) << ">\n";
        _log << "超时时间:<" << _waitTimeout << "ms>,操作间隔:<" << _waitInterval << "ms>\n");

        ASSERT_FuncErrInfoRet(IsOpen(), DeviceError::DevStateErr, "设备未打开");

        LOGGER(_log.WriteLine("AsyncRead..."));
        if(!reader.Async(_waitInterval))
        {
            _logErr(DeviceError::OperatorErr, "AsyncRead失败");
            // 读取到0个字符 
            return _logRetValue(false);
        }
        /* 处理超时 */
        Timer timer;
        size_t rlen = 0;
        size_t readCount = 0;
        size_t errCount = 0;
        // 读取间隔(成功时不延时) 
        uint sleepTime = 0;
        bool bRead = false;
        size_t lastLen = data.GetLength();
        while(timer.Elapsed() < _waitTimeout)
        {
            if(!IsOpen())
            {
                _logErr(DeviceError::DevAlreadyCloseErr);
                break;
            }
            // 每隔_waitTimeout一次轮询是否读完 
            rlen = reader.Wait(data);
            if(rlen == SIZE_EOF)
            {
                bRead = true;
                break;
            }
            if(rlen < 1)
            {
                // 一直读取数据直到超时或读到数据但超过错误尝试次数 
                if(readCount > 0)
                {
                    ++errCount;
                    if(errCount >= DEV_RETRY_COUNT)
                    {
                        _logErr(DeviceError::RecvErr, "WaitRead失败");
                        // 已经读完数据 
                        bRead = true;
                        break;
                    }
                }
                sleepTime = _waitInterval;
            }
            else
            {
                // 第一次读到数据
                LOGGER(if(readCount < 1) _log<<"Timer:<"<<timer.Elapsed()<<"ms>...\n");
                LOGGER(if(rlen > 0) _log << "WaitRead rLen:<" << rlen << ">\n");
                // 重置连续错误次数
                errCount = 0;
                readCount += rlen;
                sleepTime = 0;
            }
            /* 中断支持 */
            if(InterruptBehavior::Implement(*this))
            {
                _logErr(DeviceError::OperatorInterruptErr);
                break;
            }
            Timer::Wait(sleepTime);
        }

        if(!bRead)
        {
            if(timer.Elapsed() >= _waitTimeout)
            {
                _logErr(DeviceError::WaitTimeOutErr);
            }
            _logErr(DeviceError::RecvErr);
            // 删除已经读取的垃圾数据 
            data.RemoveTail(data.GetLength() - lastLen);
        }
        LOGGER(else
        {
            _log.WriteLine("读到数据:").WriteLine(data.SubArray(lastLen));
        });
        return _logRetValue(bRead);
    }
    /// 写入数据
    bool Write(IHandlerWriter& writer, const ByteArray& data)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Handler:<" << _hex_num(_hDev.Handle) << ">\n";
        _log << "超时时间:<" << _waitTimeout << "ms>,操作间隔:<" << _waitInterval << "ms>\n";
        _log.WriteLine("写入数据:");
        _log.WriteLine(data));

        ASSERT_FuncErrInfoRet(IsOpen(), DeviceError::DevStateErr, "设备未打开");

        LOGGER(_log.WriteLine("AsyncWrite..."));
        /* 开始异步写数据 */
        if(!writer.Async(data, _waitInterval))
        {
            _logErr(DeviceError::OperatorErr, "AsyncWrite失败");
            return _logRetValue(false);
        }

        Timer timer;
        size_t writeCount = 0;
        size_t slen = 0;
        size_t errCount = 0;
        // 写入间隔(成功时不延时) 
        uint sleepTime = 0;
        bool bWrite = false;

        while(timer.Elapsed() < _waitTimeout)
        {
            if(!IsOpen())
            {
                _logErr(DeviceError::DevAlreadyCloseErr);
                break;
            }
            // 每隔_waitTimeout一次轮询是否写完 
            slen = writer.Wait();
            // 已经发送完
            if(slen == SIZE_EOF)
            {
                bWrite = true;
                break;
            }

            if(slen < 1)
            {
                ++errCount;
                if(errCount >= DEV_RETRY_COUNT)
                {
                    _logErr(DeviceError::SendErr, "WaitWrite失败");
                    break;
                }
                sleepTime = _waitInterval;
            }
            else
            {
                // 第一次发送数据
                LOGGER(if(writeCount < 1) _log<<"Timer:<"<<timer.Elapsed()<<"ms>...\n");
                LOGGER(if(slen > 0) _log << "WaitWrite sLen=<" << slen << ">\n");
                // 重置连续错误次数
                errCount = 0;

                writeCount += slen;
                // 数据已经全部发送完 
                if(writeCount >= data.GetLength())
                {
                    bWrite = true;
                    break;
                }
                sleepTime = 0;
            }
            /* 中断支持 */
            if(InterruptBehavior::Implement(*this))
            {
                _logErr(DeviceError::OperatorInterruptErr);
                break;
            }
            Timer::Wait(sleepTime);
        }
        if(!bWrite)
        {
            if(timer.Elapsed() >= _waitTimeout)
            {
                _logErr(DeviceError::WaitTimeOutErr);
            }
            _logErr(DeviceError::SendErr);
        }
        return _logRetValue(bWrite);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    HandlerBaseDevice() : DeviceBehavior() { LOGGER(_hFactory.SelectLogger(_log)); }
    virtual ~HandlerBaseDevice() { Close(); }
    //----------------------------------------------------- 
    /// 打开设备
    virtual bool Open(const char* sArg = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "sArg:<" << _strput(sArg) << ">\n");

        // 关闭之前打开的设备 
        Close();

        ASSERT_FuncErrInfoRet(_hFactory.Create(_hDev, sArg), DeviceError::DevOpenErr, "创建设备Handle失败");
        LOGGER(_log << "Handler:<" << _hex_num(_hDev.Handle) << ">\n");
        return _logRetValue(true);
    }
    /// 返回设备是否打开
    virtual bool IsOpen()
    {
        return _hFactory.IsValid(_hDev);
    }
    /// 关闭设备
    virtual void Close()
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Handler:<" << _hex_num(_hDev.Handle) << ">\n");
        if(IsOpen())
        {
            _hFactory.Dispose(_hDev);
        }
        _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 读到数据立即返回的适配器 
class HandlerEasyReader : public IHandlerReader
{
protected:
    IHandlerReader* _pReader;
public:
    HandlerEasyReader(IHandlerReader& reader)
    {
        _pReader = &reader;
    }
    virtual bool Async(uint timeoutMs)
    {
        return _pReader->Async(timeoutMs);
    }
    virtual size_t Wait(ByteBuilder& data)
    {
        size_t rlen = _pReader->Wait(data);
        if(rlen > 0)
            return SIZE_EOF;
        return rlen;
    }
};
//--------------------------------------------------------- 
/**
 * @brief 句柄读写功能扩展类
 * 
 * @param THandlerDevice 带有DeviceBehavior的句柄设备类 
 * @param THandlerReader 句柄读取器
 * @param THandlerWriter 句柄写入器
 */
template<class THandlerDevice, class THandlerReader, class THandlerWriter>
class HandlerDevice : public THandlerDevice, public IInteractiveTrans
{
public:
    //----------------------------------------------------- 
    HandlerDevice() : THandlerDevice() {}
    //----------------------------------------------------- 
    /// 读数据(读到任何数据立即返回)
    virtual bool Read(ByteBuilder& data)
    {
        THandlerReader reader(THandlerDevice::_hDev);
        HandlerEasyReader easyReader(reader);
        return THandlerDevice::Read(easyReader, data);
    }
    /// 写数据
    virtual bool Write(const ByteArray& data)
    {
        THandlerWriter writer(THandlerDevice::_hDev);
        return THandlerDevice::Write(writer, data);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HANDLER_H_
//========================================================= 
