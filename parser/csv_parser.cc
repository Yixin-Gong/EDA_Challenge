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

        signal_title = read_str.substr(read_str.find('.') + 1, read_str.find("tc=") - read_str.find('.') - 1);
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

            /* Store information in a hash table.*/
            csv_vcd_signal_table_.insert(std::pair<std::string, struct CSV_VCDSignalStruct>(signal_title, signal));
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
            std::cout << label << '\n';
        } else {
            label = iter.first;
        }

        if (csv_vcd_signal_table_.find(label) == csv_vcd_signal_table_.end()) {
            continue;
        }

        bool read_width = false;
        for (auto &pos : iter.first) {
            if (pos == '[')
                read_width = true;
            if (read_width)
                width += pos;
        }

        std::string signal_label;
        signal_label = csv_vcd_signal_table_.find(label).value().vcd_signal_label + width;
        merge_csv_vcd_table_.insert(std::pair<std::string, struct CSVSignalStatisticStruct>(signal_label, iter.second));
    }
    for (auto &it : merge_csv_vcd_table_) {
        std::cout << it.first << " tc=" << it.second.tc << " t1=" << it.second.t1 << " t0=" << it.second.t0 << " tx="
                  << it.second.tx << " sp=" << it.second.sp << " tg=" << it.second.tg << '\n';
    }
}
