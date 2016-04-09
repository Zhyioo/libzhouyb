//========================================================= 
/**@file LastErrExtractor.h 
 * @brief 错误信息提取器
 * 
 * @date 2014-05-17   21:19:49 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
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
    Ref<ILastErrBehavior> pDev;
    string devInfo;
};
/// 错误信息的数据结点  
struct ErrPKG
{
    string ERR;
    string VAL;
    string MSG;
};
/// 多个同层次设备的错误信息提取器
class UnityLastErrExtractor : 
    public BaseDevAdapterBehavior<ILastErrBehavior>, 
    public ILastErrBehavior
{
protected:
    /// 需要提取的错误列表
    list<Ref<ILastErrBehavior> > _err_pools;
public:
    /// 选择设备
    void Select(const Ref<ILastErrBehavior>& pDev)
    {
        list<Ref<ILastErrBehavior> >::iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            // 已经选择则直接退出
            if((*itr) == pDev)
                return;
        }
        _err_pools.push_back(pDev);
    }
    /// 释放设备
    void Release(const Ref<ILastErrBehavior>& pDev)
    {
        list_helper<Ref<ILastErrBehavior> >::remove(_err_pools, pDev);
    }
    /// 移除全部设备
    inline void Release()
    {
        _err_pools.clear();
    }
    /// 删除
    /// 获取上一次失败的错误码
    virtual int GetLastErr() const
    {
        list<Ref<ILastErrBehavior> >::const_iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            if((*itr).IsNull())
                continue;
            if((*itr)->GetLastErr() != DeviceError::Success)
                return (*itr)->GetLastErr();
        }
        return DeviceError::Success;
    }
    /// 获取上一次失败的错误信息
    virtual const char* GetErrMessage()
    {
        list<Ref<ILastErrBehavior> >::iterator itr;
        for(itr = _err_pools.begin();itr != _err_pools.end(); ++itr)
        {
            if((*itr).IsNull())
                continue;
            if((*itr)->GetLastErr() != DeviceError::Success)
                return (*itr)->GetErrMessage();
        }
        return "";
    }
};
//--------------------------------------------------------- 
/// 设备间错误信息提取器(将多层嵌套的适配器间的错误信息完整的提取出来=>将错误信息累加) 
class LastErrExtractor : public ILastErrBehavior
{
protected:
    /// 完整的错误信息 
    string _msg;
    /// 按照嵌套顺序叠加的设备 
    list<ErrExtractorNode> _devlink;
public:
    /// 按照嵌套顺序,底层的先选择,上层的后选择 
    void Select(const Ref<ILastErrBehavior>& dev, const char* devInfo = NULL)
    {
        list<ErrExtractorNode>::iterator itr;
        for(itr = _devlink.begin();itr != _devlink.end(); ++itr)
        {
            // 已经选择则只设置名称 
            if((*itr).pDev == dev)
            {
                (*itr).devInfo = _strput(devInfo);
                break;
            }
        }
        _devlink.push_back(ErrExtractorNode());
        _devlink.back().pDev = dev;
        _devlink.back().devInfo = _strput(devInfo);
    }
    /// 移除选择的设备
    void Release(const Ref<ILastErrBehavior>& pDev)
    {
        list<ErrExtractorNode>::iterator itr;
        for(itr = _devlink.begin();itr != _devlink.end(); ++itr)
        {
            if((*itr).pDev == pDev)
            {
                _devlink.erase(itr);
                break;
            }
        }
    }
    /// 移除全部选择的设备
    inline void Release()
    {
        _devlink.clear();
    }

    /// 错误信息 
    virtual int GetLastErr() const
    {
        int iRet = DeviceError::Success;

        list<ErrExtractorNode>::const_iterator itr;
        for(itr = _devlink.begin();itr != _devlink.end(); ++itr)
        {
            if(itr->pDev.IsNull())
                continue;

            iRet = itr->pDev->GetLastErr();
            // 遇到第一个失败则退出 
            if(iRet != DeviceError::Success)
                break;
        }

        return static_cast<int>(iRet);
    }
    /// 获取错误的描述信息(string字符串描述)
    virtual const char* GetErrMessage()
    {
        return ErrMessage(false, false);
    }
    /**
     * @brief 获取错误信息
     * @param [in] isBaseMsg 是否只获取原始的信息(不追加附加信息)
     * @param [in] isFirst 是否只获取第一个出错的信息 
     */ 
    const char* ErrMessage(bool isBaseMsg, bool isFirst) 
    {
        _msg = "";
        list<ErrExtractorNode>::iterator itr;
        // 从底层到顶层依次累加错误信息  
        for(itr = _devlink.begin();itr != _devlink.end(); ++itr)
        {
            if((*itr).pDev.IsNull())
                continue;

            if((*itr).pDev->GetLastErr() != static_cast<int>(DeviceError::Success))
            {
                if(!isBaseMsg)
                {
                    _msg += "=>ErrIn[ID:";
                    _msg += (*itr).devInfo;
                    _msg += "]";
                }
                _msg += (*itr).pDev->GetErrMessage();

                if(isFirst)
                    break;
            }
        }

        return _msg.c_str();
    }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb 
//========================================================= 