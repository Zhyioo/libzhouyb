//========================================================= 
/**@file Detector.h
 * @brief 状态检测器 
 * 
 * @date 2015-09-24   22:54:58
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace tools {
//--------------------------------------------------------- 
/// 卡状态 
struct ICardDetector
{
    /// 是否有卡 
    virtual bool IsPresent() = 0;
    /// 卡片是否已移除 
    virtual bool IsRemoved() = 0;
};
//--------------------------------------------------------- 
/// IC卡检测 
class ICCardDetector : public ICardDetector, public RefObject
{
protected:
    Ref<IICCardDevice> _icDev;
    string _sPowerOnArg;
public:
    ICCardDetector() {}

    /// 选择设备 
    inline void Select(const Ref<IICCardDevice>& dev, const char* sArg = NULL)
    {
        _icDev = dev;
        _sPowerOnArg = _strput(sArg);
    }
    /// 是否有卡 
    virtual bool IsPresent()
    {
        return _icDev->PowerOn(_sPowerOnArg.c_str(), NULL);
    }
    /// 卡片是否已移除 
    virtual bool IsRemoved()
    {
        return !IsPresent();
    }
};
//--------------------------------------------------------- 
class CCID_Detector : public ICardDetector, public RefObject
{
protected:
    /// 上次的卡片状态 
    DWORD _eventState;
    /// 读卡器名称 
    string _reader;
    /// 状态改变 
    bool _SCardStatusChanged(DWORD from, DWORD to)
    {
        DWORD currentState = 0;
        DWORD lastEventState = 0;

        bool bRet = false;

        lastEventState = _eventState;
        // 已经测试过卡片,等待卡片拿走进行下一测试  
        if(CCID_Device().SCardReaderState(currentState, _eventState, _reader.c_str()))
        {
            if((lastEventState & from) && (_eventState & to))
            {
                if(_eventState & SCARD_STATE_CHANGED)
                {
                    bRet = true;
                }
            }
        }

        return bRet;
    }
public:
    /// 选择设备 
    bool Select(const char* readerName)
    {
        // 查找全名 
        if(!_is_empty_or_null(readerName))
        {
            list<string> ccidlist;
            CCID_Device().EnumDevice(ccidlist);

            list<string>::iterator itr;
            for(itr = ccidlist.begin();itr != ccidlist.end(); ++itr)
            {
                if(StringConvert::Contains(itr->c_str(), readerName, true))
                {
                    _reader = (*itr);
                    return true;
                }
            }
            return false;
        }
        _reader = "";
        return true;
    }
    /// 获取读卡器名称 
    inline const char* GetReader() const
    {
        return _reader.c_str();
    }
    /// 是否有卡 
    virtual bool IsPresent()
    {
        // 当读卡器名称为空时,自动查找第一个有卡片的读卡器 
        if(_reader.length() < 1)
        {
            list<string> ccidlist;
            CCID_Device().EnumDevice(ccidlist);

            list<string>::iterator itr;
            for(itr = ccidlist.begin();itr != ccidlist.end(); ++itr)
            {
                _reader = (*itr);
                if(_SCardStatusChanged(SCARD_STATE_EMPTY, SCARD_STATE_PRESENT))
                    return true;
            }
            return false;
        }
        return _SCardStatusChanged(SCARD_STATE_EMPTY, SCARD_STATE_PRESENT);
    }
    /// 卡片是否被移除 
    virtual bool IsRemoved()
    {
        _eventState = SCARD_STATE_EMPTY;
        // 如果卡片被移走,或者设备直接被移除则直接返回 
        if(_SCardStatusChanged(SCARD_STATE_PRESENT, SCARD_STATE_EMPTY) || _eventState & SCARD_STATE_EMPTY)
            return true;
        return false;
    }
};
//--------------------------------------------------------- 
class IDCardDetector : public ICardDetector, public RefObject
{
protected:
    Ref<ISdtApi> _idDev;
public:
    /// 选择设备 
    inline void Select(const Ref<ISdtApi>& dev)
    {
        _idDev = dev;
    }
    /// 是否有卡 
    virtual bool IsPresent()
    {
        if(_idDev.IsNull())
            return false;
        return _idDev->FindIDCard();
    }
    /// 卡片是否被移除 
    virtual bool IsRemoved()
    {
        return !IsPresent();
    }
};
//--------------------------------------------------------- 
} // namespace tools
} // namespace application
} // namespace zhou_yb
//========================================================= 