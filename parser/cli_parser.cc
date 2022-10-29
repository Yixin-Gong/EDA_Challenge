/**************************************************************************//**
  \file     cli_parser.cc
  \brief    Command line input validity check source code.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#include "cli_parser.h"
#include <iostream>
#include <utility>
#include <fstream>
#include "system.h"

CLIParser::CLIParser(std::string filepath,
                     std::string begin_time,
                     std::string end_time,
                     std::string scope,
                     std::string output,
                     bool using_gui) {
    filepath_ = std::move(filepath);
    begin_time_ = strtoull(begin_time.c_str(), nullptr, 0);
    end_time_ = strtoull(end_time.c_str(), nullptr, 0);
    scope_ = std::move(scope);
    output_ = std::move(output);
    using_gui_ = using_gui;
    valid_filename_ = true;
    if (filepath_.empty() || (filepath_.find(".vcd") == std::string::npos)) {
        if ((!filepath_.empty()) && (filepath_.find(".vcd") == std::string::npos))
            std::cout << "Please input the VCD file with the .vcd extension\n";
        valid_filename_ = false;
    } else if (!SystemInfo::FileExists(filepath_)) {
        valid_filename_ = false;
        std::cout << "File " << filepath_ << " doesn't exists!\n";
        if (!using_gui_)
            exit(3);
    } else {
        std::ifstream file;
        file.open(filepath_, std::ios_base::in);
        if (!file.is_open()) {
            std::cout << "\nCannot open file " << filepath_ << "\n";
            valid_filename_ = false;
            if (!using_gui_)
                exit(4);
        }
        file.close();
    }
    if (begin_time.empty() ^ end_time.empty()) {
        std::cout << "You can't just enter a start time or an end time.\n";
        exit(5);
    }
}

bool CLIParser::using_gui() {
    if (using_gui_)
        std::cout << "Starting software with GUI ...\n";
    else {
        if (filepath_.empty()) {
            std::cout << "Please input VCD file path with -f <file path>\n";
            exit(1);
        } else if (filepath_.find(".vcd") == std::string::npos) {
            std::cout << "Please input the VCD file with the .vcd extension\n";
            exit(2);
        }
        std::cout << "No gui with file: " << filepath_ << "\n";
        std::cout << "Output file path: " << output_ << "\n";
    }
    std::cout << "From time " << begin_time_ << " to time " << end_time_ << "\n";
    return using_gui_;
}
