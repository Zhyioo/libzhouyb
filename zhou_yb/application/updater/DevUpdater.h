//========================================================= 
/**@file DevUpdater.h 
 * @brief 设备固件升级模块 
 * 
 * - 固件数据格式:
 *  - 原始固件以hex文件存在
 *  - 驱动使用经过加密处理的dev文件格式(格式说明参照DevUpdaterConvert.h) 
 *  - hex文件转换后为一系列的数据行 
 *  - 升级时,以数据行为单位进行升级 
 * .
 * 
 * @date 2015-01-02   11:32:05 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "DevUpdaterConvert.h"

#include "../test_frame/TestFrame.h"
using namespace zhou_yb::application::test;

#include "../../extension/ability/logger/FolderLogger.h"
using zhou_yb::extension::ability::FolderHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 固件数据解析器 
struct IUpdateDecoder
{
    /// 将升级文件转换为数据行 
    virtual bool Parse(const char* sPath, list<ByteBuilder>& updateList) = 0;
};
//--------------------------------------------------------- 
/// 默认的固件文件数据解析器 
class UpdateDecoder : public IUpdateDecoder, public LastErrBehavior
{
protected:
    //----------------------------------------------------- 
    /// 解密dev文件时的Key
    ByteBuilder _Key;
    //----------------------------------------------------- 
    /**
     * @brief 转换升级数据
     *
     * @warning 转换时,上层循环调用ParseLine,将多个数据转换到updateBin中
     *          返回值仅仅代表是否可以进行下次转换,并不标志转换失败而退出
     *
     * @param [in] bin 升级的数据(此处的数据一定是正常格式的)
     * @param [out] updateBin 转换后的数据
     * @retval true 已经转换完成,可以进行下一次转换
     * @retval false 当前转换未完成,还需要继续转换
     */
    virtual bool ParseLine(const ByteArray& bin, ByteBuilder& updateBin)
    {
        // 不做转换 
        updateBin = bin;
        return true;
    }
    /// 自动枚举当前目录下的第一个Hex或者Dev文件 
    bool _AutoScanUpdateFile(const char* dir, string& path)
    {
        list<string> files;
        if(FolderHelper::EnumFiles(dir, files, NULL, ".dev") < 1)
        {
            if(FolderHelper::EnumFiles(dir, files, NULL, ".hex") < 1)
                return false;
        }

        path = files.front();
        return true;
    }
    /// 解析HEX文件 
    bool _ParseHEX(const char* sPath, list<ByteBuilder>& updateList)
    {
        ifstream fin;

        if(!DevUpdaterConvert::OpenHEX(fin, sPath))
        {
            _lasterr = DeviceError::DevOpenErr;
            _errinfo = "打开HEX文件失败";
            return false;
        }

        /* 获取升级数据 */

        // 读取出的hex文件行 
        ByteBuilder hexline(64);
        // 解析后的bin行 
        ByteBuilder binline(64);
        // 转换后的数据行 
        ByteBuilder parseline(64);

        while(DevUpdaterConvert::ReadHEX(fin, hexline))
        {
            if(!DevUpdaterConvert::HEXtoBIN(hexline, binline))
            {
                _lasterr = DeviceError::ArgFormatErr;
                _errinfo = "解析HEX数据失败";
                return false;
            }
            hexline.Clear();

            if(!DevUpdaterConvert::IsValidBIN(binline))
            {
                _lasterr = DeviceError::ArgFormatErr;
                _errinfo = "HEX数据校验值错误";
                return false;
            }

            if(ParseLine(binline, parseline))
            {
                updateList.push_back(parseline);
                parseline.Clear();

                ParseLine(binline, parseline);
            }
            binline.Clear();
        }
        if(!parseline.IsEmpty())
        {
            updateList.push_back(parseline);
        }
        fin.close();

        return true;
    }
    /// 解析DEV文件 
    bool _ParseDEV(const char* sPath, const ByteArray& key,  list<ByteBuilder>& updateList)
    {
        ByteBuilder info(16);
        ifstream fin;

        if(!DevUpdaterConvert::OpenDEV(fin, sPath, &info))
        {
            _lasterr = DeviceError::DevOpenErr;
            _errinfo = "打开DEV文件失败";
            return false;
        }
        /* 获取升级数据 */

        // 每个dev文件中的hex行(可能包含多行) 
        list<ByteBuilder> subline;
        // 读取出的dev文件行 
        ByteBuilder devline(64);
        // 解析后的bin行 
        ByteBuilder binline(64);
        // 转换后的数据行 
        ByteBuilder parseline(64);

        while(DevUpdaterConvert::ReadDEV(fin, devline))
        {
            subline.clear();
            if(!DevUpdaterConvert::ParseDEV(devline, subline))
            {
                _lasterr = DeviceError::ArgFormatErr;
                _errinfo = "解析DEV数据失败";
                return false;
            }
            devline.Clear();
            for(list<ByteBuilder>::iterator itr = subline.begin(); itr != subline.end(); ++itr)
            {
                if(!DevUpdaterConvert::DEVtoBIN(*itr, binline, key, info))
                {
                    _lasterr = DeviceError::ArgRangeErr;
                    _errinfo = "转换HEX数据失败";
                    return false;
                }
                if(!DevUpdaterConvert::IsValidBIN(binline))
                {
                    _lasterr = DeviceError::ArgFormatErr;
                    _errinfo = "HEX数据校验值错误";
                    return false;
                }

                if(ParseLine(binline, parseline))
                {
                    updateList.push_back(parseline);
                    parseline.Clear();
                    
                    ParseLine(binline, parseline);
                }
                binline.Clear();
            }
        }
        if(!parseline.IsEmpty())
        {
            updateList.push_back(parseline);
        }
        fin.close();

        return true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    UpdateDecoder(const ByteArray& Key = "") { _Key = Key; }
    //----------------------------------------------------- 
    /// 解析文件数据
    virtual bool Parse(const char* sPath, list<ByteBuilder>& updateList)
    {
        ByteArray pathArray(sPath);
        if(pathArray.IsEmpty())
            pathArray = ".\\";
        string path = "";
        if(pathArray[pathArray.GetLength() - 1] == PATH_SEPARATOR)
        {
            if(!_AutoScanUpdateFile(pathArray.GetString(), path))
            {
                _lasterr = DeviceError::ArgFormatErr;
                _errinfo = "没有识别到升级文件";
                return false;
            }
            pathArray = ByteArray(path.c_str(), path.length());
        }
        bool bRet = false;
        if(StringConvert::EndWith(pathArray, ".dev"))
        {
            bRet = _ParseDEV(pathArray.GetString(), _Key, updateList);
        }
        else if(StringConvert::EndWith(pathArray, ".hex"))
        {
            bRet = _ParseHEX(pathArray.GetString(), updateList);
        }
        else
        {
            _lasterr = DeviceError::DevNotValidErr;
            _errinfo = "不支持的文件类型";
            return false;
        }
        return bRet;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/**
 * @brief 设备固件升级功能的默认实现 
 *
 * @param TDevice 升级的设备类型  
 * @param TInterface 升级的接口类型 
 * @param TLink 设备连接器
 * @param TContainer 升级的接口类型转换器 
 * @param TDecoder 升级文件数据解码器 
 * 
 * - 包括以下功能:
 *  - 自动加载dev或hex文件数据 
 *  - 自动处理dev或hex文件中每个数据行的升级,并通过回调函数返回升级进度 
 * .
 *  
 * - 子类需要实现的功能:
 *  - 实现Link(const char* sArg)接口连接设备,和UnLink()断开设备  
 *  - 如果升级文件不是dev文件需要重新实现PreUpdate(const char* sPath)
 *  - 实现UpdateLine(const ByteArray& bin)处理每个升级数据
 * .
 * 
 */ 
template<class TDevice, class TInterface, class TLinker, class TContainer = TestContainer<TDevice, TInterface>, class TDecoder = UpdateDecoder>
class DevUpdater : public TestAdapter<TDevice, TInterface, TLinker, TContainer>
{
public:
    //----------------------------------------------------- 
    /**
     * @brief 开始升级 
     * 
     * @param [in] testCase 升级每一行数据的升级器 
     * @param [in] testArg 升级文件路径 
     */
    virtual bool OnTest(ITestCase<TDevice>& testCase, const ByteArray& testArg)
    {
        ASSERT_Func(IsValid() && !_testInterface.IsNull());
        // 所有需要升级的数据 
        list<ByteBuilder> updateList;
        
        TextPrint(TextPrinter::TextTips, "升级中,请稍后...");
        // 准备升级数据 
        UpdateDecoder decoder;
        if(!decoder.Parse(testArg.GetString(), updateList))
        {
            TextPrint(TextPrinter::TextError, "打开升级文件失败");
            return false;
        }
        // 升级的总进度 
        size_t updateCount = updateList.size();
        // 当前已经升级完成的进度 
        size_t updateCurrent = 0;
        
        int progress = 0;
        int lastProgress = 0;

        // 进度单位 千分比  
        const uint MAX_PROCESS = 1000;

        // 升级进度显示 
        Timer timer;
        char str[64] = { 0 };
        uint ms = 0;

        list<ByteBuilder>::iterator itr = updateList.begin();
        for(itr = updateList.begin(); itr != updateList.end(); ++itr)
        {
            if(!testCase.Test(_testInterface, *itr, *this))
                break;

            ++updateCurrent;

            // 进度 >0.1% 的时候通知UI更新 
            progress = static_cast<int>(static_cast<double>(updateCurrent)* MAX_PROCESS / static_cast<double>(updateCount));
            if((progress - lastProgress) > 1)
            {
                ms = static_cast<uint>(timer.Elapsed()/100);
                sprintf(str, "升级中...%3d.%1d%% [%d.%ds]", progress / 10, progress % 10, ms / 10, ms % 10);
                TextPrint(TextPrinter::TextTips, str);
                lastProgress = progress;
            }
        }

        // 升级失败 
        if(itr != updateList.end())
            return false;
        
        // 显示更新进度为 100.0%
        TextPrint(TextPrinter::TextTips, "升级中...100.0%%");
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
//========================================================= 