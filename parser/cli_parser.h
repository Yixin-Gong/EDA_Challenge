/**************************************************************************//**
  \file     cli_parser.cc
  \brief    Command line input validity check headers.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_PARSER_CLI_PARSER_H_
#define EDA_CHALLENGE_PARSER_CLI_PARSER_H_

#include <string>

class CLIParser {
 public:
  CLIParser(std::string filepath,
            std::string begin_time,
            std::string end_time,
            std::string scope,
            std::string output,
            bool using_gui);
  bool using_gui();
  bool valid_file() const {
      return valid_filename_;
  }
  std::string get_filename() {
      return filepath_;
  }

 private:
  std::string filepath_;
  std::string begin_time_;
  std::string end_time_;
  std::string scope_;
  std::string output_;
  bool using_gui_;
  bool valid_filename_;
};

#endif //EDA_CHALLENGE_PARSER_CLI_PARSER_H_
