//========================================================= 
/**@file LC_ComToCCID_CmdAdapter.h
 * @brief 将LC COM口协议指令转为CCID协议指令格式 
 * 
 * @date 2015-09-13   10:53:41
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LC_COMTOCCID_CMDADAPTER_H_
#define _LIBZHOUYB_LC_COMTOCCID_CMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include "../../../device/iccard/base/ICCardLibrary.h"
using zhou_yb::device::iccard::base::ICCardLibrary;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace lc {
//--------------------------------------------------------- 
/// 将IInteractiveTrans转为ITransceiveTrans接口 
class LC_ComToCCID_CmdAdapter : 
    public ITransceiveTrans, 
    public DevAdapterBehavior<IInteractiveTrans>, 
    public RefObject
{
protected:
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
public:
    /// 按照格式接收数据 
    static bool RecvByFormat(IInteractiveTrans& dev, ByteBuilder& emptyBuffer)
    {
        const byte SHORT_CMD = 0xCC;
        const byte LONG_CMD = 0xBB;

        bool bRet = false;
        size_t rLen = 0;
        size_t cmdHeaderLen = 0;
        do
        {
            bRet = dev.Read(emptyBuffer);
            if(!bRet) return false;

            size_t len = emptyBuffer.GetLength();
            // 识别指令模式 
            if(len > 0 && cmdHeaderLen < 1)
            {
                switch(emptyBuffer[0])
                {
                    // 短指令 
                case SHORT_CMD:
                    cmdHeaderLen = 4;
                    break;
                    // 长指令 
                case LONG_CMD:
                    cmdHeaderLen = 5;
                    break;
                default:
                    cmdHeaderLen = 0;
                    break;
                }
            }
            // 预读指令头 
            if(len < cmdHeaderLen)
                continue;

            if(rLen < 1)
            {
                switch(emptyBuffer[0])
                {
                case SHORT_CMD:
                    rLen = static_cast<size_t>(emptyBuffer[cmdHeaderLen - 1]);
                    break;
                case LONG_CMD:
                    rLen = static_cast<size_t>(emptyBuffer[cmdHeaderLen - 1]);
                    rLen <<= BIT_OFFSET;
                    rLen += static_cast<size_t>(emptyBuffer[cmdHeaderLen - 2]);
                    break;
                default:
                    rLen += emptyBuffer.GetLength();
                    break;
                }
                rLen += cmdHeaderLen;
            }
            if(len >= rLen)
            {
                emptyBuffer.RemoveTail(len - rLen);
                return true;
            }

        } while(bRet);

        return bRet;
    }
    /// 按照格式打包数据 
    static void PackByFormat(const ByteArray& data, ByteBuilder& recv)
    {
        const byte SHORT_CMD = 0xCC;
        const byte LONG_CMD = 0xBB;

        size_t len = data.GetLength();
        if(len > 0x0FF)
        {
            recv += LONG_CMD;
            recv += static_cast<byte>(0x00);
            recv += static_cast<byte>(0x00);
            recv += _itobyte(len);
            recv += _itobyte(len >> BIT_OFFSET);
        }
        else
        {
            recv += SHORT_CMD;
            recv += static_cast<byte>(0x00);
            recv += static_cast<byte>(0x00);
            recv += _itobyte(len);
        }
        recv += data;
    }
public:
    /// 交换数据
    virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _tmpBuffer.Clear();
        DevCommand::FromAscii("31 A0", _tmpBuffer);
        _tmpBuffer += send;

        ASSERT_FuncErrRet(_pDev->Write(_tmpBuffer), DeviceError::SendErr);
        _tmpBuffer.Clear();
        ASSERT_FuncErrRet(RecvByFormat(_pDev, _tmpBuffer), DeviceError::RecvErr);

        LOGGER(_log << "Recv:" << endl;
        _log.WriteStream(_tmpBuffer) << endl);

        PackByFormat(_tmpBuffer, recv);
        // SW状态码 
        ICCardLibrary::SetSW(ICCardLibrary::SuccessSW, recv);

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace lc
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LC_COMTOCCID_CMDADAPTER_H_
//========================================================= 
