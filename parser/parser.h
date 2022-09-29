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
#include <vector>

struct VCDHeaderStruct {
  struct tm vcd_create_time;
  unsigned int vcd_time_scale;
  std::string vcd_time_unit;
  std::string vcd_comment_str;
};

class VCDParser {
 public:
  VCDParser();
  explicit VCDParser(const std::string &filename);
  struct VCDHeaderStruct *get_vcd_header() {
      return &vcd_header_struct_;
  }
  static void get_vcd_value_change_time(const std::string &filename);
  
 private:
  struct VCDHeaderStruct vcd_header_struct_{};
  void parse_vcd_header_(const std::string &filename);
};

#endif //EDA_CHALLENGE_PARSER_PARSER_H_
