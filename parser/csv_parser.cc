/**************************************************************************//**
  \file     csv_parser.cc
  \brief    CSV file parser source code.
  \author   Zijie Chou
  \version  V1.0.1
  \date     21. November 2022
 ******************************************************************************/

#include "csv_parser.h"
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

/*! \brief Line buffer for reading lines from a file. */
static char reading_buffer[1024 * 1024] = {0};

/*!
     \brief Parse csv file and stores information in a hash table
 */
void CSVParser::parse_csv() {

    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        std::string read_str;
        std::string signal_title;

        /* Remove spaces and tabs.*/
        for (auto &pos : read_string) {
            if (pos != ' ' && pos != '\t')
                read_str += pos;
        }

        /* Cut signal information and save.*/
        signal_title = read_str.substr(0, read_str.find("tc="));
        struct CSVSignalStatisticStruct signal{};
        std::string tc, t1, t0, tx, sp, tg;
        tc = read_str.substr(read_str.find("tc=") + 3, read_str.find("t1=") - read_str.find("tc=") - 3);
        t1 = read_str.substr(read_str.find("t1=") + 3, read_str.find("t0=") - read_str.find("t1=") - 5);
        t0 = read_str.substr(read_str.find("t0=") + 3, read_str.find("tx=") - read_str.find("t0=") - 5);
        tx = read_str.substr(read_str.find("tx=") + 3, read_str.find("sp=") - read_str.find("tx=") - 5);
        sp = read_str.substr(read_str.find("sp=") + 3,
                             read_str.find("tg=") - read_str.find("sp=") - 3);
        tg = read_str.substr(read_str.find("tg=") + 3, read_str.length() - read_str.find("tg="));
        signal.tc = std::stoull(tc);
        signal.t1 = std::stoull(t1);
        signal.t0 = std::stoull(t0);
        signal.tx = std::stoull(tx);
        signal.sp = atof(sp.c_str());
        signal.tg = std::stoull(tg);
        csv_signal_table_.insert(std::pair<std::string, struct CSVSignalStatisticStruct>(signal_title, signal));
    }
}

/*!
     \brief     Get all modules and information of signals and store them in a hash table.
     \param[in] vcd_filename: The vcd file that needs to be parsed.
 */
void CSVParser::get_vcd_scope(const std::string &vcd_filename) {
    fclose(fp_);
    fp_ = fopen64(vcd_filename.c_str(), "r");

    csv_vcd_signal_table_.clear();
    std::list<std::string> all_module;
    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        /* If read the information of the signal,cut information is stored*/
        if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'v') {
            /* Cut the information of signals with a space as a demarcation.
             * And store the information in struct.*/
            struct CSV_VCDSignalStruct signal;
            int space_pos = 0;
            std::string width;
            std::string signal_title;
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
                    case 3:signal.vcd_signal_label += read_string[pos];
                        break;
                    case 4:signal_title += read_string[pos];
                        break;
                    case 5:judgment += read_string[pos];
                        break;
                    default:break;
                }
            }
            signal.vcd_signal_width = std::stoi(width);

            /* Judgment start width.
             * The default is 0, if it is not 0, it will be stored.*/
            if (judgment != "$end") {
                if (judgment.find(':') != std::string::npos) {
                    std::string number;
                    number = judgment.substr(judgment.find(':') + 1, judgment.length() - judgment.find(':') - 2);
                    if (number != "0") {
                        signal.declare_width_start = std::stoi(number);
                    }
                }
            }

            /* Store information in csv_vcd_signal_table_.
             * Module and signal title will be stored.*/
            std::string module_signal_title;
            std::string All_module;
            All_module.clear();
            for (auto &module : all_module)
                All_module += module;
            All_module = All_module.substr(0, All_module.length() - 1);
            All_module += ".";
            module_signal_title = All_module + signal_title;

            /* Store information in a hash table.*/
            csv_vcd_signal_table_.insert(std::pair<std::string, struct CSV_VCDSignalStruct>(module_signal_title,
                                                                                            signal));
        }
            /* If read the scope.*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 's') {
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
            scope_module += "/";
            all_module.emplace_back(scope_module);
        }
            /* If read the upscope.*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {
            all_module.pop_back();
        }
            /* If read the enddefinitions.*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            if (read_string == "$enddefinitions $end") {
                break;
            }
        }
    }
}

/*!
     \brief Use the stored label in the csv file to find the corresponding signal name in the vcd file and merge it
 */
void CSVParser::csv_find_vcd() {
    merge_csv_vcd_table_.clear();
    for (auto &iter : csv_signal_table_) {

        /* Determine whether it is a vector signal*/
        std::string label, width;
        if ((iter.first.length() - iter.first.rfind(']')) == 1 &&
            (iter.first.rfind('[') != std::string::npos) &&
            iter.first.length() >= 4) {
            bool read_label = true;
            for (auto &pos : iter.first) {
                if (pos == '[')
                    read_label = false;
                if (read_label)
                    label += pos;
            }

            bool read_width = false;
            for (auto &pos : iter.first) {
                if (pos == '[')
                    read_width = true;
                if (read_width)
                    width += pos;
            }

        } else
            label = iter.first;

        if (csv_vcd_signal_table_.find(label) == csv_vcd_signal_table_.end())
            continue;

        /* Determine what the starting bit width is.*/
        if (csv_vcd_signal_table_.find(label).value().declare_width_start != 0) {
            int width_pos = std::stoi(width.substr(1, width.find(']') - 1));
            width_pos = width_pos - csv_vcd_signal_table_.find(label).value().declare_width_start;
            width.clear();
            width = '[' + std::to_string(width_pos) + ']';
        }
        std::string signal_label;
        if (width.empty())
            signal_label = csv_vcd_signal_table_.find(label).value().vcd_signal_label;
        else
            signal_label = csv_vcd_signal_table_.find(label).value().vcd_signal_label + width;
        merge_csv_vcd_table_.insert(std::pair<std::string, struct CSVSignalStatisticStruct>(signal_label, iter.second));
    }
}

/*!
     \brief     Returns the signal statistics for the specified signal struct, used only in regression tests.
     \param[in] signal_label: Input module name.
     \return    A signal struct pointer.
 */
struct CSVSignalStatisticStruct *CSVParser::find_signal(const std::string &signal_label) {
    if (merge_csv_vcd_table_.find(signal_label) == merge_csv_vcd_table_.end())
        return nullptr;
    return &(merge_csv_vcd_table_.find(signal_label)->second);
}
