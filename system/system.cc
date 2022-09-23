/**************************************************************************//**
  \file     system.cc
  \brief    This source file implements the function of software system information display.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#include "system.h"
#include <iostream>
#include "gitver.h"

/*!
    \brief  Show software system compilation time and version
*/
void SystemInfo::DisplayCompileInfo(const std::string &version) {
    std::cout << "Git version: " << GIT_VERSION_HASH << "\n";
    std::cout << "Compile time: " << __TIMESTAMP__ << "\n";
    std::cout << "Software version: " << version << "\n";
}
