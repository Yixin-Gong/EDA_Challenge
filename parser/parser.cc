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
                    read_stream >> vcd_comment_str_ >> vcd_comment_str_ >> vcd_comment_str_ >> vcd_comment_str_;
                } else if (read_string.find("$end") != std::string::npos)
                    parse_status = 0;
                break;
            case 1:strptime(read_string.c_str(), "\t%c", &(this->vcd_create_time_));
                parse_status = 0;
                break;
            case 2: {
                int time_number;
                char time_scale[2];
                sscanf(read_string.c_str(), "\t%d%s", &time_number, time_scale);
                if (time_scale[0] == 'n' && time_scale[1] == 's')
                    this->vcd_time_scale_ = time_number;
                else if (time_scale[0] == 'u' && time_scale[1] == 's')
                    this->vcd_time_scale_ = time_number * 1000;
                else if (time_scale[0] == 'm' && time_scale[1] == 's')
                    this->vcd_time_scale_ = time_number * 1000000;
                else if (time_scale[0] == 's')
                    this->vcd_time_scale_ = time_number * 1000000000;
                parse_status = 0;
            }
                break;
        }
        if (read_string.find("$comment") != std::string::npos)
            break;
    }
    file.close();

    char tmp_buf[64] = {0};
    strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d %H:%M:%S", &(this->vcd_create_time_));
    std::cout << "File create time: " << tmp_buf << "\n";
    std::cout << "File time scale: " << this->vcd_time_scale_ << "ns\n";
    std::cout << "File hash value: " << this->vcd_comment_str_ << "\n";
}
