/**************************************************************************//**
  \file     vcd_parser_glitch.cc
  \brief    VCD parser with glitch statistic source code file.
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
#include <unistd.h>

static char reading_buffer[1024 * 1024] = {0};

void VCDParser::vcd_statistic_signal_(uint64_t current_timestamp,
                                      struct VCDSignalStatisticStruct *signal,
                                      tsl::hopscotch_map<std::string, int8_t> *burr_hash_table,
                                      char current_level_status, const std::string &signal_alias) {
    uint64_t time_difference = current_timestamp - signal->last_timestamp;
    bool last_level_unequal_x = true;
    switch (signal->last_level_status) {
        case '1':signal->signal1_time += time_difference;
            break;
        case '0':signal->signal0_time += time_difference;
            break;
        case 'x':signal->signalx_time += time_difference;
            last_level_unequal_x = false;
            break;
    }
    signal->last_timestamp = current_timestamp;

    if (time_difference != 0) {
        if (signal->last_level_status != signal->final_level_status
            && (last_level_unequal_x || current_level_status == 'x'))
            signal->total_invert_counter++;
        if (last_level_unequal_x)
            signal->final_level_status = signal->last_level_status;
    } else {
        if (burr_hash_table->find(signal_alias) == burr_hash_table->end())
            burr_hash_table->insert(std::pair<std::string, int8_t>(signal_alias, {0}));
        signal->total_glitch_counter++;
    }
    signal->last_level_status = current_level_status;
}

/*!
    \brief Get the initialized signals' information and store them in a hash table, running at time 0.
*/
void VCDParser::initialize_vcd_signal_flip_table_(bool enable_gitch) {
    clock_t startTime = clock();
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    /* Seek the fp_ pointer to timestamp 0 */
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        if (read_string == "#0")
            break;
    }

    /* Read VCD file and insert signals. */
    int flag = 0;
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        VCDSignalStatisticStruct cnt{0, 0, 0, 0, 0, 0, 0, 0};

        /* Skip useless lines and define cut-off range. */

        if ((flag == 0 && reading_buffer[0] == '#') || read_string == "$dumpvars")
            flag = 1;
        if (read_string == "$end")
            flag = 2;
        if (flag == 2 && reading_buffer[0] != '#')
            second_position = ftello64(fp_);
        if (flag == 2 && reading_buffer[0] == '#')
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
                    if (cnt.last_level_status != reading_buffer[signal_length - count]) {
                        vcd_signal_flip_table_.find(temp_alias).value().total_glitch_counter++;
                        if (burr_hash_table.find(temp_alias) == burr_hash_table.end())
                            burr_hash_table.insert(std::pair<std::string, int8_t>(temp_alias, 0));
                    }
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
                if (cnt.last_level_status != reading_buffer[0]) {
                    vcd_signal_flip_table_.find(signal_alias).value().total_glitch_counter++;
                    if (burr_hash_table.find(signal_alias) == burr_hash_table.end())
                        burr_hash_table.insert(std::pair<std::string, int8_t>(signal_alias, 0));
                }
                vcd_signal_flip_table_.find(signal_alias).value().last_level_status = reading_buffer[0];
            }
        }
    }
    vcd_statistic_glitch_(&burr_hash_table, 0);
    burr_hash_table.clear();
    std::cout << "Init flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void VCDParser::initialize_vcd_signal_flip_table_(const std::string &module_label, bool enable_gitch) {
    clock_t startTime = clock();
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        if (read_string == "#0")
            break;
    }
    int flag = 0;
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;
        VCDSignalStatisticStruct cnt{0, 0, 0, 0, 0, 0, 0};
        /* Skip useless lines and define cut-off range. */
        if ((flag == 0 && reading_buffer[0] == '#') || read_string == "$dumpvars")
            flag = 1;
        if (read_string == "$end")
            flag = 2;
        if (flag == 2 && reading_buffer[0] != '#')
            second_position = ftello64(fp_);
        if (flag == 2 && reading_buffer[0] == '#')
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
                        cnt.final_level_status = '\0';
                        vcd_signal_flip_table_.insert(std::pair<std::string,
                                                                struct VCDSignalStatisticStruct>(temp_alias, cnt));
                    } else {
                        if (cnt.last_level_status != reading_buffer[signal_length - count]) {
                            vcd_signal_flip_table_.find(temp_alias).value().total_glitch_counter++;
                            if (burr_hash_table.find(temp_alias) == burr_hash_table.end())
                                burr_hash_table.insert(std::pair<std::string, int8_t>(temp_alias, 0));
                        }
                        vcd_signal_flip_table_.find(temp_alias).value().last_level_status =
                            reading_buffer[signal_length - count];
                    }
                }
            }
        } else {
            std::string signal_alias = std::string((char *) (&reading_buffer[1])).substr(0, read_string.length());
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                if (vcd_signal_flip_table_.find(signal_alias) == vcd_signal_flip_table_.end()) {
                    cnt.last_level_status = reading_buffer[0];
                    cnt.final_level_status = '\0';
                    vcd_signal_flip_table_.insert(std::pair<std::string, struct VCDSignalStatisticStruct>(signal_alias,
                                                                                                          cnt));
                } else {
                    if (cnt.last_level_status != reading_buffer[0]) {
                        vcd_signal_flip_table_.find(signal_alias).value().total_glitch_counter++;
                        if (burr_hash_table.find(signal_alias) == burr_hash_table.end())
                            burr_hash_table.insert(std::pair<std::string, int8_t>(signal_alias, 0));
                    }
                    vcd_signal_flip_table_.find(signal_alias).value().last_level_status = reading_buffer[0];
                }
            }
        }
    }
    vcd_statistic_glitch_(&burr_hash_table, 0);
    burr_hash_table.clear();
    std::cout << "Init flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void VCDParser::vcd_statistic_glitch_(tsl::hopscotch_map<std::string, int8_t> *burr_hash_table,
                                      uint64_t current_timestamp) {
    for (const auto &glitch : *burr_hash_table) {
        auto signal_pos = signal_glitch_position_.find(glitch.first);
        if (signal_pos != signal_glitch_position_.end())
            signal_pos.value().emplace_back(current_timestamp);
        else {
            std::list<uint64_t> signal_list;
            signal_list.emplace_back(current_timestamp);
            signal_glitch_position_.insert(std::pair<std::string, std::list<uint64_t>>(glitch.first, signal_list));
        }
    }
}

/*!
     \brief     Count signal flip by post processing pattern
     \param[in] current_timestamp: time in current parsing section.
     \param[in] burr_hash_table: a hash table to store glitches' information.
*/
void VCDParser::vcd_signal_flip_post_processing_(uint64_t current_timestamp,
                                                 tsl::hopscotch_map<std::string, int8_t> *burr_hash_table) {
    /* Print glitches information */
    vcd_statistic_glitch_(burr_hash_table, current_timestamp);
    burr_hash_table->clear();

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
        if (it->second.last_level_status != it->second.final_level_status && it->second.last_level_status != 'x')
            it.value().total_invert_counter++;
        if (it->second.total_invert_counter != 0)
            it.value().total_invert_counter--;
    }
    total_time = current_timestamp;
}

/*!  \brief      Get all modules and information of signals and store them in a list.
 *   \param[in]  vcd_signal_alias_table_:A hash table to store information of signals.
 *   \param[in]  vcd_signal_list:A list that stores <string,unordered_map>pairs,key being module.
 */
void VCDParser::get_vcd_scope(bool enable_gitch) {
    clock_t startTime = clock();
    tsl::hopscotch_map<std::string, struct VCDSignalStruct> vcd_signal_table_;
    vcd_signal_table_.clear();
    vcd_signal_list_.clear();
    std::list<std::string> Module;
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        /* If read the information of the signal,cut information is stored*/
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            /* Cut the information of signals with a space as a demarcation.
             * And store the information in struct.*/
            auto *signal = new struct VCDSignalStruct;
            int space_pos = 0;
            std::string width, signal_label;
            signal->next_signal = nullptr;
            std::string judgment;
            for (int pos = 0; read_string[pos] != 0; pos++) {
                if (read_string[pos] == ' ') {
                    space_pos++;
                    if (space_pos == 6) {
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
                    case 5:judgment += read_string[pos];
                        break;
                    default:break;
                }
            }
            signal->vcd_signal_width = std::stoi(width);

            if (judgment != "$end") {
                if (judgment.find(':') != std::string::npos) {
                    std::string number;
                    number = judgment.substr(judgment.find(':') + 1, judgment.length() - judgment.find(':') - 2);
                    if (number != "0") {
                        signal->declare_width_start = std::stoi(number);
                    }
                }

            }

            if (signal_glitch_table_.find(signal_label) == signal_glitch_table_.end()) {
                struct VCDGlitchStruct glitch;
                std::string module_signal;
                for (auto &iter : Module)
                    module_signal += iter + '/';
                module_signal.pop_back();
                module_signal += '.' + signal->vcd_signal_title;
                glitch.all_module_signal = module_signal;
                if (signal->declare_width_start != 0)
                    glitch.declare_width_start = signal->declare_width_start;
                signal_glitch_table_.insert(std::pair<std::string, struct VCDGlitchStruct>(signal_label,
                                                                                           glitch));
            }

            /* Store information in a hash table.*/
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

            Module.emplace_back(scope_module);
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
            Module.pop_back();
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
void VCDParser::get_vcd_scope(const std::string &module_label, bool enable_gitch) {
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
            std::string judgment;
            for (int pos = 0; read_string[pos] != 0; pos++) {
                if (read_string[pos] == ' ') {
                    space_pos++;
                    if (space_pos == 6) {
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
                    case 5:judgment += read_string[pos];
                        break;
                    default:break;
                }
            }
            signal->vcd_signal_width = std::stoi(width);

            if (judgment != "$end") {
                if (judgment.find(':') != std::string::npos) {
                    std::string number;
                    number = judgment.substr(judgment.find(':') + 1, judgment.length() - judgment.find(':') - 2);
                    if (number != "0") {
                        signal->declare_width_start = std::stoi(number);
                    }
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
void VCDParser::get_vcd_signal_flip_info(bool enable_gitch) {
    vcd_signal_flip_table_.clear();
    signal_glitch_position_.clear();
    initialize_vcd_signal_flip_table_(enable_gitch);
    clock_t startTime = clock();
    static uint64_t current_timestamp = 0, buf_counter = 0;
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    fseeko64(fp_, second_position, SEEK_SET);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        size_t last_word_position = strlen(reading_buffer) - 1;

        /* Print glitches information */
        if (reading_buffer[0] == '#') {
            /* Print glitches information */
            vcd_statistic_glitch_(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
            continue;
        }

        /* If meet b,parse the signal as vector standard */
        if (reading_buffer[0] == 'b') {
            reading_buffer[last_word_position] = '[';
            size_t first_pos = std::string(reading_buffer).find_first_of(' ');
            size_t signal_length = first_pos;
            std::string signal_alias = std::string(&reading_buffer[first_pos + 1]);

            /* Split the vector signals to scalar signals by its bit ,and parse them one by one */
            for (unsigned long count = signal_length - 1; count > 0; count--) {
                /* Find position of matched signals, if current status is unequal to last status of the signal, parse the signal */
                std::string temp_alias = signal_alias + std::to_string(count - 1) + std::string("]");
                auto iter = vcd_signal_flip_table_.find(temp_alias);
                if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                    vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                          reading_buffer[signal_length - count], temp_alias);
            }
        }
            /* if not meet b,then parse the signals with scalar standard */
        else {
            /* Find position of matched signals, if current status is unequal to last status of the signal,parse the signal */
            reading_buffer[last_word_position] = '\0';
            std::string signal_alias = std::string((char *) (&reading_buffer[1]));
            auto iter = vcd_signal_flip_table_.find(signal_alias);
            vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                  reading_buffer[0], signal_alias);
        }
    }
    vcd_signal_flip_post_processing_(current_timestamp, &burr_hash_table);
    std::cout << "Get flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void VCDParser::get_vcd_signal_flip_info(const std::string &module_label, bool enable_gitch) {
    vcd_signal_flip_table_.clear();
    signal_glitch_position_.clear();
    initialize_vcd_signal_flip_table_(module_label, enable_gitch);
    clock_t startTime = clock();
    static uint64_t current_timestamp = 0;
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    fseeko64(fp_, second_position, SEEK_SET);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        size_t last_word_position = strlen(reading_buffer) - 1;

        if (reading_buffer[0] == '#') {
            /* Print glitches information */
            vcd_statistic_glitch_(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);
            continue;
        }
        if (reading_buffer[0] == 'b') {
            reading_buffer[last_word_position] = '[';
            size_t first_pos = std::string(reading_buffer).find_first_of(' ');
            std::string signal_alias = std::string(&reading_buffer[first_pos + 1]);
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                size_t signal_length = first_pos;
                for (unsigned long count = signal_length - 1; count > 0; count--) {
                    std::string temp_alias = signal_alias + std::to_string(count - 1) + std::string("]");
                    auto iter = vcd_signal_flip_table_.find(temp_alias);
                    if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                        vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                              reading_buffer[signal_length - count], temp_alias);
                }
            }
        } else {
            reading_buffer[last_word_position] = '\0';
            std::string signal_alias = std::string((char *) (&reading_buffer[1]));
            if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                auto iter = vcd_signal_flip_table_.find(signal_alias);
                vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                      reading_buffer[0], signal_alias);
            }
        }
    }
    vcd_signal_flip_post_processing_(current_timestamp, &burr_hash_table);
    std::cout << "Get flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

/*!
    \brief          Parse Signal by Time Range Function
    \param[in]      begin_time:begin time of the parsing process
    \param[in]      end_time:end time of the parsing process
 */
void VCDParser::get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time, bool enable_gitch) {
    vcd_signal_flip_table_.clear();
    signal_glitch_position_.clear();
    /* If begin time is 0, start to parse. */
    int8_t status = (begin_time == 0) ? 1 : 0;
    initialize_vcd_signal_flip_table_(enable_gitch);
    clock_t startTime = clock();
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    static uint64_t current_timestamp = 0;
    fseeko64(fp_, second_position, SEEK_SET);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        size_t last_word_position = strlen(reading_buffer) - 1;

        /* Update last_time_stamp and current stamp */
        if (reading_buffer[0] == '#') {
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);

            /* Transfer status according to conditions. */
            if (current_timestamp >= begin_time && status == 0) {
                status = 1;
                for (tsl::hopscotch_map<std::string, struct VCDSignalStatisticStruct>::iterator
                         item = vcd_signal_flip_table_.begin();
                     item != vcd_signal_flip_table_.end(); item++)
                    item.value().last_timestamp = begin_time;
            }
            if (current_timestamp >= end_time && status != 2)
                status = 2;
            else if (status == 2) {
                status = 3;
                current_timestamp = end_time;
            }
            /* Print glitches information */
            vcd_statistic_glitch_(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
            continue;
        }
        /*Set a finite stats machine */
        switch (status) {
            /* Update every signal appeared */
            case 0:
                if (reading_buffer[0] == 'b') {
                    reading_buffer[last_word_position] = '[';
                    size_t first_pos = std::string(reading_buffer).find_first_of(' ');
                    size_t signal_length = first_pos;
                    std::string signal_alias = std::string(&reading_buffer[first_pos + 1]);
                    for (unsigned long count = signal_length - 1; count > 0; count--) {
                        std::string temp_alias = signal_alias + std::to_string(count - 1) + std::string("]");
                        auto iter = vcd_signal_flip_table_.find(temp_alias);
                        iter.value().last_level_status = reading_buffer[signal_length - count];
                    }
                } else {
                    reading_buffer[last_word_position] = '\0';
                    std::string signal_alias = std::string((char *) (&reading_buffer[1]));
                    auto iter = vcd_signal_flip_table_.find(signal_alias);
                    iter.value().last_level_status = reading_buffer[0];
                }
                break;
                /* Parse all signals*/
            case 2:
            case 1:
                if (reading_buffer[0] == 'b') {
                    reading_buffer[last_word_position] = '[';
                    size_t first_pos = std::string(reading_buffer).find_first_of(' ');
                    size_t signal_length = first_pos;
                    std::string signal_alias = std::string(&reading_buffer[first_pos + 1]);
                    for (unsigned long count = signal_length - 1; count > 0; count--) {
                        std::string temp_alias = signal_alias + std::to_string(count - 1) + std::string("]");
                        auto iter = vcd_signal_flip_table_.find(temp_alias);
                        if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                            vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                                  reading_buffer[signal_length - count], temp_alias);
                    }
                } else {
                    reading_buffer[last_word_position] = '\0';
                    std::string signal_alias = std::string((char *) (&reading_buffer[1]));
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
    total_time = end_time - begin_time;
    std::cout << "Get flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void VCDParser::get_vcd_signal_flip_info(const std::string &module_label, uint64_t begin_time,
                                         uint64_t end_time, bool enable_gitch) {
    vcd_signal_flip_table_.clear();
    signal_glitch_position_.clear();
    /* If begin time is 0, start to parse. */
    int8_t status = (begin_time == 0) ? 1 : 0;
    initialize_vcd_signal_flip_table_(module_label, enable_gitch);
    clock_t startTime = clock();
    tsl::hopscotch_map<std::string, int8_t> burr_hash_table;
    static uint64_t current_timestamp = 0;
    fseeko64(fp_, second_position, SEEK_SET);
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        size_t last_word_position = strlen(reading_buffer) - 1;

        /* Update last_time_stamp and current stamp */
        if (reading_buffer[0] == '#') {
            current_timestamp = strtoll(&reading_buffer[1], nullptr, 0);

            /* Transfer status according to conditions. */
            if (current_timestamp >= begin_time && status == 0) {
                status = 1;
                for (tsl::hopscotch_map<std::string, struct VCDSignalStatisticStruct>::iterator
                         item = vcd_signal_flip_table_.begin();
                     item != vcd_signal_flip_table_.end(); item++)
                    item.value().last_timestamp = begin_time;
            }
            if (current_timestamp >= end_time && status != 2)
                status = 2;
            else if (status == 2) {
                status = 3;
                current_timestamp = end_time;
            }
            /* Print glitches information */
            vcd_statistic_glitch_(&burr_hash_table, current_timestamp);
            burr_hash_table.clear();
            continue;
        }
        /*Set a finite stats machine */
        switch (status) {
            /* Update every signal appeared */
            case 0:
                if (reading_buffer[0] == 'b') {
                    reading_buffer[last_word_position] = '[';
                    size_t first_pos = std::string(reading_buffer).find_first_of(' ');
                    size_t signal_length = first_pos;
                    std::string signal_alias = std::string(&reading_buffer[first_pos + 1]);
                    if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                        for (unsigned long count = signal_length - 1; count > 0; count--) {
                            std::string temp_alias = signal_alias + std::to_string(count - 1) + std::string("]");
                            auto iter = vcd_signal_flip_table_.find(temp_alias);
                            iter.value().last_level_status = reading_buffer[signal_length - count];
                        }
                    }
                } else {
                    reading_buffer[last_word_position] = '\0';
                    std::string signal_alias = std::string((char *) (&reading_buffer[1]));
                    if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                        auto iter = vcd_signal_flip_table_.find(signal_alias);
                        iter.value().last_level_status = reading_buffer[0];
                    }
                }
                break;
                /* Parse all signals*/
            case 2:
            case 1:
                if (reading_buffer[0] == 'b') {
                    reading_buffer[last_word_position] = '[';
                    size_t first_pos = std::string(reading_buffer).find_first_of(' ');
                    size_t signal_length = first_pos;
                    std::string signal_alias = std::string(&reading_buffer[first_pos + 1]);
                    if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                        for (unsigned long count = signal_length - 1; count > 0; count--) {
                            std::string temp_alias = signal_alias + std::to_string(count - 1) + std::string("]");
                            auto iter = vcd_signal_flip_table_.find(temp_alias);
                            if (reading_buffer[signal_length - count] != iter->second.last_level_status)
                                vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                                      reading_buffer[signal_length - count], temp_alias);
                        }
                    }
                } else {
                    reading_buffer[last_word_position] = '\0';
                    std::string signal_alias = std::string((char *) (&reading_buffer[1]));
                    if (vcd_signal_alias_table_.find(signal_alias) != vcd_signal_alias_table_.end()) {
                        auto iter = vcd_signal_flip_table_.find(signal_alias);
                        vcd_statistic_signal_(current_timestamp, &(iter.value()), &burr_hash_table,
                                              reading_buffer[0], signal_alias);
                    }
                }
                break;
            default:break;
        }
        if (status == 3)
            break;
    }
    vcd_signal_flip_post_processing_(current_timestamp, &burr_hash_table);
    total_time = end_time - begin_time;
    std::cout << "Get flip time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void VCDParser::printf_glitch_csv(const std::string &filepath) {
    clock_t startTime = clock();
    std::ofstream output_file;
    output_file.open(filepath, std::ios::out | std::ios::trunc);
    for (const auto &glitch : signal_glitch_position_) {
        std::string label, signal_bit;
        bool read_bit = false;
        for (auto &pos : glitch.first) {
            if (pos == '[' && glitch.first.length() - label.length() >= 3)
                read_bit = true;
            if (!read_bit)
                label += pos;
            else
                signal_bit += pos;
        }

        if (signal_glitch_table_.find(label) != signal_glitch_table_.end()) {

            if (signal_glitch_table_.find(label).value().declare_width_start != 0) {
                int wid_pos = std::stoi(signal_bit.substr(1, signal_bit.find(']') - 1))
                    + signal_glitch_table_.find(label).value().declare_width_start;
                signal_bit.clear();
                signal_bit = std::string("[") + std::to_string(wid_pos) + std::string("]");
            }

            auto signal_list = glitch.second;
            std::string signal_string = signal_glitch_table_.find(label).value().all_module_signal;
            std::string signal_glitch_string;
            if (!read_bit)
                signal_glitch_string = signal_string;
            else
                signal_glitch_string = signal_string + signal_bit;
            signal_glitch_string += " ";
            for (auto &it : signal_list) {
                signal_glitch_string +=
                    std::to_string(it * vcd_header_struct_.vcd_time_scale) + vcd_header_struct_.vcd_time_unit + " ";
            }
            output_file << signal_glitch_string << std::endl;
        }
    }
    output_file.close();
    std::cout << "Print glitch time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}
