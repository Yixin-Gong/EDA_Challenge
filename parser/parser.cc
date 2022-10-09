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
#include <unordered_map>

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

void VCDParser::get_vcd_scope() {
    long line = 0;
    std::vector<std::string> vcd_module;
    std::unordered_map<std::string, struct VCDSignalStruct> vcd_signal;
    std::map<std::string, std::unordered_map<std::string, struct VCDSignalStruct>> vcd_signal_umap;
    vcd_module.clear();
    vcd_signal_umap.clear();
    std::ifstream file;
    file.open(vcd_filename_, std::ios_base::in);
    if (!file.is_open()) {
        std::cout << "File open failed!\n";
        return;
    }
    std::string read_string;
    while (getline(file, read_string)) {
        line++;
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {

            unsigned long pos = read_string.rfind(' ');
            std::string scope_module = read_string.substr(14, pos - 14);
            vcd_module.push_back(scope_module);
            vcd_signal.clear();
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            struct VCDSignalStruct signal;
            unsigned long pos = read_string.rfind(' ');
            std::string signal_info = read_string.substr(5, pos - 5);
            signal.vcd_signal_type = signal_info.substr(0, signal_info.find(' '));
            signal_info.erase(0, signal_info.find(' ') + 1);
            signal.vcd_signal_width = std::stoi(signal_info.substr(0, signal_info.find(' ')));
            signal_info.erase(0, signal_info.find(' ') + 1);
            signal.vcd_signal_label = signal_info.substr(0, signal_info.find(' '));
            signal_info.erase(0, signal_info.find(' ') + 1);
            signal.vcd_signal_title = signal_info;
            vcd_signal.insert(std::pair<std::string, struct VCDSignalStruct>(signal.vcd_signal_label, signal));
            vcd_signal_umap.insert(std::pair<std::string,
                                             std::unordered_map<std::string,
                                                                struct VCDSignalStruct >>(vcd_module.back(),
                                                                                          vcd_signal));
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            std::string end_definitions = read_string.substr(1, read_string.find(' ') - 1);
            if (read_string.substr(1, read_string.find(' ') - 1) == "enddefinitions")
                break;
        }
    }
//    for(auto &iter:vcd_module){
//        std::cout << iter << std::endl;
//    }
//    for (auto & iter : vcd_signal_umap) {
//        std::cout << iter.first << std::endl;
//        for(auto &it:iter.second){
//            std::cout<<it.first<<std::endl;
//            std::cout<<it.second.vcd_signal_type<<' '<<it.second.vcd_signal_width
//            <<' '<<it.second.vcd_signal_title<<std::endl;
//        }
//    }
//    for(auto &iter:vcd_signal){
//        std::cout << iter.first << std::endl;
//        std::cout<<iter.second.vcd_signal_type<<' '<<iter.second.vcd_signal_width
//            <<' '<<iter.second.vcd_signal_title<<std::endl;
//    }
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

void VCDParser::get_vcd_value_from_time(uint64_t time) {
    std::map<uint64_t, uint64_t>::iterator it;
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
        buf[strlen(buf) - 1] = '\0';
        std::string bufs = buf;
        if (buf[0] == '$')
            continue;
        if (buf[0] == '#')
            break;
        if (buf[0] == 'b') {
            std::string signal_alias = bufs.substr(bufs.find_last_of(' ') + 1, bufs.length());
            std::string signal_value = bufs.substr(1, bufs.find_first_of(' '));
            std::cout << "Signal " << signal_alias << " is " << signal_value << "\n";
        } else {
            std::string signal_alias = std::string((char *) (&buf[1])).substr(0, bufs.length());
            std::cout << "Signal " << signal_alias << " is " << buf[0] << "\n";
        }
    }
}

void VCDParser::value_change_counter_(uint64_t time) {
    std::unordered_map<std::string, struct VCDSignalStatisticStruct>::iterator iter;
    static char buf[1024];
    FILE *fp = fopen64(vcd_filename_.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "File open failed!\n";
        return;
    }

    fseeko64(fp, (long) signal_map_.find(time)->second, SEEK_SET);
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        buf[strlen(buf) - 1] = '\0';
        std::string bufs = buf;
        if (buf[0] == '#')
            continue;
        if (bufs == "$dumpvars")
            continue;
        if (bufs == "$end")
            break;
        if (buf[0] == 'b')
            continue;
        VCDSignalStatisticStruct cnt{0, 0, 0, 0, 0, 0};
        std::string signal_alias = std::string((char *) (&buf[1])).substr(0, bufs.length());
        cnt.last_level_status = buf[0];
        counters.insert(std::pair<std::string, struct VCDSignalStatisticStruct>(signal_alias, cnt));
    }

    static uint64_t current_timestamp = 0;
    std::unordered_map<std::string, uint16_t> burr_hash_table;
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        buf[strlen(buf) - 1] = '\0';
        if (buf[0] == '#') {
            for (auto &it : burr_hash_table)
                std::cout << "Warning: Signal " << it.first << " have burred " << it.second << " time(s) in time "
                          << current_timestamp << "\n";
            burr_hash_table.clear();
            current_timestamp = strtoll(&buf[1], nullptr, 0);
            continue;
        }
        if (buf[0] == 'b') {
            continue;
        } else {
            std::string signal_alias = std::string((char *) (&buf[1])).substr(0, strlen(buf));
            iter = counters.find(signal_alias);
            iter->second.total_invert_counter++;
            uint64_t time_difference = current_timestamp - iter->second.last_timestamp;
            switch (iter->second.last_level_status) {
                case 'z':
                case '1':iter->second.signal1_time += time_difference;
                    break;
                case '0':iter->second.signal0_time += time_difference;
                    break;
                case 'x':iter->second.signalx_time += time_difference;
                    break;
            }
            if (buf[0] == 'x' || time_difference == 0 || iter->second.last_level_status == 'x')
                iter->second.total_invert_counter--;

            if (time_difference == 0) {
                if (burr_hash_table.find(signal_alias) == burr_hash_table.end())
                    burr_hash_table.insert(std::pair<std::string, uint16_t>(signal_alias, 0));
                burr_hash_table.find(signal_alias)->second++;
                if (buf[0] == 'x' || iter->second.last_level_status == 'x')
                    burr_hash_table.find(signal_alias)->second--;
            }
            iter->second.last_level_status = buf[0];
            iter->second.last_timestamp = current_timestamp;
        }
    }
    fclose(fp);

    for (auto &it : counters) {
        uint64_t time_difference = current_timestamp - it.second.last_timestamp;
        switch (it.second.last_level_status) {
            case 'z':
            case '1':it.second.signal1_time += time_difference;
                break;
            case '0':it.second.signal0_time += time_difference;
                break;
            case 'x':it.second.signalx_time += time_difference;
                break;
        }
    }

    for (auto &i : counters)
        std::cout << i.first << " " << i.second.total_invert_counter << " " << i.second.signal1_time << " "
                  << i.second.signal0_time << " " << i.second.signalx_time << "\n";
    counters.clear();
}
