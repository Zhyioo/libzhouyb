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

        typedef struct tagRGBQUAD {
            unsigned char    rgbBlue;
            unsigned char    rgbGreen;
            unsigned char    rgbRed;
            unsigned char    rgbReserved;
        } _RGBQUAD;
        typedef struct tagBITMAPFILEHEADER {
            unsigned short    bfType;
            unsigned long     bfSize;
            unsigned short    bfReserved1;
            unsigned short    bfReserved2;
            unsigned long     bfOffBits;
        } _BITMAPFILEHEADER;
        typedef struct tagBITMAPINFOHEADER{
            unsigned long      biSize;
            unsigned long      biWidth;
            unsigned long      biHeight;
            unsigned short     biPlanes;
            unsigned short     biBitCount;
            unsigned long      biCompression;
            unsigned long      biSizeImage;
            unsigned long      biXPelsPerMeter;
            unsigned long      biYPelsPerMeter;
            unsigned long      biClrUsed;
            unsigned long      biClrImportant;
        } _BITMAPINFOHEADER;

        ofstream fout;
        fout.open(filePath, ios::out | ios::binary);
        if(fout.fail())
            return false;

        ByteBuilder bmpBuff(1024);

        _BITMAPFILEHEADER bfh;
        memset(&bfh, 0, sizeof(bfh));
        bfh.bfType = 'MB';
        bfh.bfSize = sizeof(bfh) + nImgMaxSize + sizeof(_BITMAPINFOHEADER);
        bfh.bfOffBits = sizeof(_BITMAPINFOHEADER) + sizeof(_BITMAPFILEHEADER) + sizeof(_RGBQUAD) * COLOR_TABLE_LENGTH;

        bmpBuff += ByteArray((const byte*)&bfh, sizeof(bfh));
        
        _BITMAPINFOHEADER bih;
        memset(&bih, 0, sizeof(bih));
        bih.biSize = sizeof(bih);
        bih.biWidth = 152;
        bih.biHeight = 200;
        bih.biPlanes = 1;
        bih.biBitCount = 8;

        bmpBuff += ByteArray((const byte*)&bih, sizeof(bih));

        _RGBQUAD colorTable;
        for(int i = 0; i < COLOR_TABLE_LENGTH; i++)
        {
            colorTable.rgbBlue = i;
            colorTable.rgbGreen = i;
            colorTable.rgbRed = i;
            colorTable.rgbReserved = 0;

            bmpBuff += ByteArray((const byte*)&colorTable, sizeof(colorTable));
        }
        fout.write((char*)bmpBuff.GetBuffer(), bmpBuff.GetLength());
        fout.write((char*)(image.GetBuffer() + 3), nImgMaxSize);
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
        
        sCmd.Clear();
        rCmd.Clear();
        DevCommand::FromAscii("1A 50 32", sCmd);
        size_t nImgMaxSize = 152 * 200;

        ASSERT_FuncErrRet(_pDev->Write(sCmd), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(rCmd), DeviceError::RecvErr);

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
        //ASSERT_FuncErrRet(bReadImage, DeviceError::RecvFormatErr);

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