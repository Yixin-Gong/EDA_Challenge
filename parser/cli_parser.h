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

struct CLITimeStruct {
  uint64_t begin_time;
  std::string begin_time_unit;
  uint64_t end_time;
  std::string end_time_unit;
};

class CLIParser {
 public:
  CLIParser(std::string filepath,
            const std::string &begin_time,
            const std::string &end_time,
            std::string scope,
            std::string output,
            bool using_gui);
  bool using_gui();
  CLITimeStruct *get_time_range();
  bool valid_file() const {
      return valid_filename_;
  }
  bool valid_time() const {
      return valid_time_rage_;
  }
  bool valid_scope() const {
      return valid_scope_;
  }
  std::string get_filename() {
      return filepath_;
  }
  std::string get_output() {
      return output_;
  }
  std::string get_scope() {
      if (scope_[0] == '/')
          scope_.erase(0, 1);
      return scope_;
  }

 private:
  std::string filepath_;
  std::string begin_time_;
  std::string end_time_;
  std::string scope_;
  std::string output_;
  bool using_gui_;
  bool valid_filename_;
  bool valid_time_rage_;
  bool valid_scope_;
  CLITimeStruct cli_time_struct_;
};

#endif //EDA_CHALLENGE_PARSER_CLI_PARSER_H_
