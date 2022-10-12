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
#include "system.h"

CLIParser::CLIParser(std::string filepath,
                     std::string begin_time,
                     std::string end_time,
                     std::string scope,
                     std::string output,
                     bool using_gui) {
    filepath_ = std::move(filepath);
    begin_time_ = std::move(begin_time);
    end_time_ = std::move(end_time);
    scope_ = std::move(scope);
    output_ = std::move(output);
    using_gui_ = using_gui;
    if (filepath_.empty() || (filepath_.find(".vcd") == std::string::npos)) {
        if ((!filepath_.empty()) && (filepath_.find(".vcd") == std::string::npos))
            std::cout << "Please input the VCD file with the .vcd extension\n";
        valid_filename_ = false;
    } else if (!SystemInfo::FileExists(filepath_)) {
        valid_filename_ = false;
        std::cout << "File " << filepath_ << " doesn't exists!\n";
        if (!using_gui_)
            exit(3);
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
    return using_gui_;
}
