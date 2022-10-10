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
#include <list>
#include <cstdlib>
#include <cstdio>

VCDParser::VCDParser() {
    vcd_header_struct_.vcd_comment_str = "";
    vcd_header_struct_.vcd_time_unit = "";
    vcd_header_struct_.vcd_time_scale = 0;
    time_stamp_first_buffer_.first_element = nullptr;
    time_stamp_first_buffer_.next_buffer = nullptr;
    memset(&vcd_header_struct_.vcd_create_time, 0x00, sizeof(struct tm));
}

VCDParser::VCDParser(const std::string &filename) {
    vcd_filename_ = filename;
    parse_vcd_header_(filename);
    time_stamp_first_buffer_.first_element = nullptr;
    time_stamp_first_buffer_.next_buffer = nullptr;
}

VCDParser::~VCDParser() {
    vcd_delete_time_stamp_buffer_();
}

void VCDParser::vcd_delete_time_stamp_buffer_() {
    if (time_stamp_first_buffer_.first_element != nullptr) {
        while (true) {
            struct VCDTimeStampBufferStruct *current_buffer = &time_stamp_first_buffer_;
            struct VCDTimeStampBufferStruct *last_buffer{};
            if (time_stamp_first_buffer_.next_buffer == nullptr) {
                delete time_stamp_first_buffer_.first_element;
                return;
            }
            while (true) {
                if (current_buffer->next_buffer == nullptr)
                    break;
                last_buffer = current_buffer;
                current_buffer = current_buffer->next_buffer;
            }
            delete current_buffer->first_element;
            delete current_buffer;
            last_buffer->next_buffer = nullptr;
            if (last_buffer == &time_stamp_first_buffer_)
                break;
        }
        delete time_stamp_first_buffer_.first_element;
    }
}

void VCDParser::parse_vcd_header_(const std::string &filename) {
    std::ifstream file;
    std::string read_string;
    unsigned int parse_status = 0;
    static char week[32], month[32];
    static const char kab_month_name[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    std::cout << "\nOpen file: " << filename << "\n";
    file.open(filename, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "\nCannot open file " << filename << "\n";
        return;
    }

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
            case 1:
                sscanf(read_string.c_str(),
                       "\t%s %s %d %d:%d:%d %d", week, month,
                       &vcd_header_struct_.vcd_create_time.tm_mday,
                       &vcd_header_struct_.vcd_create_time.tm_hour,
                       &vcd_header_struct_.vcd_create_time.tm_min,
                       &vcd_header_struct_.vcd_create_time.tm_sec,
                       &vcd_header_struct_.vcd_create_time.tm_year);
                vcd_header_struct_.vcd_create_time.tm_year = vcd_header_struct_.vcd_create_time.tm_year - 1900;
                for (int i = 0; i < 12; ++i)
                    if (std::string(kab_month_name[i]) == std::string(month))
                        vcd_header_struct_.vcd_create_time.tm_mon = i;
                parse_status = 0;
                break;
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

void VCDParser::get_vcd_scope() {
    std::list<std::string> vcd_module;
    vcd_module.clear();
    vcd_signal_.clear();
    vcd_signal_map_.clear();
    std::ifstream file;
    file.open(vcd_filename_, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {
            unsigned long pos = read_string.rfind(' ');
            std::string scope_module = read_string.substr(14, pos - 14);
            if (vcd_signal_.empty() != 1)
                vcd_signal_map_.emplace(std::pair<std::string, std::unordered_map<std::string, struct VCDSignalStruct>>
                                            (vcd_module.back(), vcd_signal_));
            vcd_module.push_back(scope_module);
            vcd_signal_.clear();
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            struct VCDSignalStruct signal;
            int space_pos = 0;
            std::string width;
            for (int pos = 0; read_string[pos] != 0; pos++) {
                if (read_string[pos] == ' ') {
                    space_pos++;
                    if (space_pos == 5) {
                        signal.vcd_signal_width = std::stoi(width);
                        break;
                    }
                    continue;
                }
                switch (space_pos) {
                    case 1:signal.vcd_signal_type += read_string[pos];
                        break;
                    case 2:width += read_string[pos];
                        break;
                    case 3:signal.vcd_signal_label += read_string[pos];
                        break;
                    case 4:signal.vcd_signal_title += read_string[pos];
                        break;
                    default:break;
                }
            }
            vcd_signal_.insert(std::pair<std::string, struct VCDSignalStruct>(signal.vcd_signal_label, signal));
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            if (read_string.substr(1, read_string.find(' ') - 1) == "enddefinitions") {
                vcd_signal_map_.emplace(std::pair<std::string, std::unordered_map<std::string, struct VCDSignalStruct>>
                                            (vcd_module.back(), vcd_signal_));
                break;
            }
        }
    }

//    for(auto &iter:vcd_module){
//        std::cout << iter << std::endl;
//    }
//    for (auto &iter : vcd_signal_map_) {
//        std::cout << "Key:" << iter.first << std::endl;
//        for (auto &it : iter.second) {
//            std::cout << it.first << " Signal title: " << it.second.vcd_signal_title << std::endl;
//        }
//    }
}

struct VCDSignalStruct *VCDParser::get_vcd_signal(const std::string &label) {
    for (auto &iter : vcd_signal_map_)
        if (iter.second.find(label) != nullptr)
            return &(iter.second.find(label)->second);
    std::cout << "Cannot find alias named" << label << "\n";
    return nullptr;
}

void VCDParser::get_vcd_value_change_time() {
    uint64_t buf_counter = 0;
    static char reading_buffer[1024 * 1024] = {0};
    vcd_delete_time_stamp_buffer_();
    auto *vcdtime_buf = new struct VCDTimeStampStruct[ktime_stamp_buffer_size_];
    time_stamp_first_buffer_.first_element = vcdtime_buf;
    time_stamp_first_buffer_.next_buffer = nullptr;
    struct VCDTimeStampBufferStruct *current_buffer = &time_stamp_first_buffer_;
    FILE *fp = fopen64(vcd_filename_.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "File open failed!\n";
        return;
    }
    while (fgets(reading_buffer, sizeof(reading_buffer), fp) != nullptr) {
        if (reading_buffer[0] == '#') {
            current_buffer->first_element[buf_counter].timestamp = strtoull(&reading_buffer[1], nullptr, 0);
            current_buffer->first_element[buf_counter].location = ftello64(fp);
            buf_counter++;
            if (buf_counter == ktime_stamp_buffer_size_) {
                buf_counter = 0;
                auto *next_buffer = new struct VCDTimeStampBufferStruct;
                vcdtime_buf = new struct VCDTimeStampStruct[ktime_stamp_buffer_size_];
                current_buffer->next_buffer = next_buffer;
                next_buffer->first_element = vcdtime_buf;
                next_buffer->next_buffer = nullptr;
                current_buffer = next_buffer;
                std::cout << "current buffer was full!\n";
            }
        }
    }
    for (uint64_t counter = buf_counter; counter < ktime_stamp_buffer_size_; ++counter) {
        current_buffer->first_element[buf_counter].timestamp = 0;
        current_buffer->first_element[buf_counter].location = 0;
    }
    fclose(fp);
}

void VCDParser::get_vcd_value_from_time_range(uint64_t begin_time, uint64_t end_time) {
    uint64_t time_location = 0;
    struct VCDTimeStampBufferStruct *current_buffer = &time_stamp_first_buffer_;
    while (true) {
        for (uint64_t counter = 0; counter < ktime_stamp_buffer_size_; ++counter) {
            if (current_buffer->first_element[counter].timestamp == begin_time) {
                time_location = current_buffer->first_element[counter].location;
                std::cout << "Find time " << begin_time << " in byte " << time_location << "\n";
                break;
            }
        }
        if (current_buffer->next_buffer == nullptr || time_location != 0)
            break;
        current_buffer = current_buffer->next_buffer;
    }

    if (time_location == 0) {
        std::cout << "Can't find time " << begin_time << " in file.\n";
        return;
    }

    static char reading_buffer[1024 * 1024] = {0};
    FILE *fp = fopen64(vcd_filename_.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "File open failed!\n";
        return;
    }
    fseeko64(fp, (long) time_location, SEEK_SET);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string bufs = reading_buffer;
        if (reading_buffer[0] == '$')
            continue;
        if (reading_buffer[0] == '#')
            break;
        if (reading_buffer[0] == 'b') {
            std::string signal_alias = bufs.substr(bufs.find_last_of(' ') + 1, bufs.length());
            std::string signal_value = bufs.substr(1, bufs.find_first_of(' '));
            std::cout << "Signal " << signal_alias << " is " << signal_value << "\n";
        } else {
            std::string signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, bufs.length());
            std::cout << "Signal " << signal_alias << " is " << reading_buffer[0] << "\n";
        }
    }
}
#pragma clang diagnostic pop