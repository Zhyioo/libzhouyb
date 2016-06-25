//========================================================= 
/**@file FingerDevAdapter.h
 * @brief 维尔指纹仪驱动
 * 
 * @date 2016-05-11   12:25:31
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_FINGERDEVADAPTER_H_
#define _LIBZHOUYB_FINGERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../device/cmd_adapter/ComICCardCmdAdapter.h"
using zhou_yb::device::cmd_adapter::ComICCardCmdAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace finger {
//--------------------------------------------------------- 
class WE_FingerDevAdapter : 
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    /// 接收数据
    bool _TransCommand(const ByteArray& cmd, ByteBuilder& recv)
    {
        ASSERT_FuncErr(_pDev->Write(cmd), DeviceError::SendErr);
        ByteBuilder tmp(32);
        ASSERT_FuncErr(ComICCardCmdAdapter::RecvByFormat(_pDev, tmp), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("RecvByFormat:").WriteLine(tmp));
        ByteBuilder data(32);
        ByteConvert::Fold(tmp.SubArray(5, tmp.GetLength() - 8), data);
        recv += data;
        return true;
    }
public:
    /// 将指纹数据保存成图片
    static bool ToBmp(const ByteArray& image, const char* filePath)
    {
        const size_t COLOR_TABLE_LENGTH = 256;
        const size_t nImgMaxSize = 152 * 200;

        ofstream fout;
        fout.open(filePath, ios::out | ios::binary);
        if(fout.fail())
            return false;

        ByteBuilder bmpBuff(1024);
        // BITMAPFILEHEADER
        DevCommand::FromAscii("424DF67600000000000036040000", bmpBuff);
        // BITMAPINFOHEADER
        DevCommand::FromAscii("2800000098000000C800000001000800000000000000000000000000000000000000000000000000", bmpBuff);
        // COLOR_TABLE
        for(int i = 0; i < COLOR_TABLE_LENGTH; i++)
        {
            bmpBuff += _itobyte(i);
            bmpBuff += _itobyte(i);
            bmpBuff += _itobyte(i);
            bmpBuff += static_cast<byte>(0x00);
        }
        fout.write(bmpBuff.GetString(), bmpBuff.GetLength());
        fout.write(image.GetString() + 3, nImgMaxSize);
        fout.close();

        return true;
    }
    /// 取版本信息
    bool GetVersion(ByteBuilder& version)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        return _logRetValue(true);
    }
    /// 取特征
    bool GetFeature(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder cmd(8);
        DevCommand::FromAscii("02 30 30 30 34 30 3C 30 30 30 30 30 30 30 38 03", cmd);
        ASSERT_FuncErrRet(_TransCommand(cmd, data), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 取模板
    bool GetTemplate(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder cmd(8);
        DevCommand::FromAscii("02 30 30 30 34 30 3B 30 30 30 30 30 30 30 3F 03", cmd);
        ASSERT_FuncErrRet(_TransCommand(cmd, data), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 取指纹图片
    bool GetImage(ByteBuilder& image)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ByteBuilder sCmd(8);
        ByteBuilder rCmd(64);
        DevCommand::FromAscii("1A 50 31", sCmd);

        ASSERT_FuncErrRet(_pDev->Write(sCmd), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(rCmd), DeviceError::RecvErr);
        
        Timer::Wait(50);

        sCmd.Clear();
        rCmd.Clear();
        DevCommand::FromAscii("1A 50 32", sCmd);
        size_t nImgMaxSize = 152 * 200;

        ASSERT_FuncErrRet(_pDev->Write(sCmd), DeviceError::SendErr);

        bool bReadImage = true;
        size_t lastLen = image.GetLength();
        while((image.GetLength() - lastLen) < (nImgMaxSize + 3))
        {
            if(!_pDev->Read(image))
            {
                bReadImage = false;
                break;
            }
        }
        LOGGER(_log << "FingerRecvLength:" << (image.GetLength() - lastLen) << endl);
        ASSERT_FuncErrRet(bReadImage, DeviceError::RecvFormatErr);

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace finger
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_FINGERDEVADAPTER_H_
//========================================================= 