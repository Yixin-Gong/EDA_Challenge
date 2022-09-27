/**************************************************************************//**
  \file     parser.cc
  \brief    VCD parser source code file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     25. September 2022
 ******************************************************************************/

#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
void VCDParser::parse_vcd_header_(const std::string &filename) {
    std::ifstream file;
    file.open(filename, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }

    std::string read_string;
    unsigned int parse_status = 0;
    while (getline(file, read_string)) {
        switch (parse_status) {
            default:
            case 0:
                if (read_string.find("$date") != std::string::npos)
                    parse_status = 1;
                else if (read_string.find("$timescale") != std::string::npos)
                    parse_status = 2;
                else if (read_string.find("$comment") != std::string::npos) {
                    std::istringstream read_stream(read_string);
                    read_stream >> vcd_header_struct_.vcd_comment_str >> vcd_header_struct_.vcd_comment_str
                                >> vcd_header_struct_.vcd_comment_str >> vcd_header_struct_.vcd_comment_str;
                } else if (read_string.find("$end") != std::string::npos)
                    parse_status = 0;
                break;
            case 1:strptime(read_string.c_str(), "\t%c", &(vcd_header_struct_.vcd_create_time));
                parse_status = 0;
                break;
            case 2: {
                std::istringstream read_stream(read_string);
                read_stream >> vcd_header_struct_.vcd_time_scale >> vcd_header_struct_.vcd_time_unit;
                parse_status = 0;
            }
                break;
        }
        if (read_string.find("$comment") != std::string::npos)
            break;
    }
    file.close();

    char tmp_buf[64] = {0};
    strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d %H:%M:%S", &(vcd_header_struct_.vcd_create_time));
    std::cout << "File create time: " << tmp_buf << "\n";
    std::cout << "File time scale: " << this->vcd_header_struct_.vcd_time_scale << vcd_header_struct_.vcd_time_unit
              << "\n";
    std::cout << "File hash value: " << this->vcd_header_struct_.vcd_comment_str << "\n";
}

long long VCDParser::find_dumpvars_line(const std::string &filename) {
    std::ifstream file;
    file.open(filename, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return -1;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        if (read_string.find("$dumpvars") != std::string::npos) {
            vcd_signal_struct_.dump_vars_line = file.tellg();
        }
    }
    return vcd_signal_struct_.dump_vars_line;
}
void VCDParser::read_signal(const std::string &filename) {
    std::ifstream handle;
    long long now_location = find_dumpvars_line(filename);
    char buf[1024] = {0};
    handle.open(filename, std::ios_base::in);

    if (!handle.is_open()) {
        std::cout << "open file failed" << filename << std::endl;
    }
    handle.seekg(now_location, std::ios_base::beg);
    while (handle.getline(buf, sizeof(buf))) {
        now_location = handle.tellg();
        if (now_location == -1) {
            break;
        }
        std::cout << buf << std::endl;
    }
    handle.close();
}