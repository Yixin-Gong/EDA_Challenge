/**************************************************************************//**
  \file     parser.h
  \brief    VCD parser header file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     25. September 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_PARSER_PARSER_H_
#define EDA_CHALLENGE_PARSER_PARSER_H_

#include <string>

class VCDParser {
 public:
  explicit VCDParser(const std::string &filename) {
      parse_vcd_header_(filename);
  }
  struct tm *get_vcd_create_time() {
      return &vcd_create_time_;
  }
  int get_vcd_time_scale() {
      return vcd_time_scale_;
  }
  std::string get_vcd_comment_str() {
      return vcd_comment_str_;
  }

 private:
  struct tm vcd_create_time_{};
  unsigned int vcd_time_scale_{};
  std::string vcd_comment_str_{};
  void parse_vcd_header_(const std::string &filename);
};

#endif //EDA_CHALLENGE_PARSER_PARSER_H_
