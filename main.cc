/**************************************************************************//**
  \file     main.cc
  \brief    This source file is the program entry file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#ifdef IS_NOT_RUNNING_GOOGLE_TEST
#include <iostream>
#include "ncurses.h"
#include "system.h"
#else
#include "gtest/gtest.h"
#endif

/*!
    \brief      Program entry
    \param[in]  argc: Number of command-line arguments sent to the main function at program runtime
    \param[in]  argv: Store an array of pointers to string parameters
    \return     Program execution results
*/
int main(int argc, char **argv) {
#ifdef IS_NOT_RUNNING_GOOGLE_TEST
    SystemInfo::DisplayCompileInfo();
    std::cout << "Hello, World!" << std::endl;
    return 0;
#else
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#endif
}
