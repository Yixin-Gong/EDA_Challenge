/**************************************************************************//**
  \file     system.cc
  \brief    This source file implements the function of software system information display.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#include "system.h"
#include <iostream>
#include <sys/stat.h>
#include "gitver.h"

/*!
    \brief  Show software system compilation time and version
*/
void SystemInfo::DisplayCompileInfo(const std::string &version) {
    std::cout << "Compiler version: " << CXX_COMPILER_VERSION << "\n";
    std::cout << "GTK library version: " << GTK_LIBRARY_VERSION << "\n";
    std::cout << "GTKMM library version: " << GTKMM_LIBRARY_VERSION << "\n";
    std::cout << "Git version: " << GIT_VERSION_HASH << "\n";
    std::cout << "Compile time: " << __TIMESTAMP__ << "\n";
    std::cout << "Software version: " << version << "\n";
}

bool SystemInfo::FileExists(const std::string &filename) {
    struct stat buffer{};
    return (stat(filename.c_str(), &buffer) == 0);
}
