//========================================================= 
/**@file CommandDriver.cpp
 * @brief CommandDriver����Դ�ļ�
 * 
 * @date 2016-04-15   13:52:40
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "CommandDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// ��������
list<Command> Command::_Cmds;
/// ������Ŀ
size_t Command::_Count = 0;
//--------------------------------------------------------- 
} // namespace driver
} // namespace application 
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 