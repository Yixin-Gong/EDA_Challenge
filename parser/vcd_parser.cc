/**************************************************************************//**
  \file     parser.cc
  \brief    VCD parser source code file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     25. September 2022
 ******************************************************************************/

#include "vcd_parser.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

static char reading_buffer[1024 * 1024] = {0};

void VCDParser::parse_vcd_header_() {
    unsigned int parse_status = 0;
    static char week[32], month[32];
    static const char kab_month_name[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
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
                read_stream >> vcd_header_struct_.vcd_time_scale >> vcd_header_struct_.vcd_time_unit;
                parse_status = 0;
            }
                break;
        }
        if (read_string.find("$comment") != std::string::npos)
            break;
    }

    strftime(reading_buffer, sizeof(reading_buffer), "%Y-%m-%d %H:%M:%S", &vcd_header_struct_.vcd_create_time);
    std::cout << "File create time: " << reading_buffer << "\n";
    std::cout << "File time scale: " << vcd_header_struct_.vcd_time_scale << vcd_header_struct_.vcd_time_unit << "\n";
    std::cout << "File hash value: " << vcd_header_struct_.vcd_comment_str << "\n\n";
}

void VCDParser::vcd_statistic_signal_(uint64_t current_timestamp,
                                      struct VCDSignalStatisticStruct *signal,
                                      tsl::hopscotch_map<std::string, int8_t> *burr_hash_table,
                                      char current_level_status, const std::string &signal_alias) {
    uint64_t time_difference = current_timestamp - signal->last_timestamp;
    switch (signal->last_level_status) {
        case '1':signal->signal1_time += time_difference;
            break;
        case '0':signal->signal0_time += time_difference;
            break;
        case 'x':signal->signalx_time += time_difference;
            break;
    }
    signal->last_timestamp = current_timestamp;

    if (time_difference != 0) {
        bool case0 =
            signal->last_level_status == 'x' && signal->final_level_status != 'x' && current_level_status != 'x';
        bool case1 =
            signal->last_level_status != 'x' && signal->final_level_status == 'x' && current_level_status == 'x';

        if (signal->last_level_status != signal->final_level_status && !(case0 || case1))
            signal->total_invert_counter++;
        if (signal->last_level_status != 'x')
            signal->final_level_status = signal->last_level_status;
    }
#ifndef NO_STATISTIC_GLITCH
    else {
        if (burr_hash_table->find(signal_alias) == burr_hash_table->end())
            burr_hash_table->insert(std::pair<std::string, int8_t>(signal_alias, {0}));
        signal->total_glitch_counter++;
    }
#endif
    signal->last_level_status = current_level_status;
}

/*!
    \brief Get the initialized signals' information and store them in a hash table, running at time 0.
*/
void VCDParser::initialize_vcd_signal_flip_table_() {
    clock_t startTime = clock();

    /* Seek the fp_ pointer to timestamp 0 */
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        if (read_string == "#0")
            break;
    }

    /* Read VCD file and insert signals. */
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        VCDSignalStatisticStruct cnt{0, 0, 0, 0, 0, 0, 0, 0};

        /* Skip useless lines and define cut-off range. */
        if (reading_buffer[0] == '#' || read_string == "$dumpvars")
            continue;
        if (read_string == "$end")
            break;

        /* If meet b, parse the signal as vectors' standard */
        if (reading_buffer[0] == 'b') {
            /* Separate signal alias and length */
            std::string signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
            unsigned long signal_length = (read_string.substr(1, read_string.find_first_of(' '))).length();

            /* Split the vector signals to scalar signals by its bit, and parse them one by one. */
            for (unsigned long count = signal_length - 1; count > 0; count--) {
                std::string temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");

                /* Insert signal into vcd_signal_flip_table_ and initialize the content */
                if (vcd_signal_flip_table_.find(temp_alias) == vcd_signal_flip_table_.end()) {
                    cnt.last_level_status = reading_buffer[signal_length - count];
                    cnt.final_level_status = '\0';
                    vcd_signal_flip_table_.insert(std::pair<std::string, struct VCDSignalStatisticStruct>
                                                      (temp_alias, cnt));
                } else {
                    /* If signal exists in hash table, update last_level_status. */
                    vcd_signal_flip_table_.find(temp_alias).value().last_level_status =
                        reading_buffer[signal_length - count];
                }
            }
        } else {
            /* If not meet b then parse the signal with scalar standard */
            std::string signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, read_string.length());

            /* Insert signal into vcd_signal_flip_table_ and initialize the content */
            if (vcd_signal_flip_table_.find(signal_alias) == vcd_signal_flip_table_.end()) {
                cnt.last_level_status = reading_buffer[0];
                cnt.final_level_status = '\0';
                vcd_signal_flip_table_.insert(std::pair<std::string, struct VCDSignalStatisticStruct>
                                                  (signal_alias, cnt));
            } else {
                /* If signal exists in hash table, update last_level_status. */
                vcd_signal_flip_table_.find(signal_alias).value().last_level_status = reading_buffer[0];
            }
        }
    }
    std::cout << "Init flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

/*!
     \brief     Count signal flip by post processing pattern
     \param[in] current_timestamp: time in current parsing section.
     \param[in] burr_hash_table: a hash table to store glitches' information.
*/
void VCDParser::vcd_signal_flip_post_processing_(uint64_t current_timestamp,
                                                 tsl::hopscotch_map<std::string, int8_t> *burr_hash_table) {
#ifndef NO_STATISTIC_GLITCH
    /* Print glitches information */
    printf_glitch_csv(burr_hash_table, current_timestamp);
    burr_hash_table->clear();
#endif

    tsl::hopscotch_map<std::string, struct VCDSignalStatisticStruct>::iterator it;

    /* Ergodic the vcd_signal_flip_table to parse all signals */
    for (it = vcd_signal_flip_table_.begin(); it != vcd_signal_flip_table_.end(); it++) {
        uint64_t time_difference = current_timestamp - it->second.last_timestamp;

        /* Count signal's every status' lasting time */
        switch (it->second.last_level_status) {
            case '1':it.value().signal1_time += time_difference;
                break;
            case '0':it.value().signal0_time += time_difference;
                break;
            case 'x':it.value().signalx_time += time_difference;
                break;
        }

        /* Count total change times of signals */
        if ((it->second.last_level_status != it->second.final_level_status)
            && (it->second.final_level_status != 'x'))
            it.value().total_invert_counter++;
        if (it->second.total_invert_counter != 0)
            it.value().total_invert_counter--;
    }
}

/*!  \brief      Get all modules and information of signals and store them in a list.
 *   \param[in]  vcd_signal_alias_table_:A hash table to store information of signals.
 *   \param[in]  vcd_signal_list:A list that stores <string,unordered_map>pairs,key being module.
 */
void VCDParser::get_vcd_scope() {
    clock_t startTime = clock();
    tsl::hopscotch_map<std::string, struct VCDSignalStruct> vcd_signal_table_;
    vcd_signal_table_.clear();
    vcd_signal_list_.clear();
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        /* If read the information of the signal,cut information is stored*/
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            /* Cut the information of signals with a space as a demarcation.
             * And store the information in struct.*/
            struct VCDSignalStruct signal;
            int space_pos = 0;
            std::string width;
            std::string signal_label;
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
                    case 2:width += read_string[pos];
                        break;
                    case 3:signal_label += read_string[pos];
                        break;
                    case 4:signal.vcd_signal_title += read_string[pos];
                        break;
                    default:break;
                }
            }
            vcd_signal_table_.insert(std::pair<std::string, struct VCDSignalStruct>(signal_label,
                                                                                    signal));
        }

            /* If read the information of the module.*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {
            /* Cut the module title.*/
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

            /* Insert the stored signal in the hash table into the list in the corresponding scope_module.
             * Store the scope_module in the list*/
            if (vcd_signal_table_.empty() != 1) {
                vcd_signal_list_.back().second = vcd_signal_table_;
                vcd_signal_list_.emplace_back(scope_module, 0);
            } else
                vcd_signal_list_.emplace_back(scope_module, 0);
            vcd_signal_table_.clear();
        }

            /* If read the upscope*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {
            /* Insert the stored signal in the hash table into the list in the corresponding scope_module.*/
            if (vcd_signal_table_.empty() != 1) {
                vcd_signal_list_.back().second = vcd_signal_table_;
                vcd_signal_table_.clear();
            }

            /* Store the upscope in the list*/
            vcd_signal_list_.emplace_back("upscope", 0);
        }

            /* If read the enddefinitions*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            if (read_string == "$enddefinitions $end") {
                /* Insert the stored signal in the hash table into the list in the corresponding scope_module.*/
                vcd_signal_list_.back().second = vcd_signal_table_;
                break;
            }
        }
    }
    std::cout << "Get scope time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

/*!  \brief      Get specified scope contains information of signals and store them in a hash table.
 *   \param[in]  vcd_signal_alias_table_:A hash table to store information of signals.
 */
void VCDParser::get_vcd_scope(const std::string &module_label) {
    vcd_signal_list_.clear();
    vcd_signal_alias_table_.clear();
    clock_t startTime = clock();
    int module_cnt = 1;
    for (char pos : module_label) {
        if (pos == '/')
            module_cnt++;
    }

    /* Cut module_label to store in vector.*/
    std::vector<std::string> label(module_cnt);
    int module_level = 0;
    for (char pos : module_label) {
        if (pos != '/')
            label[module_level] += pos;
        else
            module_level++;
    }

    /* Start reading the signal when label_pos is equal to module_cnt minus 1.
     * read_label_start is true will start reading signal.
     * skip_store is true will skip store.*/
    int label_pos = 0;
    bool read_label_start = false;
    std::list<std::string> all_module;
    std::string break_module;
    tsl::hopscotch_map<std::string, struct VCDSignalStruct> vcd_signal_table_;

    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        /* If read the upscope and read_label_start is false.*/
        if (!read_label_start && read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {
            all_module.pop_back();
        }
            /* If read the upscope and read_label_start is true.*/
        else if (read_label_start && read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {

            if (vcd_signal_table_.empty() != 1) {
                vcd_signal_list_.back().second = vcd_signal_table_;
                vcd_signal_table_.clear();
            }

            /* When the specified scope is to be upscope*/
            if (all_module.back() == break_module) {
                break;
            }

            vcd_signal_list_.emplace_back("upscope", 0);
            all_module.pop_back();
        }
            /* If read the scope module.*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {
            /* Cut the module title.*/
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
            if (vcd_signal_table_.empty() != 1) {
                vcd_signal_list_.back().second = vcd_signal_table_;
                vcd_signal_table_.clear();
            }
            /* Compare scope_module and module_label in order.
             * Start with the first in module_label.
             * When the comparison is successful, the next one of the module_label will be compared.*/
            if (!read_label_start && label[label_pos] == scope_module && label_pos != module_level) {
                label_pos++;
            }

                /* When all the modules in the module_label are compared, start reading the signal.
                 * Store the current scope_module.*/
            else if (!read_label_start && scope_module == label[label_pos]) {
                break_module = scope_module;
                std::string module;
                for (const auto &it : all_module) {
                    module += it + '/';
                }
                module += scope_module;
                read_label_start = true;
                vcd_signal_list_.emplace_back(module, 0);
            } else if (read_label_start) {
                vcd_signal_list_.emplace_back(scope_module, 0);
            }
            all_module.emplace_back(scope_module);

        }

            /* If start reading the signal and read the information of the signal.*/
        else if (read_label_start && read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            /*Cut the information of signals with a space as a demarcation.
             *And store the information in struct.*/
            auto *signal = new struct VCDSignalStruct;
            int space_pos = 0;
            std::string width;
            std::string signal_label;
            signal->next_signal = nullptr;
            for (int pos = 0; read_string[pos] != 0; pos++) {
                if (read_string[pos] == ' ') {
                    space_pos++;
                    if (space_pos == 5) {
                        signal->vcd_signal_width = std::stoi(width);
                        break;
                    }
                    continue;
                }
                switch (space_pos) {
                    case 2:width += read_string[pos];
                        break;
                    case 3:signal_label += read_string[pos];
                        break;
                    case 4:signal->vcd_signal_title += read_string[pos];
                        break;
                    default:break;
                }
            }

            /* Store information in a hash table.*/
            vcd_signal_alias_table_.insert(std::pair<std::string, int8_t>(signal_label, 0));
            auto current_signal = vcd_signal_table_.find(signal_label);
            if (current_signal == vcd_signal_table_.end()) {
                vcd_signal_table_.insert(std::pair<std::string,
                                                   struct VCDSignalStruct>(signal_label, *signal));
            } else {
                auto *current_signal_struct = &(current_signal.value());
                while (true) {
                    if (current_signal_struct->next_signal == nullptr)
                        break;
                    current_signal_struct = current_signal_struct->next_signal;
                }
                current_signal_struct->next_signal = signal;
            }
        }
    }
    std::cout << "Get scope time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

/*!
 \brief         Total Signal Parse Function
 */
void VCDParser::get_vcd_signal_flip_info() {
    vcd_signal_flip_table_.clear();
    initialize_vcd_signal_flip_table_();
    clock_t startTime = clock();
    static uint64_t current_timestamp = 0, buf_counter = 0;
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        /* Print glitches information */
        if (reading_buffer[0] == '#') {
#ifndef NO_STATISTIC_GLITCH
            /* Print glitches information */
            printf_glitch_csv(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
#endif
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
            continue;
        }

        /* If meet b,parse the signal as vector standard */
        if (reading_buffer[0] == 'b') {
            std::string signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
            unsigned long signal_length = (read_string.substr(1, read_string.find_first_of(' '))).length();

            /* Split the vector signals to scalar signals by its bit ,and parse them one by one */
            for (unsigned long count = signal_length - 1; count > 0; count--) {
                /* Find position of matched signals, if current status is unequal to last status of the signal, parse the signal */
                std::string temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                auto iter = vcd_signal_flip_table_.find(temp_alias);
                if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                    vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                          reading_buffer[signal_length - count], temp_alias);
            }
        }

            /* if not meet b,then parse the signals with scalar standard */
        else {
            /* Find position of matched signals, if current status is unequal to last status of the signal,parse the signal */
            std::string signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, strlen(reading_buffer));
            auto iter = vcd_signal_flip_table_.find(signal_alias);
            vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                  reading_buffer[0], signal_alias);
        }
    }
    vcd_signal_flip_post_processing_(current_timestamp, &burr_hash_table);
    std::cout << "Get flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void VCDParser::get_vcd_signal_flip_info(const std::string &module_label) {
    vcd_signal_flip_table_.clear();
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        if (read_string == "#0")
            break;
    }
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        VCDSignalStatisticStruct cnt{0, 0, 0, 0, 0, 0, 0};
        if (reading_buffer[0] == '#' || read_string == "$dumpvars")
            continue;
        if (read_string == "$end")
            break;
        if (reading_buffer[0] == 'b') {
            std::string signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                unsigned long signal_length = (read_string.substr(1, read_string.find_first_of(' '))).length();
                for (unsigned long count = signal_length - 1; count > 0; count--) {
                    std::string temp_alias;
                    temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                    if (vcd_signal_flip_table_.find(temp_alias) == vcd_signal_flip_table_.end()) {
                        cnt.last_level_status = reading_buffer[signal_length - count];
                        cnt.final_level_status = 'x';
                        vcd_signal_flip_table_.insert(std::pair<std::string,
                                                                struct VCDSignalStatisticStruct>(temp_alias,
                                                                                                 cnt));
                    } else {
                        auto iter = vcd_signal_flip_table_.find(temp_alias);
                        iter.value().last_level_status = reading_buffer[signal_length - count];
                        iter.value().final_level_status = 'x';
                    }
                }
            }
        } else {
            std::string signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, read_string.length());
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                if (vcd_signal_flip_table_.find(signal_alias) == vcd_signal_flip_table_.end()) {
                    cnt.last_level_status = reading_buffer[0];
                    cnt.final_level_status = 'x';
                    vcd_signal_flip_table_.insert(std::pair<std::string, struct VCDSignalStatisticStruct>(signal_alias,
                                                                                                          cnt));
                } else {
                    auto iter = vcd_signal_flip_table_.find(signal_alias);
                    iter.value().last_level_status = reading_buffer[0];
                    iter.value().final_level_status = 'x';
                }
            }
        }
    }

    static uint64_t current_timestamp = 0;
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;

    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        if (reading_buffer[0] == '#') {
#ifndef NO_STATISTIC_GLITCH
            /* Print glitches information */
            printf_glitch_csv(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
#endif
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
            continue;
        }
        if (reading_buffer[0] == 'b') {
            std::string signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                unsigned long signal_length = (read_string.substr(1, read_string.find_first_of(' '))).length();
                for (unsigned long count = signal_length - 1; count > 0; count--) {
                    std::string
                        temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                    auto iter = vcd_signal_flip_table_.find(temp_alias);
                    if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                        vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                              reading_buffer[signal_length - count], temp_alias);
                }
            }
        } else {
            std::string signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, strlen(reading_buffer));
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                auto iter = vcd_signal_flip_table_.find(signal_alias);
                vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                      reading_buffer[0], signal_alias);
            }
        }
    }
    vcd_signal_flip_post_processing_(current_timestamp, &burr_hash_table);
}
/*!
    \brief          Parse Signal by Time Range Function
    \param[in]      begin_time:begin time of the parsing process
    \param[in]      end_time:end time of the parsing process
 */
void VCDParser::get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time) {
    vcd_signal_flip_table_.clear();
    /* If begin time is 0, start to parse. */
    int8_t status = (begin_time == 0) ? 1 : 0;
    initialize_vcd_signal_flip_table_();
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    static uint64_t current_timestamp = 0, last_timestamp = 0;

    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        /* Update last_time_stamp and current stamp */
        if (reading_buffer[0] == '#') {
            last_timestamp = current_timestamp;
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);

            /* Transfer status according to conditions. */
            if (current_timestamp == begin_time)
                status = 1;
            if (current_timestamp >= end_time && status != 2)
                status = 2;
            else if (status == 2) {
                status = 3;
                current_timestamp = last_timestamp;
            }
#ifndef NO_STATISTIC_GLITCH
            /* Print glitches information */
            printf_glitch_csv(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
#endif
            continue;
        }
        /*Set a finite stats machine */
        switch (status) {
            /* Update every signal appeared */
            case 0:
                if (reading_buffer[0] == 'b') {
                    std::string
                        signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
                    unsigned long signal_length = (read_string.substr(1, read_string.find_first_of(' '))).length();
                    for (unsigned long count = signal_length - 1; count > 0; count--) {
                        std::string
                            temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                        auto iter = vcd_signal_flip_table_.find(temp_alias);
                        iter.value().final_level_status = 'x';
                        iter.value().last_timestamp = current_timestamp;
                        iter.value().last_level_status = reading_buffer[signal_length - count];
                    }
                } else {
                    std::string
                        signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, strlen(reading_buffer));
                    auto iter = vcd_signal_flip_table_.find(signal_alias);
                    iter.value().final_level_status = 'x';
                    iter.value().last_timestamp = current_timestamp;
                    iter.value().last_level_status = reading_buffer[0];
                }
                break;
                /* Parse all signals*/
            case 2:
            case 1:
                if (reading_buffer[0] == 'b') {
                    std::string
                        signal_alias = read_string.substr(read_string.find_last_of(' ') + 1, read_string.length());
                    unsigned long signal_length = (read_string.substr(1, read_string.find_first_of(' '))).length();
                    for (unsigned long count = signal_length - 1; count > 0; count--) {
                        std::string
                            temp_alias = signal_alias + std::string("[") + std::to_string(count - 1) + std::string("]");
                        auto iter = vcd_signal_flip_table_.find(temp_alias);
                        if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                            vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                                  reading_buffer[signal_length - count], temp_alias);
                    }
                } else {
                    std::string
                        signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, strlen(reading_buffer));
                    auto iter = vcd_signal_flip_table_.find(signal_alias);
                    vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                          reading_buffer[0], signal_alias);
                }
                break;
            default:break;
        }
        if (status == 3)
            break;
    }
    vcd_signal_flip_post_processing_(current_timestamp, &burr_hash_table);
}

/*!  \brief      Output the stored and counted results to file.
 *   \param[in]  vcd_signal_list:A list that stores <string,unordered_map>pairs,key being module.
 */
void VCDParser::printf_source_csv(const std::string &filepath) {
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::trunc);

    /* Output the information in the list.
     * all_module is used to store modules that have not exited at each layer.*/
    std::list<std::string> all_module;
    for (auto &iter : vcd_signal_list_) {

        /* If read upscope delete a module*/
        if (iter.first == "upscope") {
            all_module.pop_back();
            continue;
        }

        /* Merging of modules at all levels.*/
        std::string All_module;
        All_module.clear();
        for (auto &module : all_module)
            All_module += module + "/";

        all_module.emplace_back(iter.first);

        /* Output modules and signals to file.*/
        if (iter.second.empty() != 1) {
            for (auto &it : iter.second) {
                struct VCDSignalStatisticStruct signal{};
                char sp_buffer[16] = {0};

                /* 1-bit wide and multi-bit wide outputs.*/
                if (it.second.vcd_signal_width == 1) {
                    if (vcd_signal_flip_table_.find(it.first) == vcd_signal_flip_table_.end())
                        memset(&signal, 0x00, sizeof(struct VCDSignalStatisticStruct));
                    else
                        signal = vcd_signal_flip_table_.find(it.first)->second;
                    sprintf(sp_buffer, "%.5lf", (double) signal.signal1_time
                        / (double) (signal.signal0_time + signal.signal1_time + signal.signalx_time));

                    auto *current_signal = &(it.second);
                    while (true) {
                        file << All_module << iter.first << "." << current_signal->vcd_signal_title
                             << "    tc = " << signal.total_invert_counter
                             << "    t1 = " << signal.signal1_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "    t0 = " << signal.signal0_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit
                             << "    tx = " << signal.signalx_time * vcd_header_struct_.vcd_time_scale
                             << vcd_header_struct_.vcd_time_unit << "    sp = " << sp_buffer << std::endl;
                        if (current_signal->next_signal == nullptr)
                            break;
                        current_signal = current_signal->next_signal;
                    }

                } else {
                    for (int wid_pos = 0; wid_pos < it.second.vcd_signal_width; wid_pos++) {
                        std::string
                            temp_alias = it.first + std::string("[") + std::to_string(wid_pos) + std::string("]");
                        if (vcd_signal_flip_table_.find(temp_alias) == vcd_signal_flip_table_.end())
                            memset(&signal, 0x00, sizeof(struct VCDSignalStatisticStruct));
                        else
                            signal = vcd_signal_flip_table_.find(temp_alias)->second;
                        sprintf(sp_buffer, "%.5lf", (double) signal.signal1_time
                            / (double) (signal.signal0_time + signal.signal1_time + signal.signalx_time));

                        auto *current_signal = &(it.second);
                        while (true) {
                            file << All_module << iter.first << "." << current_signal->vcd_signal_title << "["
                                 << wid_pos
                                 << "]    tc = " << signal.total_invert_counter
                                 << "    t1 = " << signal.signal1_time * vcd_header_struct_.vcd_time_scale
                                 << vcd_header_struct_.vcd_time_unit
                                 << "    t0 = " << signal.signal0_time * vcd_header_struct_.vcd_time_scale
                                 << vcd_header_struct_.vcd_time_unit
                                 << "    tx = " << signal.signalx_time * vcd_header_struct_.vcd_time_scale
                                 << vcd_header_struct_.vcd_time_unit << "    sp = " << sp_buffer << std::endl;
                            if (current_signal->next_signal == nullptr)
                                break;
                            current_signal = current_signal->next_signal;
                        }
                    }
                }
            }
        }
    }
    file.close();
}

VCDSignalStatisticStruct *VCDParser::get_signal_flip_info(const std::string &signal_alias) {
    VCDSignalStatisticStruct *signal = &(vcd_signal_flip_table_.find(signal_alias).value());
    return signal;
}

void VCDParser::get_total_flips_in_time_range(uint64_t begin_time,
                                              uint64_t end_time,
                                              std::vector<double> *x_value,
                                              std::vector<double> *y_value) {
    uint64_t current_timestamp = 0, signal_counter = 0;
    fseeko64(fp_, 1, SEEK_SET);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        if (reading_buffer[0] == '#') {
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
            if (current_timestamp >= begin_time)
                break;
        }
    }
    (*x_value).push_back((double) current_timestamp - 1);
    (*y_value).push_back(0);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        if (reading_buffer[0] == '#') {
            (*x_value).push_back((double) current_timestamp);
            (*y_value).push_back((double) signal_counter);
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
            signal_counter = 0;
            if (current_timestamp >= end_time)
                break;
            continue;
        } else if (reading_buffer[0] == '0' || reading_buffer[0] == '1' || reading_buffer[0] == 'b')
            signal_counter++;
    }
}

std::string VCDParser::get_vcd_signal_(const std::string &label) {
    std::list<std::string> all_module;
    std::string signal_title;
    for (auto &it : vcd_signal_list_) {
        if (it.second.find(label) != it.second.end()) {
            std::string module;
            for (auto &iter : all_module) {
                module += iter + "/";
            }
            module += it.first;
            signal_title = module + "." + it.second.find(label).value().vcd_signal_title;
            break;
        }
        all_module.emplace_back(it.first);
    }
    return signal_title;
}

void VCDParser::printf_glitch_csv(tsl::hopscotch_map<std::string, int8_t> *burr_hash_table,
                                  uint64_t current_timestamp) {
    FILE *fp = fopen("./glitch.csv", "a");
    for (const auto &glitch : *burr_hash_table) {
        auto signal_pos = signal_glitch_position_.find(glitch.first);
        if (signal_pos != signal_glitch_position_.end()) {
            fseeko64(fp, (long) signal_pos->second, SEEK_CUR);
            fprintf(fp, " %lu%s",
                    current_timestamp * vcd_header_struct_.vcd_time_scale,
                    vcd_header_struct_.vcd_time_unit.c_str());
            signal_pos.value() = ftello64(fp);
        } else {
            fseeko64(fp, 0, SEEK_END);
            fprintf(fp, "\n%s %lu%s", get_vcd_signal_(glitch.first).c_str(),
                    current_timestamp * vcd_header_struct_.vcd_time_scale,
                    vcd_header_struct_.vcd_time_unit.c_str());
            signal_glitch_position_.insert(std::pair<std::string, uint64_t>(glitch.first, ftello64(fp)));
        }
    }
    fclose(fp);
}

