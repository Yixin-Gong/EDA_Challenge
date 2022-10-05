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
#include <cstring>
#include <cstdlib>
#include <cstdio>

VCDParser::VCDParser() {
    vcd_header_struct_.vcd_comment_str = "";
    vcd_header_struct_.vcd_time_unit = "";
    vcd_header_struct_.vcd_time_scale = 0;
    memset(&vcd_header_struct_.vcd_create_time, 0x00, sizeof(struct tm));
}

VCDParser::VCDParser(const std::string &filename) {
    vcd_filename_ = filename;
    parse_vcd_header_(filename);
}

void VCDParser::parse_vcd_header_(const std::string &filename) {
    std::ifstream file;
    std::string read_string;
    unsigned int parse_status = 0;

    std::cout << "Open file: " << filename << "\n";
    file.open(filename, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "Cannot open file " << filename << "\n";
        return;
    }

    while (getline(file, read_string)) {
        switch (parse_status) {
            default:
            case 0: {
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
            }
            case 1: {
                char week[32], month[32];
                static const char ab_month_name[12][4] =
                    {
                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                    };
                sscanf(read_string.c_str(),
                       "\t%s %s %d %d:%d:%d %d", week, month,
                       &vcd_header_struct_.vcd_create_time.tm_mday,
                       &vcd_header_struct_.vcd_create_time.tm_hour,
                       &vcd_header_struct_.vcd_create_time.tm_min,
                       &vcd_header_struct_.vcd_create_time.tm_sec,
                       &vcd_header_struct_.vcd_create_time.tm_year);
                vcd_header_struct_.vcd_create_time.tm_year = vcd_header_struct_.vcd_create_time.tm_year - 1900;

                for (int i = 0; i < 12; ++i) {
                    if (std::string(ab_month_name[i]) == std::string(month))
                        vcd_header_struct_.vcd_create_time.tm_mon = i;
                }

                parse_status = 0;
                break;
            }
            case 2: {
                std::istringstream read_stream(read_string);
                read_stream >> this->vcd_header_struct_.vcd_time_scale >> this->vcd_header_struct_.vcd_time_unit;
                parse_status = 0;
            }
                break;
        }
        if (read_string.find("$comment") != std::string::npos)
            break;
    }
    file.close();

    char tmp_buf[64] = {0};
    strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d %H:%M:%S", &(this->vcd_header_struct_.vcd_create_time));
    std::cout << "File create time: " << tmp_buf << "\n";
    std::cout << "File time scale: " << this->vcd_header_struct_.vcd_time_scale << vcd_header_struct_.vcd_time_unit
              << "\n";
    std::cout << "File hash value: " << this->vcd_header_struct_.vcd_comment_str << "\n";
}

void VCDParser::get_vcd_value_change_time() {
    signal_map_.clear();
    FILE *fp;
    fp = fopen64(vcd_filename_.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "File open failed!\n";
        return;
    }
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        if (buf[0] == '#') {
            unsigned long long time_stamp = 0;
            time_stamp = strtoull(&buf[1], nullptr, 0);
            signal_map_.insert(std::pair<unsigned long long, unsigned long long>(time_stamp, ftello64(fp)));
        }
    }
    fclose(fp);
    /*std::map<unsigned long long, unsigned long long>::iterator it;
    std::map<unsigned long long, unsigned long long>::iterator itEnd;
    it = signal_map_.begin();
    itEnd = signal_map_.end();
    while (it != itEnd) {
        std::cout << it->first << ' ' << it->second << std::endl;
        it++;
    }
*/
}

void VCDParser::get_vcd_value_from_time(unsigned long long time) {
    std::map<unsigned long long, unsigned long long>::iterator it;
    it = signal_map_.find(time);
    if (it == signal_map_.end())
        std::cout << "we do not find the time_stamp" << std::endl;
    else std::cout << "Time Stamp: " << time << "  in byte: " << it->second << std::endl;

    FILE *fp;
    fp = fopen64(vcd_filename_.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "File open failed!\n";
        return;
    }
    fseeko64(fp, it->second, SEEK_SET);
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        std::string bufs;
        int i = 0;
        while (buf[i] != '\0') {
            bufs += buf[i];
            i++;
        }
        if (buf[0] == '#')
            break;
        if (buf[0] == 'b') {
            std::string signal_alias = bufs.substr(bufs.find_last_of(' ') + 1, bufs.length());
            std::string signal_value = bufs.substr(1, bufs.find_first_of(' '));
            std::cout << "Signal " << signal_alias << " is " << signal_value << "\n";
        } else {
            std::string signal_alias = (char *) (&bufs.c_str()[1]);
            std::cout << "Signal " << signal_alias << " is " << bufs.c_str()[0] << "\n";
        }
        std::cout << bufs << "\n";
    }
}
