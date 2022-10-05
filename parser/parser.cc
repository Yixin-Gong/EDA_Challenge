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