/**************************************************************************//**
  \file     template.cc
  \brief    This is a google test file for parser class.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     4. October 2022
 ******************************************************************************/

#include "gtest/gtest.h"
#include "vcd_parser.h"
#include "csv_parser.h"

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

TEST(TestParser, ParseSignal) {
    auto *parser = new VCDParser("../testcase/case0/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    VCDSignalStatisticStruct *signal = parser->get_signal_flip_info("!");
    EXPECT_EQ(signal->total_invert_counter, 10);
    EXPECT_EQ(signal->signal1_time, 250);
    EXPECT_EQ(signal->signal0_time, 250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"");
    EXPECT_EQ(signal->total_invert_counter, 1);
    EXPECT_EQ(signal->signal1_time, 100);
    EXPECT_EQ(signal->signal0_time, 400);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("#[0]");
    EXPECT_EQ(signal->total_invert_counter, 4);
    EXPECT_EQ(signal->signal1_time, 200);
    EXPECT_EQ(signal->signal0_time, 250);
    EXPECT_EQ(signal->signalx_time, 50);

    signal = parser->get_signal_flip_info("#[2]");
    EXPECT_EQ(signal->total_invert_counter, 1);
    EXPECT_EQ(signal->signal1_time, 50);
    EXPECT_EQ(signal->signal0_time, 400);
    EXPECT_EQ(signal->signalx_time, 50);

    signal = parser->get_signal_flip_info("#[3]");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 0);
    EXPECT_EQ(signal->signal0_time, 450);
    EXPECT_EQ(signal->signalx_time, 50);
    delete parser;

    parser = new VCDParser("../testcase/case1/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("\"G[1]");
    EXPECT_EQ(signal->total_invert_counter, 414);
    EXPECT_EQ(signal->signal1_time, 695750);
    EXPECT_EQ(signal->signal0_time, 141000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"W");
    EXPECT_EQ(signal->total_invert_counter, 114);
    EXPECT_EQ(signal->signal1_time, 28500);
    EXPECT_EQ(signal->signal0_time, 808250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"s[30]");
    EXPECT_EQ(signal->total_invert_counter, 491);
    EXPECT_EQ(signal->signal1_time, 207500);
    EXPECT_EQ(signal->signal0_time, 427000);
    EXPECT_EQ(signal->signalx_time, 202250);

    signal = parser->get_signal_flip_info("\"s[30]");
    EXPECT_EQ(signal->total_invert_counter, 491);
    EXPECT_EQ(signal->signal1_time, 207500);
    EXPECT_EQ(signal->signal0_time, 427000);
    EXPECT_EQ(signal->signalx_time, 202250);

    signal = parser->get_signal_flip_info("#");
    EXPECT_EQ(signal->total_invert_counter, 3347);
    EXPECT_EQ(signal->signal1_time, 418250);
    EXPECT_EQ(signal->signal0_time, 418500);
    EXPECT_EQ(signal->signalx_time, 0);
    delete parser;

    parser = new VCDParser("../testcase/case2/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("\"]%");
    EXPECT_EQ(signal->total_invert_counter, 20);
    EXPECT_EQ(signal->signal1_time, 471250);
    EXPECT_EQ(signal->signal0_time, 15000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("#");
    EXPECT_EQ(signal->total_invert_counter, 1945);
    EXPECT_EQ(signal->signal1_time, 243000);
    EXPECT_EQ(signal->signal0_time, 243250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("#`");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 486250);
    EXPECT_EQ(signal->signal0_time, 0);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"*[6]");
    EXPECT_EQ(signal->total_invert_counter, 96);
    EXPECT_EQ(signal->signal1_time, 26000);
    EXPECT_EQ(signal->signal0_time, 460250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"C[11]");
    EXPECT_EQ(signal->total_invert_counter, 191);
    EXPECT_EQ(signal->signal1_time, 210500);
    EXPECT_EQ(signal->signal0_time, 275750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("$s[0]");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 0);
    EXPECT_EQ(signal->signal0_time, 0);
    EXPECT_EQ(signal->signalx_time, 486250);
    delete parser;

    parser = new VCDParser("../testcase/case3/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("=");
    EXPECT_EQ(signal->total_invert_counter, 6);

    signal = parser->get_signal_flip_info("$");
    EXPECT_EQ(signal->total_invert_counter, 5);
    EXPECT_EQ(signal->signal1_time, 455986);
    EXPECT_EQ(signal->signal0_time, 457764);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("!");
    EXPECT_EQ(signal->total_invert_counter, 3655);
    EXPECT_EQ(signal->signal1_time, 456750);
    EXPECT_EQ(signal->signal0_time, 457000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("$=[31]");
    EXPECT_EQ(signal->total_invert_counter, 2);
    EXPECT_EQ(signal->signal1_time, 494750);
    EXPECT_EQ(signal->signal0_time, 207000);
    EXPECT_EQ(signal->signalx_time, 212000);

    signal = parser->get_signal_flip_info("##[31]");
    EXPECT_EQ(signal->total_invert_counter, 4);
    EXPECT_EQ(signal->signal1_time, 10000);
    EXPECT_EQ(signal->signal0_time, 903750);
    EXPECT_EQ(signal->signalx_time, 0);
    delete parser;

    parser = new VCDParser("../testcase/case4/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("k");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 0);
    EXPECT_EQ(signal->signal0_time, 2910000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"*[4]");
    EXPECT_EQ(signal->total_invert_counter, 2);
    EXPECT_EQ(signal->signal1_time, 500);
    EXPECT_EQ(signal->signal0_time, 2909500);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("#");
    EXPECT_EQ(signal->total_invert_counter, 11640);
    EXPECT_EQ(signal->signal1_time, 1455000);
    EXPECT_EQ(signal->signal0_time, 1455000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"]d");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 2910000);
    EXPECT_EQ(signal->signal0_time, 0);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("~P[1]");
    EXPECT_EQ(signal->total_invert_counter, 17);
    EXPECT_EQ(signal->signal1_time, 10500);
    EXPECT_EQ(signal->signal0_time, 2696750);
    EXPECT_EQ(signal->signalx_time, 202750);

    signal = parser->get_signal_flip_info("~P[1]");
    EXPECT_EQ(signal->total_invert_counter, 17);
    EXPECT_EQ(signal->signal1_time, 10500);
    EXPECT_EQ(signal->signal0_time, 2696750);
    EXPECT_EQ(signal->signalx_time, 202750);
    delete parser;

    parser = new VCDParser("../testcase/case5/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("\"L");
    EXPECT_EQ(signal->total_invert_counter, 10);
    EXPECT_EQ(signal->signal1_time, 4500);
    EXPECT_EQ(signal->signal0_time, 703750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"*[28]");
    EXPECT_EQ(signal->total_invert_counter, 266);
    EXPECT_EQ(signal->signal1_time, 71500);
    EXPECT_EQ(signal->signal0_time, 636750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\":P[0]");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 0);
    EXPECT_EQ(signal->signal0_time, 708250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("~I");
    EXPECT_EQ(signal->total_invert_counter, 1);
    EXPECT_EQ(signal->signal1_time, 506250);
    EXPECT_EQ(signal->signal0_time, 202000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"7M");
    EXPECT_EQ(signal->total_invert_counter, 2011);
    EXPECT_EQ(signal->signal1_time, 251250);
    EXPECT_EQ(signal->signal0_time, 457000);
    EXPECT_EQ(signal->signalx_time, 0);
    delete parser;

    parser = new VCDParser("../testcase/case6/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("\"\"");
    EXPECT_EQ(signal->total_invert_counter, 76);
    EXPECT_EQ(signal->signal1_time, 19000);
    EXPECT_EQ(signal->signal0_time, 22588750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"]e[0]");
    EXPECT_EQ(signal->total_invert_counter, 17501);
    EXPECT_EQ(signal->signal1_time, 4436500);
    EXPECT_EQ(signal->signal0_time, 17969000);
    EXPECT_EQ(signal->signalx_time, 202250);

    signal = parser->get_signal_flip_info("~#[9]");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 0);
    EXPECT_EQ(signal->signal0_time, 22607750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"!(");
    EXPECT_EQ(signal->total_invert_counter, 17254);
    EXPECT_EQ(signal->signal1_time, 4314500);
    EXPECT_EQ(signal->signal0_time, 18293250);
    EXPECT_EQ(signal->signalx_time, 0);
    delete parser;

    parser = new VCDParser("../testcase/case7/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("!");
    EXPECT_EQ(signal->total_invert_counter, 4165);
    EXPECT_EQ(signal->signal1_time, 520500);
    EXPECT_EQ(signal->signal0_time, 520750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("-$[17]");
    EXPECT_EQ(signal->total_invert_counter, 519);
    EXPECT_EQ(signal->signal1_time, 263500);
    EXPECT_EQ(signal->signal0_time, 777750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("~P[4]");
    EXPECT_EQ(signal->total_invert_counter, 252);
    EXPECT_EQ(signal->signal1_time, 454000);
    EXPECT_EQ(signal->signal0_time, 384500);
    EXPECT_EQ(signal->signalx_time, 202750);

    signal = parser->get_signal_flip_info("~d[28]");
    EXPECT_EQ(signal->total_invert_counter, 410);
    EXPECT_EQ(signal->signal1_time, 104500);
    EXPECT_EQ(signal->signal0_time, 734000);
    EXPECT_EQ(signal->signalx_time, 202750);

    signal = parser->get_signal_flip_info("\"!W");
    EXPECT_EQ(signal->total_invert_counter, 632);
    EXPECT_EQ(signal->signal1_time, 662250);
    EXPECT_EQ(signal->signal0_time, 379000);
    EXPECT_EQ(signal->signalx_time, 0);
    delete parser;

    parser = new VCDParser("../testcase/case8/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("\"A[21]");
    EXPECT_EQ(signal->total_invert_counter, 56);
    EXPECT_EQ(signal->signal1_time, 14000);
    EXPECT_EQ(signal->signal0_time, 9777750);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"M");
    EXPECT_EQ(signal->total_invert_counter, 901);
    EXPECT_EQ(signal->signal1_time, 405500);
    EXPECT_EQ(signal->signal0_time, 9386250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"i");
    EXPECT_EQ(signal->total_invert_counter, 93);
    EXPECT_EQ(signal->signal1_time, 7160500);
    EXPECT_EQ(signal->signal0_time, 2392500);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"]3[31]");
    EXPECT_EQ(signal->total_invert_counter, 5036);
    EXPECT_EQ(signal->signal1_time, 1582000);
    EXPECT_EQ(signal->signal0_time, 8007500);
    EXPECT_EQ(signal->signalx_time, 202250);

    signal = parser->get_signal_flip_info("$");
    EXPECT_EQ(signal->total_invert_counter, 39167);
    EXPECT_EQ(signal->signal1_time, 4895750);
    EXPECT_EQ(signal->signal0_time, 4896000);
    EXPECT_EQ(signal->signalx_time, 0);
    delete parser;

    parser = new VCDParser("../testcase/case9/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("#");
    EXPECT_EQ(signal->total_invert_counter, 263991);
    EXPECT_EQ(signal->signal1_time, 32998750);
    EXPECT_EQ(signal->signal0_time, 32999000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("\"]X[1]");
    EXPECT_EQ(signal->total_invert_counter, 230);
    EXPECT_EQ(signal->signal1_time, 65630500);
    EXPECT_EQ(signal->signal0_time, 367250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("#%");
    EXPECT_EQ(signal->total_invert_counter, 1);
    EXPECT_EQ(signal->signal1_time, 65795750);
    EXPECT_EQ(signal->signal0_time, 202000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("z[16]");
    EXPECT_EQ(signal->total_invert_counter, 58);
    EXPECT_EQ(signal->signal1_time, 14500);
    EXPECT_EQ(signal->signal0_time, 65983250);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("#_[31]");
    EXPECT_EQ(signal->total_invert_counter, 0);
    EXPECT_EQ(signal->signal1_time, 0);
    EXPECT_EQ(signal->signal0_time, 65795000);
    EXPECT_EQ(signal->signalx_time, 202750);

    parser = new VCDParser("../testcase/case10/test.vcd");
    parser->get_vcd_scope();
    parser->get_vcd_signal_flip_info();
    signal = parser->get_signal_flip_info("\"");
    EXPECT_EQ(signal->total_invert_counter, 4469);
    EXPECT_EQ(signal->signal1_time, 446950000);
    EXPECT_EQ(signal->signal0_time, 447000000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("=,[260]");
    EXPECT_EQ(signal->total_invert_counter, 6);
    EXPECT_EQ(signal->signal1_time, 1800000);
    EXPECT_EQ(signal->signal0_time, 891150000);
    EXPECT_EQ(signal->signalx_time, 1000000);

    signal = parser->get_signal_flip_info("`B[2]");
    EXPECT_EQ(signal->total_invert_counter, 1);
    EXPECT_EQ(signal->signal1_time, 788800000);
    EXPECT_EQ(signal->signal0_time, 105150000);
    EXPECT_EQ(signal->signalx_time, 0);

    signal = parser->get_signal_flip_info("`v");
    EXPECT_EQ(signal->total_invert_counter, 2);
    EXPECT_EQ(signal->signal1_time, 800000);
    EXPECT_EQ(signal->signal0_time, 892150000);
    EXPECT_EQ(signal->signalx_time, 1000000);

    signal = parser->get_signal_flip_info("#wI[5]");
    EXPECT_EQ(signal->total_invert_counter, 2);
    EXPECT_EQ(signal->signal1_time, 626500000);
    EXPECT_EQ(signal->signal0_time, 160300000);
    EXPECT_EQ(signal->signalx_time, 107150000);
}

TEST(RegressionTest, VCDParser) {
    auto *vcd_parser = new VCDParser("../testcase/case0/test.vcd");
    auto *csv_parser = new CSVParser("../testcase/case0/test.csv");
    csv_parser->parse_csv();
    csv_parser->get_vcd_scope("../testcase/case0/test.vcd");
    csv_parser->csv_find_vcd();
    vcd_parser->get_vcd_scope();
    vcd_parser->get_vcd_signal_flip_info();

    auto *vcd_flip_table = vcd_parser->get_signal_flip_table();
    for (const auto &it : *vcd_flip_table) {
        auto *csv_signal = csv_parser->find_signal(it.first);
        auto *vcd_signal = vcd_parser->get_signal_flip_info(it.first);
        if (csv_parser == nullptr || vcd_signal == nullptr)
            std::cout << "Cannot find signal " << it.first << "in csv" << std::endl;
        else {
            EXPECT_EQ(csv_signal->tc, vcd_signal->total_invert_counter);
            EXPECT_EQ(csv_signal->t1, vcd_signal->signal1_time);
            EXPECT_EQ(csv_signal->t0, vcd_signal->signal0_time);
            EXPECT_EQ(csv_signal->tx, vcd_signal->signalx_time);
        }
    }
}