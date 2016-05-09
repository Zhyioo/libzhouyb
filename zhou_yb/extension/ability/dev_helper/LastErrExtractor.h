//========================================================= 
/**@file LastErrExtractor.h 
 * @brief 错误信息提取器
 * 
 * @date 2014-05-17   21:19:49 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_LASTERREXTRACTOR_H_
#define _LIBZHOUYB_LASTERREXTRACTOR_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 错误数据结构结点 
struct ErrExtractorNode
{
    Ref<ILastErrBehavior> Err;
    string Msg;
};
//--------------------------------------------------------- 
/// 托管两个变量实现ILastErrBehavior
class LastErrInvoker : public ILastErrBehavior, public RefObject
{
protected:
    int* _pErr;
    string* _pMsg;
public:
    LastErrInvoker()
    {
        _pErr = NULL;
        _pMsg = NULL;
    }
    inline void Invoke(int& err, string& msg)
    {
        _pErr = &err;
        _pMsg = &msg;
    }
    inline bool IsValid() const
    {
        return _pErr != NULL && _pMsg != NULL;
    }
    inline void Uninvoke()
    {
        _pErr = NULL;
        _pMsg = NULL;
    }
    virtual int GetLastErr() const
    {
        if(_pErr != NULL)
            return (*_pErr);
        return DeviceError::Success;
    }
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage()
    {
        if(_pMsg != NULL)
            return _pMsg->c_str();
        return "";
    }
    /// 重置错误信息
    virtual void ResetErr()
    {
        (*_pErr) = DeviceError::Success;
        (*_pMsg) = "";
    }
};
//--------------------------------------------------------- 
/// 设备间错误信息提取器(将多层嵌套的适配器间的错误信息完整的提取出来=>将错误信息累加) 
class LastErrExtractor : public ILastErrBehavior, public RefObject
{
protected:
    /// 错误信息
    string _err_msg;
    /// 按照嵌套顺序叠加的设备 
    list<ErrExtractorNode> _err_pools;
public:
    LastErrExtractor()
    {
        IsFormatMSG = false;
        IsLayerMSG = false;
    }
    /// 按照嵌套顺序,底层的先选择,上层的后选择 
    void Select(const Ref<ILastErrBehavior>& err, const char* msg = NULL)
    {
        list<ErrExtractorNode>::iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            // 已经选择则只设置名称 
            if((*itr).Err == err)
            {
                (*itr).Msg = _strput(msg);
                break;
            }
        }
        _err_pools.push_back();
        _err_pools.back().Err = err;
        _err_pools.back().Msg = _strput(msg);
    }
    /// 移除选择的设备
    void Release(const Ref<ILastErrBehavior>& err)
    {
        list<ErrExtractorNode>::iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            if((*itr).Err == err)
            {
                _err_pools.erase(itr);
                break;
            }
        }
    }
    /// 移除全部选择的设备
    inline void Release()
    {
        _err_pools.clear();
    }
    /// 错误信息 
    virtual int GetLastErr() const
    {
        int iRet = DeviceError::Success;
        list<ErrExtractorNode>::const_iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            if(itr->Err.IsNull())
                continue;

            iRet = itr->Err->GetLastErr();
            // 遇到第一个失败则退出 
            if(iRet != DeviceError::Success)
                break;
        }

        return static_cast<int>(iRet);
    }
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage()
    {
        _err_msg = "";
        list<ErrExtractorNode>::iterator itr;
        // 从底层到顶层依次累加错误信息  
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            Ref<ILastErrBehavior> err = (*itr).Err;
            if(err.IsNull())
                continue;
            int lastErrId = err->GetLastErr();
            if(lastErrId != static_cast<int>(DeviceError::Success))
            {
                if(!IsFormatMSG)
                {
                    if(_err_msg.length() > 0)
                        _err_msg += " <- ";

                    if((*itr).Msg.length() > 0)
                    {
                        _err_msg += (*itr).Msg;
                        _err_msg += ':';
                    }
                    _err_msg +=  err->GetErrMessage();
                }
                else
                {
                    // 格式化 错误信息 
                    _err_msg += "{EXP:[";
                    _err_msg += (*itr).Msg;
                    _err_msg += "],ERR:[";
                    _err_msg += ArgConvert::ToString(lastErrId);
                    _err_msg += "],MSG:[";
                    _err_msg += err->GetErrMessage();
                    _err_msg += "]}";
                }
                if(!IsLayerMSG)
                    break;
            }
        }

        return _err_msg.c_str();
    }
    /// 重置错误信息
    virtual void ResetErr()
    {
        list<ErrExtractorNode>::iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            if(itr->Err.IsNull())
                continue;
            itr->Err->ResetErr();
        }
    }
    /// 是否需要格式化错误信息
    bool IsFormatMSG;
    /// 是否堆叠错误信息
    bool IsLayerMSG;
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_LASTERREXTRACTOR_H_
//========================================================= 