//========================================================= 
/**@file ComDevice.h 
 * @brief Linux下串口设备
 * 
 * @date 2012-09-07   14:48:24 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//---------------------------------------------------------  
#include "FileDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// 串口设备类
template<class THandlerDevice>
class ComHandlerAppender : public THandlerDevice
{
protected:
    //-----------------------------------------------------
    /// 设置前的串口参数
    struct termios _oldTermio;
    /// 设置后的串口参数
    struct termios _newTermio;
    /// 标记外部是否设置郭 termio
    bool _hasTermio;
    //-----------------------------------------------------
    /// 初始化成员
    inline void _init()
    {
        _hasTermio = false;
    }
    //-----------------------------------------------------
public:
    //----------------------------------------------------- 
    ComHandlerAppender() : THandlerDevice() { _init(); }
    virtual ~ComHandlerAppender() { Close(); }
    //-----------------------------------------------------
    /**
     * @brief 打开设备(直接打开,sArg不能为空)
     * @param [in] sArg [default:NULL]
     * - 参数格式:
     *  - NULL 根据上次的配置打开
     *  - 设备或文件的路径
     *  - string FileBaseDevice::DevPathKey 文件或设备路径配置项参数
     * .
     */
    virtual bool Open(const char* sArg = NULL)
    {
        LOGGER(LoggerAdapter _log = THandlerDevice::_log);
        LOG_FUNC_NAME();
        ASSERT_FuncRet(THandlerDevice::Open(sArg));

        int iRet = tcgetattr(THandlerDevice::_hDev.Handle, &_oldTermio);
        if(iRet >= 0)
        {
            if(!_hasTermio)
            {
                memcpy(&_newTermio, &_oldTermio, sizeof(struct termios));

                _newTermio.c_lflag &= ~(ISIG|ECHO|ICANON|NOFLSH);
                _newTermio.c_iflag &= ~(IXON|INLCR|IGNCR|ICRNL|ISTRIP);
                _newTermio.c_oflag &= ~OPOST;

                _newTermio.c_cc[VMIN] = 0;
                _newTermio.c_cc[VTIME] = 1;
                _newTermio.c_cc[VQUIT] = 0x7f;

                _newTermio.c_cflag &= ~CBAUD;
                _newTermio.c_cflag &= ~CSIZE;
                _newTermio.c_cflag |= B9600;
                _newTermio.c_cflag |= CS8;
                _newTermio.c_cflag |= (CLOCAL|CREAD);
                _newTermio.c_cflag &= ~(PARENB|CSTOPB);

                _hasTermio = true;
            }

            iRet = tcsetattr(THandlerDevice::_hDev.Handle, TCSADRAIN, &_newTermio);
        }
        if(iRet < 0)
        {
            THandlerDevice::Close();
            return _logRetValue(false);
        }

        return _logRetValue(true);
    }
    /// 关闭设备
    virtual void Close()
    {
        LOGGER(LoggerAdapter _log = THandlerDevice::_log);
        LOG_FUNC_NAME();
        if(IsOpen())
        {
            // 重置串口属性 
            if(_hasTermio)
            {
                tcsetattr(THandlerDevice::_hDev.Handle, TCSANOW, &_oldTermio);
                _hasTermio = false;
            }
            THandlerDevice::Close();
        }
        _logRetValue(true);
    }
    /// 获取串口属性
    inline void GetTermio(struct termios* termio) const
    {
        if(termio != NULL)
            memcpy(termio, &_newTermio, sizeof(struct termios));
    }
    /// 设置串口属性
    inline void SetTermio(struct termios* termio)
    {
        LOGGER(LoggerAdapter _log = THandlerDevice::_log);
        LOG_FUNC_NAME();
        if(termio != NULL)
        {
            _hasTermio = true;
            memcpy(&_newTermio, termio, sizeof(struct termios));

            // 如果当前设备已经打开则重新设置属性
            if(IsOpen()) tcsetattr(THandlerDevice::_hDev.Handle, TCSADRAIN, &_newTermio);
        }
        else
        {
            _hasTermio = false;
        }

        LOGGER(_logRetValue(true));
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 串口设备(同步)
typedef ComHandlerAppender<FileDevice> ComDevice;
/// 串口设备(异步)
typedef ComHandlerAppender<AsyncFileDevice> AsyncComDevice;
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device 
} // namespace zhou_yb
//========================================================= 
