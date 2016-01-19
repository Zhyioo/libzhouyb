//========================================================= 
/**@file WinTestHelper.h 
 * @brief Windows下测试辅助接口函数  
 * 
 * @date 2015-01-31   11:41:13 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TextPrinter.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
struct WinTestHelper
{
    static COLORREF TextColor(int mode)
    {
        // 默认为黑色 
        COLORREF color = RGB(0x00, 0x00, 0x00);
        switch(mode)
        {
        /* 对客户不重要的信息 */
        case TextPrinter::TextLogger:
            // 灰色 
            color = RGB(0x80, 0x80, 0x80);
            break;
        case TextPrinter::TextNote:
            // 灰白 
            color = RGB(0x59, 0x59, 0x59);
            break;
        case TextPrinter::TextMessage:
            // 淡蓝色 
            color = RGB(0x7E, 0xC2, 0xFF);
            break;
        case TextPrinter::TextWarning:
            // 橙色 
            color = RGB(0xD7, 0xAB, 0x69);
            break;
        case TextPrinter::TextHighWarning:
            // 暗红 
            color = RGB(0xBD, 0x63, 0xC5);
            break;
        /* 需要回显给客户的信息 */
        case TextPrinter::TextTips:
            // 粉红色 
            color = RGB(0xFF, 0x80, 0xC0);
            break;
        case TextPrinter::TextInfo:
            // 淡蓝色 
            color = RGB(0x00, 0x82, 0xFF);
            break;
        case TextPrinter::TextResult:
            // 深蓝 
            color = RGB(0, 0, 0xFF);
            break;
        case TextPrinter::TextError:
            // 红色 
            color = RGB(0xFF, 0x00, 0x00);
            break;
        default:
            break;
        }
        
        return color;
    }
    
    
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension 
} // namespace zhou_yb
//========================================================= 