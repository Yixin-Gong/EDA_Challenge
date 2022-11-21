//
// Created by ubuntu on 22-11-17.
//
#include "csv_parser.h"
#include "vcd_parser.h"
#include "gtest/gtest.h"
#include <fstream>

void print_signal_original_info(const std::string &filename, const std::string &signal_name) {
    std::ofstream output_file;
    output_file.open("./signal_extract.txt", std::ios::out | std::ios::trunc);
    uint64_t current_timestamp = 0, last_timestamp = 0;
    FILE *fp_ = fopen64(filename.c_str(), "r");
    static char reading_buffer[1024 * 1024] = {0};
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        if (reading_buffer[0] == '#')
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
        else if (reading_buffer[0] == 'b') {
            std::string signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
            if (signal_alias == signal_name) {
                if (current_timestamp != last_timestamp || current_timestamp == 0) {
                    output_file << "#" << std::to_string(current_timestamp) << "\n";
                    last_timestamp = current_timestamp;
                }
                output_file << read_string << "\n";
            }
        } else if (std::string(&reading_buffer[1]) == signal_name) {
            if (current_timestamp != last_timestamp || current_timestamp == 0) {
                output_file << "#" << std::to_string(current_timestamp) << "\n";
                last_timestamp = current_timestamp;
            }
            output_file << read_string << "\n";
        }
    }
    output_file.close();
}

TEST(RegressionTest, VCDParser) {
    for (int counter = 0; counter < 5; ++counter) {
        auto *vcd_parser = new VCDParser("../testcase/case" + std::to_string(counter) + "/test.vcd");
        auto *csv_parser = new CSVParser("../testcase/case" + std::to_string(counter) + "/test.csv");
        csv_parser->parse_csv();
        csv_parser->get_vcd_scope("../testcase/case" + std::to_string(counter) + "/test.vcd");
        csv_parser->csv_find_vcd();
        vcd_parser->get_vcd_scope();
        vcd_parser->get_vcd_signal_flip_info();
        uint64_t time_scale = vcd_parser->get_vcd_header()->vcd_time_scale;

        auto *vcd_flip_table = vcd_parser->get_signal_flip_table();
        for (const auto &it : *vcd_flip_table) {
            auto *csv_signal = csv_parser->find_signal(it.first);
            auto *vcd_signal = vcd_parser->get_signal_flip_info(it.first);
            if (csv_signal != nullptr && vcd_signal != nullptr) {
                EXPECT_EQ(csv_signal->tc, vcd_signal->total_invert_counter);
                EXPECT_EQ(csv_signal->t1, vcd_signal->signal1_time * time_scale);
                EXPECT_EQ(csv_signal->t0, vcd_signal->signal0_time * time_scale);
                EXPECT_EQ(csv_signal->tx, vcd_signal->signalx_time * time_scale);
            }
        }
        delete csv_parser;
        delete vcd_parser;
    }
}
