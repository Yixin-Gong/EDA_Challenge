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
#include <map>

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

void VCDParser::get_vcd_value_change_time(const std::string &filename) {
    long line = 0;
    std::map<long, int> map;
    std::map<long, int>::iterator it;
    std::map<long, int>::iterator itEnd;
    map.clear();
    std::ifstream file;
    file.open(filename, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        line++;
        if (read_string.c_str()[0] == '#') {
            int time_stamp = 0;
            sscanf(read_string.c_str(), "#%d", &time_stamp);
            map.insert(std::pair<long, int>(time_stamp, line));
        }
    }

//    it = map.begin();
//    itEnd = map.end();
//    while (it != itEnd) {
//        std::cout << it->first << ' ' << it->second << std::endl;
//        it++;
//    }
//    it = map.find(1300);
//    if (it == map.end())
//        std::cout << "we do not find the time_stamp" << std::endl;
//    else std::cout << it->second << std::endl;
}