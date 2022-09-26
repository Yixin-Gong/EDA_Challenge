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

struct VCDHeaderStruct {
  struct tm vcd_create_time;
  unsigned int vcd_time_scale;
  std::string vcd_time_unit;
  std::string vcd_comment_str;
};

struct VCDSignalStruct {
  long long dump_vars_line;

};
class VCDParser {
 public:
  explicit VCDParser(const std::string &filename) {
      parse_vcd_header_(filename);
      find_dumpvars_line(filename);
  }
  struct VCDHeaderStruct *get_vcd_header() {
      return &vcd_header_struct_;
  }

  struct VCDSignalStruct *find_dumpvars_line() {
      return &vcd_signal_struct_;

  }
 private:
  struct VCDHeaderStruct vcd_header_struct_{};
  struct VCDSignalStruct vcd_signal_struct_{};

  void parse_vcd_header_(const std::string &filename);
  long long find_dumpvars_line(const std::string &filename);
};

#endif //EDA_CHALLENGE_PARSER_PARSER_H_
