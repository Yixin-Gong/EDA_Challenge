/**************************************************************************//**
  \file     parser.cc
  \brief    VCD parser source code file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     25. September 2022
 ******************************************************************************/

#include "vcd_parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <list>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <iomanip>

void VCDParser::vcd_delete_time_stamp_buffer_() {
    if (time_stamp_first_buffer_.first_element != nullptr) {
        if (time_stamp_first_buffer_.next_buffer != nullptr) {
            struct VCDTimeStampBufferStruct *last_buffer = time_stamp_first_buffer_.previous_buffer;
            while (true) {
                last_buffer = last_buffer->previous_buffer;
                delete last_buffer->next_buffer->first_element;
                delete last_buffer->next_buffer;
                if (last_buffer == &time_stamp_first_buffer_)
                    break;
            }
        }
        delete time_stamp_first_buffer_.first_element;
    }
}

void VCDParser::parse_vcd_header_() {
    std::ifstream file;
    std::string read_string;
    unsigned int parse_status = 0;
    static char week[32], month[32];
    static const char kab_month_name[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    std::cout << "\nOpen file: " << vcd_filename_ << "\n";
    file.open(vcd_filename_, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "\nCannot open file " << vcd_filename_ << "\n";
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
    std::cout << "File hash value: " << this->vcd_header_struct_.vcd_comment_str << "\n\n";
}

void VCDParser::get_vcd_scope() {
    vcd_signal_alias_table_.clear();
    vcd_signal_list_.clear();
    std::ifstream file;
    file.open(vcd_filename_, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
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
            vcd_signal_alias_table_.insert(std::pair<std::string, struct VCDSignalStruct>(signal.vcd_signal_label,
                                                                                          signal));
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {
            std::string scope_module;
            int space_pos = 0;
            for (int pos = 0; read_string[pos] != 0 && space_pos != 3; pos++) {
                if (read_string[pos] == ' ') {
                    space_pos++;
                    continue;
                }
                if (space_pos == 2)
                    scope_module += read_string[pos];
            }
            if (vcd_signal_alias_table_.empty() != 1) {
                vcd_signal_list_.back().second = vcd_signal_alias_table_;
                vcd_signal_list_.emplace_back(scope_module, 0);
            } else {
                vcd_signal_list_.emplace_back(scope_module, 0);
            }
            vcd_signal_alias_table_.clear();
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {
            if (vcd_signal_alias_table_.empty() != 1) {
                vcd_signal_list_.back().second = vcd_signal_alias_table_;
                vcd_signal_alias_table_.clear();
            }
            vcd_signal_list_.emplace_back("upscope", 0);
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            if (read_string == "$enddefinitions $end") {
                vcd_signal_list_.back().second = vcd_signal_alias_table_;
                break;
            }
        }
    }

//    for (auto &iter : vcd_module_) {
//        std::cout << iter << std::endl;
//    }
//    for (auto &iter : vcd_signal_list_) {
//        std::cout << "Key:" << iter.first << std::endl;
//        for (auto &it : iter.second) {
//            std::cout << it.first << " Signal title: " << it.second.vcd_signal_title << std::endl;
//        }
//    }
}

struct VCDSignalStruct *VCDParser::get_vcd_signal(const std::string &label) {
    for (auto &iter : vcd_signal_list_)
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
    time_stamp_first_buffer_.previous_buffer = nullptr;
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
                next_buffer->previous_buffer = current_buffer;
                next_buffer->first_element = vcdtime_buf;
                next_buffer->next_buffer = nullptr;
                current_buffer = next_buffer;
            }
        }
    }
    for (uint64_t counter = buf_counter; counter < ktime_stamp_buffer_size_; ++counter) {
        current_buffer->first_element[buf_counter].timestamp = 0;
        current_buffer->first_element[buf_counter].location = 0;
    }
    time_stamp_first_buffer_.previous_buffer = current_buffer;
    fclose(fp);
}

VCDParser::VCDTimeStampStruct *VCDParser::get_time_stamp_from_pos(uint32_t pos) {
    struct VCDTimeStampBufferStruct *tmp_buffer = &time_stamp_first_buffer_;
    for (int counter = 0; counter < pos / ktime_stamp_buffer_size_; ++counter) {
        if (tmp_buffer->next_buffer == nullptr) return nullptr;
        tmp_buffer = tmp_buffer->next_buffer;
    }
    return &(tmp_buffer->first_element[pos % ktime_stamp_buffer_size_]);
}

bool VCDParser::get_position_using_timestamp(uint64_t *begin) {
    uint32_t begin_pos_est = *begin
        / (time_stamp_first_buffer_.first_element[1].timestamp - time_stamp_first_buffer_.first_element[0].timestamp);
    for (uint32_t last_begin_pos_est = 0;
         !(last_begin_pos_est == begin_pos_est || get_time_stamp_from_pos(begin_pos_est)->timestamp == *begin);) {
        last_begin_pos_est = begin_pos_est;
        if (get_time_stamp_from_pos(begin_pos_est) == nullptr)
            return false;
        begin_pos_est =
            begin_pos_est - ((int64_t) get_time_stamp_from_pos(begin_pos_est)->timestamp - (int64_t) *begin) /
                (int64_t) (get_time_stamp_from_pos(begin_pos_est)->timestamp
                    - get_time_stamp_from_pos(begin_pos_est - 1)->timestamp);
    }
    *begin = get_time_stamp_from_pos(begin_pos_est)->location;
    return true;
}

void VCDParser::get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time) {
    vcd_signal_flip_table_.clear();
    std::unordered_map<std::string, struct VCDSignalStatisticStruct>::iterator iter;
    static char buf[1024 * 1024];
    FILE *fp = fopen64(vcd_filename_.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "File open failed!\n";
        return;
    }
    fseeko64(fp, (long) time_stamp_first_buffer_.first_element[0].location, SEEK_SET);
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        VCDSignalStatisticStruct cnt{0, 0, 0, 0, 0, 0, 0};
        buf[strlen(buf) - 1] = '\0';
        std::string bufs = buf;
        if (buf[0] == '#' || bufs == "$dumpvars")
            continue;
        if (bufs == "$end")
            break;
        if (buf[0] == 'b') {
            std::string signal_alias = bufs.substr(bufs.find_last_of(' ') + 1, bufs.length());
            unsigned long signal_length = (bufs.substr(1, bufs.find_first_of(' '))).length();
            for (unsigned long count = signal_length - 1; count > 0; count--) {
                std::string temp_alias;
                temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                if (vcd_signal_flip_table_.find(temp_alias) == vcd_signal_flip_table_.end()) {
                    cnt.last_level_status = buf[signal_length - count];
                    cnt.final_level_status = 'x';
                    vcd_signal_flip_table_.insert(std::pair<std::string, struct VCDSignalStatisticStruct>(temp_alias,
                                                                                                          cnt));
                } else {
                    iter = vcd_signal_flip_table_.find(temp_alias);
                    iter->second.last_level_status = buf[signal_length - count];
                    iter->second.final_level_status = 'x';
                }
            }
        } else {
            std::string signal_alias = std::string((char *) (&buf[1])).substr(0, bufs.length());
            if (vcd_signal_flip_table_.find(signal_alias) == vcd_signal_flip_table_.end()) {
                cnt.last_level_status = buf[0];
                cnt.final_level_status = 'x';
                vcd_signal_flip_table_.insert(std::pair<std::string, struct VCDSignalStatisticStruct>(signal_alias,
                                                                                                      cnt));
            } else {
                iter = vcd_signal_flip_table_.find(signal_alias);
                iter->second.last_level_status = buf[0];
                iter->second.final_level_status = 'x';
            }
        }
    }

    static uint64_t current_timestamp = 0;
    std::unordered_map<std::string, int8_t> burr_hash_table;

    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        buf[strlen(buf) - 1] = '\0';
        std::string bufs = buf;
        if (buf[0] == '#') {
            for (auto &it : burr_hash_table)
                std::cout << "The Signal " << it.first << " glitch at " <<
                          "time " << current_timestamp << "\n";
            burr_hash_table.clear();
            current_timestamp = strtoll(&buf[1], nullptr, 0);
            continue;
        }
        if (buf[0] == 'b') {
            std::string signal_alias = bufs.substr(bufs.find_last_of(' ') + 1, bufs.length());
            unsigned long signal_length = (bufs.substr(1, bufs.find_first_of(' '))).length();
            for (unsigned long count = signal_length - 1; count > 0; count--) {
                std::string temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                iter = vcd_signal_flip_table_.find(temp_alias);
                uint64_t time_difference = vcd_statistic_time_(current_timestamp, iter);
                if (buf[signal_length - count] != iter->second.last_level_status)
                    vcd_statistic_burr_(buf, time_difference, temp_alias, iter,
                                        &burr_hash_table, signal_length - count);
            }
        } else {
            std::string signal_alias = std::string((char *) (&buf[1])).substr(0, strlen(buf));
            iter = vcd_signal_flip_table_.find(signal_alias);
            uint64_t time_difference = vcd_statistic_time_(current_timestamp, iter);
            vcd_statistic_burr_(buf, time_difference, signal_alias, iter, &burr_hash_table, 0);
        }
    }
    fclose(fp);

    for (auto &it : burr_hash_table)
        std::cout << "The Signal " << it.first << " glitch at " << "time " << current_timestamp << "\n";
    std::unordered_map<std::string, struct VCDSignalStatisticStruct>::iterator it;
    for (it = vcd_signal_flip_table_.begin(); it != vcd_signal_flip_table_.end(); it++) {
        vcd_statistic_time_(current_timestamp, it);
        if ((it->second.last_level_status != it->second.final_level_status)
            && (it->second.last_level_status != 'x'))
            it->second.total_invert_counter++;
        it->second.final_level_status = it->second.last_level_status;
        it->second.total_invert_counter--;
    }

    for (auto &i : vcd_signal_flip_table_)
        std::cout << i.first << " " << i.second.total_invert_counter << " " << i.second.signal1_time << " "
                  << i.second.signal0_time << " " << i.second.signalx_time << " sp: " <<
                  ((double) i.second.signal1_time
                      / (double) (i.second.signal1_time + i.second.signal0_time + i.second.signalx_time)) << "\n";
}

void VCDParser::printf_source_csv(const std::string &filepath) {
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::trunc);
    std::list<std::string> all_module;
    for (auto &iter : vcd_signal_list_) {
        if (iter.first == "upscope") {
            all_module.pop_back();
            continue;
        }
        std::string All_module;
        All_module.clear();
        for (auto &module : all_module)
            All_module += module + "/";
        all_module.emplace_back(iter.first);
        if (iter.second.empty() != 1) {
            for (auto &it : iter.second) {
                struct VCDSignalStatisticStruct signal{};
                if (it.second.vcd_signal_width == 1) {
                    if (vcd_signal_flip_table_.find(it.first) == vcd_signal_flip_table_.end())
                        std::cout << "Cannot find signal " << it.first << "\n";
                    else
                        signal = vcd_signal_flip_table_.find(it.first)->second;
                    file << All_module << iter.first << "." << it.second.vcd_signal_title
                         << "    tc = " << signal.total_invert_counter
                         << "    t1 = " << signal.signal1_time * vcd_header_struct_.vcd_time_scale
                         << vcd_header_struct_.vcd_time_unit
                         << "    t0 = " << signal.signal0_time * vcd_header_struct_.vcd_time_scale
                         << vcd_header_struct_.vcd_time_unit
                         << "    tx = " << signal.signalx_time * vcd_header_struct_.vcd_time_scale
                         << vcd_header_struct_.vcd_time_unit
                         << "    sp = " << std::to_string(((double) signal.signal1_time
                        / (double) (signal.signal0_time + signal.signal1_time + signal.signalx_time))) << std::endl;
                } else {
                    for (int wid_pos = 0; wid_pos < it.second.vcd_signal_width; wid_pos++) {
                        std::string
                            temp_alias = it.first + std::string("[") + std::to_string(wid_pos) + std::string("]");;
                        if (vcd_signal_flip_table_.find(temp_alias) == vcd_signal_flip_table_.end()) {
                            std::cout << "Cannot find signal " << temp_alias << "\n";
                            break;
                        } else
                            signal = vcd_signal_flip_table_.find(temp_alias)->second;
                        file << All_module << iter.first << "." << it.second.vcd_signal_title << "[" << wid_pos
                             << "]    tc = " << signal.total_invert_counter
                             << "    t1 = " << signal.signal1_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "    t0 = " << signal.signal0_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "    tx = " << signal.signalx_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "    sp = " << std::to_string(((double) signal.signal1_time
                            / (double) (signal.signal0_time + signal.signal1_time + signal.signalx_time))) << std::endl;
                    }
                }
            }
        }
    }
    file.close();
}

void VCDParser::printf_source_csv(const std::string &filepath, const std::string &label) {
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::trunc);
    std::list<std::string> all_module;
    for (auto &iter : vcd_signal_list_) {
        if (iter.first == "upscope") {
            all_module.pop_back();
            continue;
        }
        if (iter.first != label) {
            all_module.emplace_back(iter.first);
            continue;
        } else {
            std::string All_module;
            All_module.clear();
            for (auto &module : all_module) {
                All_module += module + "/";
            }
            all_module.emplace_back(iter.first);
            if (iter.second.empty() != 1) {
                for (auto &it : iter.second) {
                    struct VCDSignalStatisticStruct signal{};
                    if (vcd_signal_flip_table_.find(it.first) == vcd_signal_flip_table_.end())
                        std::cout << "Cannot find signal " << it.first << "\n";
                    else
                        signal = vcd_signal_flip_table_.find(it.first)->second;

                    if (it.second.vcd_signal_width == 1) {
                        file << All_module << iter.first << "." << it.second.vcd_signal_title
                             << "   tc= " << signal.total_invert_counter
                             << "   t1= " << signal.signal1_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "   t0= " << signal.signal0_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "   tx= " << signal.signalx_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "   sp= " << ((double) signal.signal1_time
                            / (double) (signal.signal0_time + signal.signal1_time + signal.signalx_time))
                             << std::endl;
                    } else {
                        for (int wid_pos = 0; wid_pos < it.second.vcd_signal_width; wid_pos++)
                            file << All_module << iter.first << "." << it.second.vcd_signal_title << "[" << wid_pos
                                 << "] "
                                 << std::endl;
                    }
                }
            }
        }
    }
    file.close();
}
uint64_t VCDParser::vcd_statistic_time_(uint64_t current_timestamp,
                                        std::unordered_map<std::string,
                                                           struct VCDSignalStatisticStruct>::iterator iter) {
    uint64_t time_difference = current_timestamp - iter->second.last_timestamp;
    switch (iter->second.last_level_status) {
        case '1':iter->second.signal1_time += time_difference;
            break;
        case '0':iter->second.signal0_time += time_difference;
            break;
        case 'x':iter->second.signalx_time += time_difference;
            break;
    }
    iter->second.last_timestamp = current_timestamp;
    return time_difference;
}

void VCDParser::vcd_statistic_burr_(const char *buf, uint64_t time_difference, const std::string &signal_alias,
                                    std::unordered_map<std::string, struct VCDSignalStatisticStruct>::iterator iter,
                                    std::unordered_map<std::string, int8_t> *burr_hash_table,
                                    uint32_t buf_index) {
    if (time_difference != 0) {
        if ((iter->second.last_level_status != iter->second.final_level_status)
            && (iter->second.last_level_status != 'x'))
            iter->second.total_invert_counter++;
        iter->second.final_level_status = iter->second.last_level_status;
    } else if (burr_hash_table->find(signal_alias) == burr_hash_table->end())
        burr_hash_table->insert(std::pair<std::string, int8_t>(signal_alias, {0}));
    iter->second.last_level_status = buf[buf_index];
}