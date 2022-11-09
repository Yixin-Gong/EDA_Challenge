/**************************************************************************//**
  \file     parser.h
  \brief    VCD parser header file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     25. September 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_PARSER_VCD_PARSER_H_
#define EDA_CHALLENGE_PARSER_VCD_PARSER_H_

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include "hopscotch_map.h"

struct VCDHeaderStruct {
  struct tm vcd_create_time;
  unsigned int vcd_time_scale;
  std::string vcd_time_unit;
  std::string vcd_comment_str;
};

struct VCDSignalStruct {
  unsigned int vcd_signal_width;
  std::string vcd_signal_title;
};

struct VCDSignalStatisticStruct {
  uint64_t total_invert_counter;
  uint64_t signal0_time;
  uint64_t signal1_time;
  uint64_t signalx_time;
  uint64_t last_timestamp;
  int8_t last_level_status;
  int8_t final_level_status;
};

class VCDParser {
 public:
  explicit VCDParser(const std::string &filename) {
      fp_ = fopen64(filename.c_str(), "r");
      std::cout << "\nOpen file: " << filename << "\n";
      parse_vcd_header_();
  }
  ~VCDParser() {
      fclose(fp_);
      vcd_signal_list_.clear();
      vcd_signal_flip_table_.clear();
      vcd_signal_alias_table_.clear();
  }
  struct VCDHeaderStruct *get_vcd_header() {
      return &vcd_header_struct_;
  }
  void get_vcd_scope();
  void get_vcd_scope(const std::string &module_label);
  void get_vcd_signal_flip_info();
  void get_vcd_signal_flip_info(const std::string &module_label);
  void get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time);
  void printf_source_csv(const std::string &filepath);
  VCDSignalStatisticStruct *get_signal_flip_info(const std::string &signal_alias);
  void get_total_flips_in_time_range(uint64_t begin_time,
                                     uint64_t end_time,
                                     std::vector<double> *x_value,
                                     std::vector<double> *y_value);

 private:
  FILE *fp_;
  struct VCDHeaderStruct vcd_header_struct_{};

  std::list<std::pair<std::string, tsl::hopscotch_map<std::string, struct VCDSignalStruct>>> vcd_signal_list_;
  tsl::hopscotch_map<std::string, struct VCDSignalStatisticStruct> vcd_signal_flip_table_;
  tsl::hopscotch_map<std::string, struct VCDSignalStruct> vcd_signal_alias_table_;

  void parse_vcd_header_();
  static void vcd_statistic_signal_(uint64_t current_timestamp,
                                    struct VCDSignalStatisticStruct *signal,
                                    tsl::hopscotch_map<std::string, int8_t> *burr_hash_table,
                                    char current_level_status, const std::string &signal_alias);
  void initialize_vcd_signal_flip_table_();
  void vcd_signal_flip_post_processing_(uint64_t timestamp, tsl::hopscotch_map<std::string, int8_t> *burr_hash_table);
};

#endif //EDA_CHALLENGE_PARSER_VCD_PARSER_H_
