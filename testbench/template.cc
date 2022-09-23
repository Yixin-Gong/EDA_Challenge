/**************************************************************************//**
  \file     template.cc
  \brief    This is a google test sample file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     22. September 2022
 ******************************************************************************/

#include "gtest/gtest.h"

/*!
    \brief      Addition functions
    \param[in]  a: First number
    \param[in]  b: Second number
    \return     Sum of two numbers
*/
static int add_operation(int a, int b) {
    return a + b;
}

/*! \brief Demonstrate some basic assertions. */
TEST(TestTemplate, AddOperation) {
    EXPECT_EQ(add_operation(1, 2), 3);
    EXPECT_EQ(add_operation(4, 1), 5);
    EXPECT_EQ(add_operation(4, 9), 13);
    EXPECT_EQ(add_operation(123, 25), 148);
    EXPECT_EQ(add_operation(341, 652), 993);
}
