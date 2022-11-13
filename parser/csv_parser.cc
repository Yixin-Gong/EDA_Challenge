#include "csv_parser.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

static char reading_buffer[1024 * 1024] = {0};

void CSVParser::parse_csv() {

    while (fgets(reading_buffer, sizeof(reading_buffer), fp_) != nullptr) {
        reading_buffer[strlen(reading_buffer) - 1] = '\0';
        std::string read_string = reading_buffer;

        std::string read_str;
        std::string signal_title;

        for (auto &pos : read_string) {
            if (pos != ' ' && pos != '\t')
                read_str += pos;
        }

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
        signal.tc = std::stoi(tc);
        signal.t1 = std::stoi(t1);
        signal.t0 = std::stoi(t0);
        signal.tx = std::stoi(tx);
        signal.sp = atof(sp.c_str());
        signal.tg = std::stoi(tg);
        csv_signal_table_.insert(std::pair<std::string, struct CSVSignalStatisticStruct>(signal_title, signal));
    }
}
void CSVParser::get_vcd_scope(const std::string &vcd_filename) {
    clock_t startTime = clock();

    fclose(fp_);
    fp_ = fopen64(vcd_filename.c_str(), "r");
    std::cout << "\nOpen file: " << vcd_filename << "\n";

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
                    case 3:signal.vcd_signal_label += read_string[pos];
                        break;
                    case 4:signal_title += read_string[pos];
                        break;
                    default:break;
                }
            }
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
            scope_module += "/";
            all_module.emplace_back(scope_module);
        } else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'u') {
            all_module.pop_back();
        }
            /* If read the enddefinitions*/
        else if (read_string.c_str()[0] == '$' && read_string.c_str()[1] == 'e') {
            if (read_string == "$enddefinitions $end") {
                break;
            }
        }
    }
    std::cout << "Get scope time: " << (double) (clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}
void CSVParser::csv_find_vcd() {
    merge_csv_vcd_table_.clear();
    for (auto &iter : csv_signal_table_) {

        std::string label, width;
        if ((iter.first.length() - iter.first.rfind('[')) <= 4) {
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

        } else {
            label = iter.first;
        }

        if (csv_vcd_signal_table_.find(label) == csv_vcd_signal_table_.end()) {
            continue;
        }

        std::string signal_label;
        if (width.empty())
            signal_label = csv_vcd_signal_table_.find(label).value().vcd_signal_label;
        else
            signal_label = csv_vcd_signal_table_.find(label).value().vcd_signal_label + width;
        merge_csv_vcd_table_.insert(std::pair<std::string, struct CSVSignalStatisticStruct>(signal_label, iter.second));
    }
}

struct CSVSignalStatisticStruct *CSVParser::find_signal(const std::string &signal_label) {
    if (merge_csv_vcd_table_.find(signal_label) == merge_csv_vcd_table_.end())
        return nullptr;
    return &merge_csv_vcd_table_.find(signal_label)->second;
}

