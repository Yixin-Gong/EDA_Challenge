/**************************************************************************//**
  \file     template.cc
  \brief    This is a google test file for parser class.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     4. October 2022
 ******************************************************************************/

#include "gtest/gtest.h"
#include "parser.h"

TEST(TestParser, ParseHeader) {
    auto *parser = new VCDParser("../testcase/case0/test.vcd");
    VCDHeaderStruct *vcd_header = parser->get_vcd_header();
    EXPECT_TRUE((vcd_header->vcd_comment_str == "9ba2991b94438432"));
    EXPECT_TRUE((vcd_header->vcd_time_unit == "ns"));
    EXPECT_EQ(vcd_header->vcd_time_scale, 1);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_sec, 49);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_min, 3);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_hour, 16);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mday, 23);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mon, 8 - 1);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_year, 2022 - 1900);
    delete parser;

    parser = new VCDParser("../testcase/case17/test.vcd");
    vcd_header = parser->get_vcd_header();
    EXPECT_TRUE((vcd_header->vcd_comment_str == "fb9a912cdb4ffb79"));
    EXPECT_TRUE((vcd_header->vcd_time_unit == "ps"));
    EXPECT_EQ(vcd_header->vcd_time_scale, 100);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_sec, 43);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_min, 47);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_hour, 14);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mday, 22);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mon, 8 - 1);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_year, 2022 - 1900);
    delete parser;

    parser = new VCDParser("../testcase/case18/test.vcd");
    vcd_header = parser->get_vcd_header();
    EXPECT_TRUE((vcd_header->vcd_comment_str == "fb9a912cdb4ffb79"));
    EXPECT_TRUE((vcd_header->vcd_time_unit == "ps"));
    EXPECT_EQ(vcd_header->vcd_time_scale, 100);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_sec, 57);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_min, 59);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_hour, 14);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mday, 22);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mon, 8 - 1);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_year, 2022 - 1900);
    delete parser;

    parser = new VCDParser("../testcase/case19/test.vcd");
    vcd_header = parser->get_vcd_header();
    EXPECT_TRUE((vcd_header->vcd_comment_str == "fb9a912cdb4ffb79"));
    EXPECT_TRUE((vcd_header->vcd_time_unit == "ps"));
    EXPECT_EQ(vcd_header->vcd_time_scale, 100);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_sec, 44);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_min, 36);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_hour, 15);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mday, 22);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_mon, 8 - 1);
    EXPECT_EQ(vcd_header->vcd_create_time.tm_year, 2022 - 1900);
    delete parser;
}
