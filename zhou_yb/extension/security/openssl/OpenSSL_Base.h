//========================================================= 
/**@file OpenSSL_Base.h 
 * @brief OpenSSL引用基本定义 
 * 
 * @date 2014-10-25   10:56:48 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include <openssl/rand.h>

#pragma comment(lib, "libeay32.lib")
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace security {
//--------------------------------------------------------- 
/// openssl基本对象的封装,主要用于自动释放对象 
template<class T, pointer TFunc>
class openssl_obj
{
protected:
    shared_obj<T*> _val;
public:
    openssl_obj(const T* val = NULL)
    {
        _val.obj() = const_cast<T*>(val);
    }
    openssl_obj(const openssl_obj& other)
    {
        _val = other._val;
    }
    virtual ~openssl_obj()
    {
        Free();
    }
    bool IsNull() const
    {
        return _val.obj() == NULL;
    }
    void Free()
    {
        if(_val.ref_count() < 2 && _val.obj())
        {
            typedef void(*FuncFree)(T*);
            if(TFunc != NULL)
            {
                FuncFree _FuncFree = reinterpret_cast<FuncFree>(TFunc);
                _FuncFree(_val.obj());
                _val.obj() = NULL;
            }
        }
    }
    operator T*()
    {
        return _val.obj();
    }
    operator const T*()
    {
        return _val.obj();
    }
    T& operator *()
    {
        return *(_val.obj());
    }
    T* operator ->()
    {
        return &(operator *());
    }
    const T& operator *() const
    {
        return *(_val.obj());
    }
    const T* operator ->() const
    {
        return &(operator *());
    }
    openssl_obj& operator =(const T* val)
    {
        Free();
        _val.obj() = const_cast<T*>(val);
        return *this;
    }
    openssl_obj& operator =(const openssl_obj& other)
    {
        Free();
        _val = other._val;
        return *this;
    }
};
//--------------------------------------------------------- 
class openssl_helper
{
protected:
    openssl_helper() {}
public:
    /// 获取指定字节长度的随机数 
    static bool Random(ByteBuilder& random, size_t randomlen)
    {
        size_t lastLen = random.GetLength();
        random.Append(static_cast<byte>(0x00), randomlen);
        byte* pDst = const_cast<byte*>(random.GetBuffer(lastLen));

        if(RAND_bytes(pDst, randomlen) <= 0)
        {
            random.RemoveTail(randomlen);
            return false;
        }

        return true;
    }
};
//--------------------------------------------------------- 
} // namespace security
} // namespace extension
} // namespace zhou_yb
//========================================================= 