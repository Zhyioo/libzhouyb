//========================================================= 
/**@file SrcBase.h 
 * @brief 包含base库中的一些全局变量、函数 
 * 
 * @date 2011-10-17   19:32:35 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
// @warning 务必保证该资源文件第一个被加载,使得内存池第一个构造最后一个析构  
#include "../src/allocator.cpp"
// base
#include "../src/base/Convert.cpp"
#include "../src/base/Logger.cpp"
#include "../src/base/ArgParser.cpp"
//=========================================================