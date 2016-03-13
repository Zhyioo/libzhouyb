//========================================================= 
/**@file WinIDCardImagePrinter.h 
 * @brief Windows下身份证贴图处理
 * 
 * @date 2014-05-07   21:25:31 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../device/idcard/IDCardParser.h"
using zhou_yb::device::idcard::IDCardInformation;

#include <Windows.h>
#include <wingdi.h>

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// Windows下身份证贴图处理 
class WinIDCardImagePrinter
{
protected:
    //----------------------------------------------------- 
    WinIDCardImagePrinter() {}
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 保存BMP图片到文件中
    static BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName)
    { 
        HDC hDC; //设备描述表 
        int iBits; //当前显示分辨率下每个像素所占字节数 
        WORD wBitCount; //位图中每个像素所占字节数 
        DWORD dwPaletteSize=0, //定义调色板大小， 位图中像素字节大小 ，位图文件大小 ， 写入文件字节数 
            dwBmBitsSize, 
            dwDIBSize, dwWritten; 
        BITMAP Bitmap; //位图属性结构 
        BITMAPFILEHEADER bmfHdr; //位图文件头结构 
        BITMAPINFOHEADER bi; //位图信息头结构 
        LPBITMAPINFOHEADER lpbi; //指向位图信息头结构 

        HANDLE fh, hDib, hPal,hOldPal=NULL; //定义文件，分配内存句柄，调色板句柄 

        //计算位图文件每个像素所占字节数 
        CharConverter cvt;
        HDC hWndDC = CreateDC(cvt.to_char_t("DISPLAY"), NULL, NULL, NULL);
        hDC = ::CreateCompatibleDC( hWndDC ) ; 
        iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES); 
        DeleteDC(hDC);

        if (iBits <= 1) 
            wBitCount = 1; 
        else if (iBits <= 4)
            wBitCount = 4; 
        else if (iBits <= 8) 
            wBitCount = 8; 
        else if (iBits <= 24) 
            wBitCount = 24; 
        else 
            wBitCount = 24 ; 

        //计算调色板大小 
        if (wBitCount <= 8) 
            dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD); 

        //设置位图信息头结构 
        GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 
        bi.biSize = sizeof(BITMAPINFOHEADER); 
        bi.biWidth = Bitmap.bmWidth; 
        bi.biHeight = Bitmap.bmHeight; 
        bi.biPlanes = 1; 
        bi.biBitCount = wBitCount; 
        bi.biCompression = BI_RGB; 
        bi.biSizeImage = 0; 
        bi.biXPelsPerMeter = 0; 
        bi.biYPelsPerMeter = 0; 
        bi.biClrUsed = 0; 
        bi.biClrImportant = 0; 

        dwBmBitsSize = ((Bitmap.bmWidth * wBitCount+31)/32) * 4 * Bitmap.bmHeight ; 

        //为位图内容分配内存 
        hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER)); 
        lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
        *lpbi = bi; 

        // 处理调色板 
        hPal = GetStockObject(DEFAULT_PALETTE); 
        if (hPal) 
        { 
            hDC = ::GetDC(NULL); 
            hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE); 
            RealizePalette(hDC); 
        }

        // 获取该调色板下新的像素值 
        GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, 
            (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
            +dwPaletteSize, 
            (LPBITMAPINFO ) 
            lpbi, DIB_RGB_COLORS); 

        //恢复调色板 
        if (hOldPal) 
        { 
            SelectPalette(hDC, (HPALETTE)hOldPal, TRUE); 
            RealizePalette(hDC); 
            ::ReleaseDC(NULL, hDC); 
        } 

        //创建位图文件 
        fh = CreateFile(lpFileName, GENERIC_WRITE, 
            0, NULL, CREATE_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

        if (fh == INVALID_HANDLE_VALUE) 
            return FALSE; 

        // 设置位图文件头 
        bmfHdr.bfType = 0x4D42; // "BM" 
        dwDIBSize = sizeof(BITMAPFILEHEADER) 
            + sizeof(BITMAPINFOHEADER) 
            + dwPaletteSize + dwBmBitsSize; 
        bmfHdr.bfSize = dwDIBSize; 
        bmfHdr.bfReserved1 = 0; 
        bmfHdr.bfReserved2 = 0; 
        bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) 
            + (DWORD)sizeof(BITMAPINFOHEADER) 
            + dwPaletteSize; 

        // 写入位图文件头 
        WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 

        // 写入位图文件其余内容 
        WriteFile(fh, (LPSTR)lpbi, dwDIBSize, 
            &dwWritten, NULL); 

        //清除 
        GlobalUnlock(hDib); 
        GlobalFree(hDib); 
        CloseHandle(fh); 

        return TRUE; 
    }
    /**
     * @brief 将身份证背面信息打印到图片中
     * @param [in] idcInfo 身份证信息
     * @param [in] bmpfile 身份证上的背景图片
     * @param [in] outputBmpfile 输出的BMP文件名
     * @retval -1 传入的指针或路径为空
     * @retval -2 找不到bmpfile
     * @retval 0 成功
     */
    static int PrintBackInfoToBmp(const IDCardInformation& idcInfo, const char* bmpfile, const char* outputBmpfile)
    {
        if(NULL == bmpfile||
            NULL == outputBmpfile)
        {
            return -1;
        }

        // 身份证的文字信息(标准为华文细黑)
        const char txtFont[] = "黑体";
        // 身份证上的数字信息
        const char numFont[] = "楷体";

        HBITMAP hBmp;
        BITMAP bmp;
        HDC hDC;
        char tmp[80];

        HFONT hFont,hNumFont,hOldFont;
    
        CharConverter cvt;
        string_t cvtStr;
        hBmp = (HBITMAP)LoadImage(NULL, cvt.to_char_t(bmpfile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

        if(NULL == hBmp)
            return -2;

        hFont = CreateFont(36,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(txtFont));

        hNumFont = CreateFont(40,
            16,
            0,
            0,
            FW_NORMAL,
            0,
            0,
            0,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(numFont));

        hDC = CreateCompatibleDC(NULL);

        SetBkMode(hDC, TRANSPARENT);
    
        hOldFont = (HFONT)SelectObject(hDC, hFont);

        GetObject(hBmp, sizeof(bmp), &bmp);
        SelectObject(hDC, hBmp);

        cvtStr = cvt.to_char_t(idcInfo.Department.c_str());
        TextOut(hDC, 415, 460, cvtStr.c_str(), cvtStr.length());   //签发机关

        SelectObject(hDC, hNumFont);

        sprintf(tmp, "%c%c%c%c.%c%c.%c%c-%c%c%c%c.%c%c.%c%c",
            idcInfo.StartDate[0],idcInfo.StartDate[1],idcInfo.StartDate[2],idcInfo.StartDate[3],
            idcInfo.StartDate[4],idcInfo.StartDate[5],idcInfo.StartDate[6],idcInfo.StartDate[7],
            idcInfo.EndDate[0],idcInfo.EndDate[1],idcInfo.EndDate[2],idcInfo.EndDate[3],
            idcInfo.EndDate[4],idcInfo.EndDate[5],idcInfo.EndDate[6],idcInfo.EndDate[7]);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 415, 545, cvtStr.c_str(), cvtStr.length());   //有效期限
    
        SelectObject(hDC, hOldFont);
        DeleteObject(hFont);
        DeleteObject(hNumFont);
        DeleteDC(hDC);

        SaveBitmapToFile(hBmp, cvt.to_char_t(outputBmpfile));

        return 0;
    }
    /**
     * @brief 打印照片和身份证信息到图片中
     * @param [in] idcInfo 身份证信息
     * @param [in] bmpfile 身份证正面图片
     * @param [in] headfile 身份证头像信息
     * @param [in] outputBmpfile 输出的身份证图片信息
     * @retval -1 传入的指针或路径为空 
     * @retval -2 找不到bmpfile
     * @retval -3 找不到headfile
     * @retval 0 生成成功
     */
    static int PrintFrontInfoToBmp(const IDCardInformation& idcInfo, const char* bmpfile, const char* headfile, const char* outputBmpfile)
    {
        if(NULL == bmpfile ||
            NULL == headfile ||
            NULL == outputBmpfile)
        {
            return -1;
        }

        // 身份证的文字信息(标准为华文细黑)
        const char txtFont[] = "黑体";
        // 身份证上的数字信息
        const char numFont[] = "楷体";
        // 身份证号的字体
        const char idNumFont[] = "OCR-B 10 BT";

        HBITMAP hBmp;
        BITMAP bmp;
        HDC hDC;
        char tmp[128] = {0};
        int chcout = 0, index = 0;
        HFONT hFont, hNameFont, hIDNumFont, hNumFont, hOldFont;
        CharConverter cvt;
        string_t cvtStr;
        hBmp = (HBITMAP)LoadImage(NULL, cvt.to_char_t(bmpfile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        // 加载正面图片失败
        if(NULL == hBmp)
            return -2;

        hNameFont = CreateFont(45,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            PROOF_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(txtFont));

        hFont = CreateFont(38,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(txtFont));

        hIDNumFont = CreateFont(50,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(idNumFont));

        hNumFont = CreateFont(38,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(numFont));

        hDC = CreateCompatibleDC(NULL);

        SetBkMode(hDC, TRANSPARENT);

        hOldFont = (HFONT)SelectObject(hDC, hNameFont);

        GetObject(hBmp, sizeof(bmp), &bmp);
        SelectObject(hDC, hBmp);
        // 姓名
        cvtStr = cvt.to_char_t(idcInfo.Name.c_str());
        TextOut(hDC, 200, 92, cvtStr.c_str(), cvtStr.length());
        SelectObject(hDC, hFont);
        // 民族
        cvtStr = cvt.to_char_t(idcInfo.Nation.c_str());
        TextOut(hDC, 420, 172, cvtStr.c_str(), cvtStr.length());
        // 性别
        cvtStr = cvt.to_char_t(idcInfo.Gender.c_str());
        TextOut(hDC, 200, 172, cvtStr.c_str(), cvtStr.length());

        SelectObject(hDC, hNumFont);
        // 年
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Birthday.c_str(), 4);
        int num = atoi(tmp);
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%d", num);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 197, 250, cvtStr.c_str(), cvtStr.length());
        // 月 
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Birthday.c_str() + 4, 2);
        num = atoi(tmp);
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%d", num);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 350, 250, cvtStr.c_str(), cvtStr.length());
        // 日 
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Birthday.c_str() + 6, 2);
        num = atoi(tmp);
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%d", num);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 435, 250, cvtStr.c_str(), cvtStr.length());

        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Address.c_str(), 70);

        SelectObject(hDC, hFont);
        // 住址 
        chcout = strlen(tmp);
        while(chcout > 0)
        {
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, idcInfo.Address.c_str() + 22*index, 22);
            cvtStr = cvt.to_char_t(tmp);
            TextOut(hDC, 200, 335 + 50 * index, cvtStr.c_str(), cvtStr.length());
            chcout -= 22;
            index++;
        }

        SelectObject(hDC, hIDNumFont);
        // 身份证号码
        cvtStr = cvt.to_char_t(idcInfo.ID.c_str());
        TextOut(hDC, 340, 532, cvtStr.c_str(), cvtStr.length());

        SelectObject(hDC, hOldFont);
        DeleteObject(hFont);
        DeleteObject(hNumFont);
        DeleteObject(hIDNumFont);
        DeleteObject(hNameFont);

        // 贴照片
        HDC hHeadDC;
        HBITMAP hHeadBmp;
        BITMAP headBmp;
        hHeadDC = CreateCompatibleDC(NULL);
        // 载入位图
        hHeadBmp = (HBITMAP)LoadImage(NULL, cvt.to_char_t(headfile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        if(NULL == hHeadBmp)
            return -3;

        SetBkMode(hHeadDC, TRANSPARENT);
        GetObject(hHeadBmp, sizeof(headBmp), &headBmp);
        SelectObject(hHeadDC, hHeadBmp);
        //BitBlt(hDC,220,20,headBmp.bmWidth,headBmp.bmHeight,hHeadDC,0,0,SRCCOPY);

        // 删除照片背景
        COLORREF color = GetPixel(hHeadDC, 1, 1);
        TransparentBlt(hDC, 635, 72, 3*(headBmp.bmWidth), 3*(headBmp.bmHeight),
            hHeadDC, 0, 0, headBmp.bmWidth, headBmp.bmHeight, color);

        DeleteDC(hHeadDC);
        DeleteDC(hDC);

        SaveBitmapToFile(hBmp, cvt.to_char_t(outputBmpfile));

        return 0;
    }
    /**
     * @brief 打印照片和身份证信息到图片中(1:1的比例)
     * @param [in] idcInfo 身份证信息
     * @param [in] bmpfile 身份证正面图片
     * @param [in] headfile 身份证头像信息
     * @param [in] outputBmpfile 输出的身份证图片信息
     * @retval -1 传入的指针或路径为空 
     * @retval -2 找不到bmpfile
     * @retval -3 找不到headfile
     * @retval 0 生成成功
     */
    static int SmallPrintFrontInfoToBmp(const IDCardInformation& idcInfo, const char* bmpfile, const char* headfile, const char* outputBmpfile)
    {
        if(NULL == bmpfile ||
            NULL == headfile ||
            NULL == outputBmpfile)
        {
            return -1;
        }

        // 身份证的文字信息(标准为华文细黑)
        const char txtFont[] = "黑体";
        // 身份证上的数字信息
        const char numFont[] = "楷体";
        // 身份证号的字体
        const char idNumFont[] = "OCR-B 10 BT";

        HBITMAP hBmp;
        BITMAP bmp;
        HDC hDC;
        char tmp[128] = {0};
        int chcout = 0, index = 0;
        HFONT hFont, hNameFont, hIDNumFont, hNumFont, hOldFont;
        CharConverter cvt;
        string_t cvtStr;
        hBmp = (HBITMAP)LoadImage(NULL, cvt.to_char_t(bmpfile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        // 加载正面图片失败
        if(NULL == hBmp)
            return -2;

        hNameFont = CreateFont(15,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            PROOF_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(txtFont));

        hFont = CreateFont(12,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(txtFont));

        hIDNumFont = CreateFont(16,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(idNumFont));

        hNumFont = CreateFont(13,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(numFont));

        hDC = CreateCompatibleDC(NULL);

        SetBkMode(hDC, TRANSPARENT);

        hOldFont = (HFONT)SelectObject(hDC, hNameFont);

        GetObject(hBmp, sizeof(bmp), &bmp);
        SelectObject(hDC, hBmp);

        cvtStr = cvt.to_char_t(idcInfo.Name.c_str());
        TextOut(hDC, 65, 32, cvtStr.c_str(), cvtStr.length()); //姓名

        SelectObject(hDC, hFont);
        
        cvtStr = cvt.to_char_t(idcInfo.Nation.c_str());
        TextOut(hDC, 136, 60, cvtStr.c_str(), cvtStr.length());    //民族
    
        cvtStr = cvt.to_char_t(idcInfo.Gender.c_str());
        TextOut(hDC, 65, 60, cvtStr.c_str(), cvtStr.length()); //性别

        SelectObject(hDC, hNumFont);

        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Birthday.c_str(), 4);
        int num = atoi(tmp);
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%d", num);

        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 65, 86, cvtStr.c_str(), cvtStr.length()); //年

        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Birthday.c_str() + 4, 2);
        num = atoi(tmp);
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%d", num);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 125, 86, cvtStr.c_str(), cvtStr.length());    //月

        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Birthday.c_str() + 6, 2);
        num = atoi(tmp);
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%d", num);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 150, 86, cvtStr.c_str(), cvtStr.length());//日

        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, idcInfo.Address.c_str(), 70);

        SelectObject(hDC, hFont);

        chcout = strlen(tmp);
        while(chcout > 0)
        {
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, idcInfo.Address.c_str() + 22*index, 22);
            cvtStr = cvt.to_char_t(tmp);
            TextOut(hDC, 65, 114 + 15 * index, cvtStr.c_str(), cvtStr.length()); //住址
            chcout -= 22;
            index++;
        }

        SelectObject(hDC, hIDNumFont);
        // 身份证号码
        cvtStr = cvt.to_char_t(idcInfo.ID.c_str());
        TextOut(hDC, 115, 178, cvtStr.c_str(), cvtStr.length());

        SelectObject(hDC, hOldFont);
        DeleteObject(hFont);
        DeleteObject(hNumFont);
        DeleteObject(hIDNumFont);
        DeleteObject(hNameFont);

        // 贴照片
        HDC hHeadDC;
        HBITMAP hHeadBmp;
        BITMAP headBmp;
        hHeadDC = CreateCompatibleDC(NULL);
        // 载入位图
        hHeadBmp = (HBITMAP)LoadImage(NULL, cvt.to_char_t(headfile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        if(NULL == hHeadBmp)
            return -3;

        SetBkMode(hHeadDC, TRANSPARENT);
        GetObject(hHeadBmp, sizeof(headBmp), &headBmp);
        SelectObject(hHeadDC, hHeadBmp);
        //BitBlt(hDC,220,20,headBmp.bmWidth,headBmp.bmHeight,hHeadDC,0,0,SRCCOPY);

        // 删除照片背景
        COLORREF color = GetPixel(hHeadDC, 1, 1);
        TransparentBlt(hDC, 220, 20, (headBmp.bmWidth), (headBmp.bmHeight),
            hHeadDC, 0, 0, headBmp.bmWidth, headBmp.bmHeight, color);

        DeleteDC(hHeadDC);
        DeleteDC(hDC);

        SaveBitmapToFile(hBmp, cvt.to_char_t(outputBmpfile));

        return 0;
    }
    //---------------------------------------------------------
    /**
     * @brief 将身份证背面信息打印到图片中(1:1的比例)
     * @param [in] idcInfo 身份证信息
     * @param [in] bmpfile 身份证上的背景图片
     * @param [in] outputBmpfile 输出的BMP文件名
     * @retval -1 传入的指针或路径为空
     * @retval -2 找不到bmpfile
     * @retval 0 成功
     */
    static int SmallPrintBackInfoToBmp(const IDCardInformation& idcInfo, const char* bmpfile, const char* outputBmpfile)
    {
        if(NULL == bmpfile || NULL == outputBmpfile)
            return -1;
     
        // 身份证的文字信息(标准为华文细黑)
        const char txtFont[] = "黑体";
        // 身份证上的数字信息
        const char numFont[] = "楷体";
    
        HBITMAP hBmp;
        BITMAP bmp;
        HDC hDC;
        char tmp[80];
    
        HFONT hFont,hNumFont,hOldFont;
        CharConverter cvt;
        string_t cvtStr;
        hBmp = (HBITMAP)LoadImage(NULL, cvt.to_char_t(bmpfile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    
        if(NULL == hBmp)
            return -2;
     
        hFont = CreateFont(13,
            0,
            0,
            0,
            FW_BLACK,
            0,
            0,
            0,
            GB2312_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(txtFont));
    
        hNumFont = CreateFont(15,
            6,
            0,
            0,
            FW_NORMAL,
            0,
            0,
            0,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            cvt.to_char_t(numFont));
     
        hDC = CreateCompatibleDC(NULL);
     
        SetBkMode(hDC, TRANSPARENT);
     
        hOldFont = (HFONT)SelectObject(hDC, hFont);
     
        GetObject(hBmp, sizeof(bmp), &bmp);
        SelectObject(hDC, hBmp);
    
        cvtStr = cvt.to_char_t(idcInfo.Department.c_str());
        TextOut(hDC, 135, 155, cvtStr.c_str(), cvtStr.length());   //签发机关
    
        SelectObject(hDC, hNumFont);
    
        sprintf(tmp, "%c%c%c%c.%c%c.%c%c-%c%c%c%c.%c%c.%c%c",
            idcInfo.StartDate[0],idcInfo.StartDate[1],idcInfo.StartDate[2],idcInfo.StartDate[3],
            idcInfo.StartDate[4],idcInfo.StartDate[5],idcInfo.StartDate[6],idcInfo.StartDate[7],
            idcInfo.EndDate[0],idcInfo.EndDate[1],idcInfo.EndDate[2],idcInfo.EndDate[3],
            idcInfo.EndDate[4],idcInfo.EndDate[5],idcInfo.EndDate[6],idcInfo.EndDate[7]);
        cvtStr = cvt.to_char_t(tmp);
        TextOut(hDC, 135, 182, cvtStr.c_str(), cvtStr.length());   //有效期限
     
        SelectObject(hDC, hOldFont);
        DeleteObject(hFont);
        DeleteObject(hNumFont);
        DeleteDC(hDC);
    
        SaveBitmapToFile(hBmp, cvt.to_char_t(outputBmpfile));
    
        return 0;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension
} // namespace zhou_yb
//========================================================= 