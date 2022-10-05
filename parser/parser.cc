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

void VCDParser::get_vcd_scope(const std::string &filename) {
    long line = 0;
    std::map<std::string, unsigned long long> vcd_module;
    std::unordered_map<std::string, unsigned long long> vcd_signal;
    std::map<std::string, std::unordered_map<std::string, unsigned long long>> vcd_signal_umap;
    vcd_module.clear();
    vcd_signal_umap.clear();
    std::ifstream file;
    file.open(filename, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        line++;
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {
            int pos = read_string.rfind(" ");
            std::string scope_module = read_string.substr(14, pos - 14);
            vcd_module.insert(std::pair<std::string, unsigned long long>(scope_module, line));
            vcd_signal.clear();
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            struct VCDSignalStruct signal;
            int pos = read_string.rfind(" ");
            std::string signal_info = read_string.substr(5, pos - 5);
            vcd_module.insert(std::pair<std::string, unsigned long long>(signal_info, line));
            signal.vcd_signal_type = signal_info.substr(0, signal_info.find(' '));
            signal_info.erase(0, signal_info.find(' ') + 1);
            signal.vcd_signal_width = std::stoi(signal_info.substr(0, signal_info.find(' ')));
            signal_info.erase(0, signal_info.find(' ') + 1);
            signal.vcd_signal_label = signal_info.substr(0, signal_info.find(' '));
            signal_info.erase(0, signal_info.find(' ') + 1);
            signal.vcd_signal_title = signal_info;
            vcd_signal.insert(std::pair<std::string, unsigned long long>(signal.vcd_signal_title, line));
            vcd_signal_umap.insert(std::pair < std::string,
                                   std::unordered_map < std::string,
                                   unsigned long long >> (vcd_module.rbegin()->first, vcd_signal));
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {
            vcd_module.insert(std::pair<std::string, unsigned long long>("upscope", line));
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            std::string end_definitions = read_string.substr(1, read_string.find(' ') - 1);
            if (read_string.substr(1, read_string.find(' ') - 1) == "enddefinitions")
                vcd_module.insert(std::pair<std::string, unsigned long long>("enddefinitions", line));
        }
    }
    for (auto iter = vcd_module.begin(); iter != vcd_module.end(); ++iter) {
        std::cout << iter->first << " " << iter->second << std::endl;
    }

//    std::unordered_map<std::string, unsigned long long>::const_iterator module = vcd_module.find("test");
//    if (module == vcd_module.end()) {
//        std::cout << "not found"<<std::endl;
//    } else {
//        std::cout << module->first << " is " << module->second << std::endl;
//    }
//    for (auto iter = vcd_module.begin(); iter != vcd_module.end(); ++iter) {
//        std::cout << iter->second << " " << iter->first << std::endl;
//    }

//    std::unordered_map<std::string, unsigned long long>::const_iterator module = vcd_signal.find("reset");
//    if (module == vcd_signal.end()) {
//        std::cout << "not found"<<std::endl;
//    } else {
//        std::cout << module->first << " is " << module->second << std::endl;
//    }
//    for (auto iter = vcd_signal.begin(); iter != vcd_signal.end(); ++iter) {
//        std::cout << iter->first << " " << iter->second  << std::endl;
//    }
}

void VCDParser::get_vcd_value_change_time() {
    long line = 0;
    signal_map_.clear();
    std::ifstream file;
    file.open(vcd_filename_, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        line++;
        if (read_string.c_str()[0] == '#') {
            unsigned long long time_stamp = 0;
            sscanf(read_string.c_str(), "#%lld", &time_stamp);
            signal_map_.insert(std::pair<unsigned long long, unsigned long long>(time_stamp, file.tellg()));
        }
    }

//    std::map<unsigned long long, unsigned long long>::iterator it;
//    std::map<unsigned long long, unsigned long long>::iterator itEnd;
//    it = map.begin();
//    itEnd = map.end();
//    while (it != itEnd) {
//        std::cout << it->first << ' ' << it->second << std::endl;
//        it++;
//    }
}

void VCDParser::get_vcd_value_from_time(unsigned long long time) {
    std::map<unsigned long long, unsigned long long>::iterator it;
    it = signal_map_.find(time);
    if (it == signal_map_.end())
        std::cout << "we do not find the time_stamp" << std::endl;
    else std::cout << "Time Stamp: " << time << "  in byte: " << it->second << std::endl;

    std::ifstream file;
    file.open(vcd_filename_, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    file.seekg(it->second);
    std::string read_string;
    while (getline(file, read_string)) {
        if (read_string.c_str()[0] == '#')
            break;
        if (read_string.c_str()[0] == 'b') {
            std::string signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
            std::string signal_value = read_string.substr(1, read_string.find_first_of(' '));
            std::cout << "Signal " << signal_alias << " is " << signal_value << "\n";
        } else {
            std::string signal_alias = (char *) (&read_string.c_str()[1]);
            std::cout << "Signal " << signal_alias << " is " << read_string.c_str()[0] << "\n";
        }
        std::cout << read_string << "\n";
    }
}
