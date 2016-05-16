//========================================================= 
/**@file LC_ReaderDevAdapter.h
 * @brief LC IC卡读卡器操作 
 * 
 * @date 2015-05-14   22:30:50
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_READERDEVADAPTER_H_
#define _LIBZHOUYB_LC_READERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include "../../../device/iccard/base/ICCardLibrary.h"
using zhou_yb::device::iccard::base::ICCardLibrary;

#include "LC_Provider.h"
using zhou_yb::application::lc::LC_Provider;

#include "../../../device/iccard/base/ICCardAppAdapter.h"
using zhou_yb::device::iccard::base::ICCardAppAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace lc {
//--------------------------------------------------------- 
/// LC IC读卡器操作 
class LC_ReaderDevAdapter : public ICCardAppAdapter
{
public:
    /// 电源管理模块
    enum PowerModule
    {
        /// 二代证
        IDCard_Module = 0x00,
        /// 密码键盘 
        PinPad_Module = 0x01,
        /// 打印机 
        Printer_Module = 0x02,
        /// 指纹仪 
        Finger_Module = 0x03
    };
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuffer;
    /// 接收缓冲区  
    ByteBuilder _recvBuffer;
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
    /// 交换数据 
    bool _trans_cmd(const ByteArray& cmd, byte* pStatus, byte* pParam, ByteBuilder* pData)
    {
        const byte SHORT_CMD = 0xCC;
        const byte LONG_CMD = 0xBB;
        const byte SW_OK = 0x00;

        _tmpBuffer.Clear();
        if(!_apdu(cmd, _tmpBuffer))
            return false;

        size_t cmdLen = 0;
        size_t len = 0;
        switch(_tmpBuffer[0])
        {
        // 短响应 
        case SHORT_CMD:
            cmdLen = 4;
            if(_tmpBuffer.GetLength() < cmdLen)
            {
                _logErr(DeviceError::RecvFormatErr, "CC短响应");
                return false;
            }
            len = static_cast<size_t>(_tmpBuffer[cmdLen - 1]);
            break;
        // 长响应 
        case LONG_CMD:
            cmdLen = 5;
            if(_tmpBuffer.GetLength() < cmdLen)
            {
                _logErr(DeviceError::RecvFormatErr, "BB长响应");
                return false;
            }
            len = static_cast<size_t>(_tmpBuffer[cmdLen - 1]);
            len <<= BIT_OFFSET;
            len += static_cast<size_t>(_tmpBuffer[cmdLen - 2]);
            break;
        default:
            _logErr(DeviceError::RecvFormatErr, "无法识别的响应格式");
            return false;
        }

        if((_tmpBuffer.GetLength() - cmdLen) != len)
        {
            _logErr(DeviceError::ArgLengthErr, "接收数据的长度错误");
            return false;
        }

        LOGGER(_log.WriteLine("Recv:").WriteLine(_tmpBuffer));

        byte status = _tmpBuffer[1];
        if(pStatus != NULL)
            (*pStatus) = status;

        if(status != SW_OK)
        {
            LOGGER(_log << "SW:<" << _hex(status) << ">\n");
            _logErr(DeviceError::OperatorStatusErr, "状态码错误");
            return false;
        }
        ByteArray data = _tmpBuffer.SubArray(cmdLen, len);
        LOGGER(_log << "Param:<" << _hex(_tmpBuffer[2]));
        LOGGER(_log << "Data:" << data << endl);
        if(pParam != NULL)
            (*pParam) = _tmpBuffer[2];
        if(pData != NULL)
            pData->Append(data);

        return true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 根据直线的x求y
    static int LineY(int x1, int y1, int x2, int y2, int x)
    {
        // 直线公式
        if(x1 == x2)
            return x1;
        double k = double(y2 - y1) / double(x2 - x1);
        double b = double(x1*y2 - x2*y1) / double(x1 - x2);

        return static_cast<int>(k * x + b);
    }
    /// 将电压转为电量(0.001V -> 100.0%)
    static uint VoltageToElectricity(uint voltage)
    {
        // OCV曲线表
        uint OCV[] = {
            1000, SIZE_EOF,
            1000, 4177,
            850,  4040,
            550,  3832,
            450,  3799,
            250,  3755,
            150,  3693,
            100,  3687,
            50,   3593,
            0,    3213,
            0,    0
        };
        size_t OCV_SIZE = SizeOfArray(OCV) / 2;
        for(size_t i = 1;i < OCV_SIZE; ++i)
        {
            if(voltage > OCV[2*i + 1])
                return LineY(OCV[2*(i - 1) + 1], OCV[2*(i - 1)], OCV[2*i + 1], OCV[2*i], voltage);
        }

        return 0;
    }
    /// 设置设备为升级模式 
    bool SetUpdateMode()
    {
        LOG_FUNC_NAME();
        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 22 00 00 00", _sendBuffer);
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, NULL), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 认证设备是否是合法的设备 
    bool Verify()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder random(16);
        ByteConvert::Random(random, 16);

        // FF 21 00 00 10 XX
        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 21 00 00 10", _sendBuffer);
        _sendBuffer += random;

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, &_recvBuffer), DeviceError::TransceiveErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() == random.GetLength(), DeviceError::RecvFormatErr);

        return _logRetValue(LC_Provider::Verify(random, _recvBuffer));
    }
    /**
     * @brief 获取电源状态
     * 
     * @param [in] module 需要操作的模块 
     * @param [out] timeoutM 电源剩余的时间,0表示已断电,-1表示长开  
     * 
     * @return bool 
     */
    bool PowerStatus(PowerModule module, byte& timeoutM)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "Module:<" << _hex(static_cast<byte>(module)) << ">\n");

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 13 00 00 00", _sendBuffer);
        _sendBuffer[4] = static_cast<byte>(module);

        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, &timeoutM, NULL), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /**
     * @brief 电源控制
     * 
     * @param [in] module 需要设置的模块 
     * @param [in] timeoutM [default:0] 电源开启的时间(秒),0表示关闭,-1表示长开 
     * 
     * @return bool 是否操作成功 
     */
    bool PowerControl(PowerModule module, byte timeoutM = 0)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 14 00 00 00", _sendBuffer);
        _sendBuffer[4] = static_cast<byte>(module);
        _sendBuffer[5] = timeoutM;

        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, NULL), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 获取设备版本号
    bool GetVersion(ByteBuilder& version)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 20 00 00 00", _sendBuffer);

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, &_recvBuffer), DeviceError::TransceiveErr);

        LOGGER(_log.WriteLine("Version:").WriteStream(_recvBuffer) << endl);
        version += _recvBuffer;

        return _logRetValue(true);
    }
    /// 获取设备序列号 
    bool GetSerialNumber(ByteBuilder& serialNumber)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 23 00 00 00", _sendBuffer);

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, &_recvBuffer), DeviceError::TransceiveErr);

        LOGGER(_log.WriteLine("序列号:").WriteStream(_recvBuffer) << endl);
        serialNumber += _recvBuffer;

        return _logRetValue(true);
    }
    /// 设置设备序列号 
    bool SetSerialNumber(const ByteArray& serialNumber)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ASSERT_FuncErrRet(serialNumber.GetLength() < 256, DeviceError::ArgLengthErr);

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 24 00 00", _sendBuffer);
        _sendBuffer += _itobyte(serialNumber.GetLength());
        _sendBuffer += serialNumber;

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, NULL), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 获取电池电压(0.01V) 
    bool GetVoltage(uint& val)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 15 00 00 00", _sendBuffer);

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, &_recvBuffer), DeviceError::TransceiveErr);
        ASSERT_FuncErrRet(_recvBuffer.GetLength() >= 2, DeviceError::RecvFormatErr);

        val = _recvBuffer[0];
        val <<= BIT_OFFSET;
        val += _recvBuffer[1];

        LOGGER(_log << "电压:<" << val << ">\n");

        return _logRetValue(true);
    }
    /// 设置ACK校验模式
    bool SetAckMode(bool isACK)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuffer.Clear();
        DevCommand::FromAscii("FF 16 00 00 00", _sendBuffer);
        _sendBuffer[ICCardLibrary::P1_INDEX] = isACK ? 0x01 : 0x00;

        _recvBuffer.Clear();
        ASSERT_FuncErrRet(_trans_cmd(_sendBuffer, NULL, NULL, NULL), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace lc
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_READERDEVADAPTER_H_
//========================================================= 
