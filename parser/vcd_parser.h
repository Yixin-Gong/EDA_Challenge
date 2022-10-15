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
#include <unordered_map>

struct VCDHeaderStruct {
  struct tm vcd_create_time;
  unsigned int vcd_time_scale;
  std::string vcd_time_unit;
  std::string vcd_comment_str;
};

struct VCDSignalStruct {
  std::string vcd_signal_type;
  unsigned int vcd_signal_width;
  std::string vcd_signal_label;
  std::string vcd_signal_title;
};

class VCDParser {
 public:
  VCDParser();
  explicit VCDParser(const std::string &filename);
  ~VCDParser();
  struct VCDHeaderStruct *get_vcd_header() {
      return &vcd_header_struct_;
  }
  struct VCDSignalStruct *get_vcd_signal(const std::string &label);
  void get_vcd_scope();
  void get_vcd_value_change_time();
  void get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time);
  void get_vcd_value_from_time_range(uint64_t begin_time, uint64_t end_time);
  void printf_source_csv(const std::string &filepath);

 private:
  struct VCDTimeStampStruct {
    uint64_t timestamp;
    uint64_t location;
  };
  struct VCDTimeStampBufferStruct {
    struct VCDTimeStampStruct *first_element;
    struct VCDTimeStampBufferStruct *next_buffer;
  };
  struct VCDSignalStatisticStruct {
    uint64_t total_invert_counter;
    uint64_t signal0_time;
    uint64_t signal1_time;
    uint64_t signalx_time;
    uint64_t last_timestamp;
    int8_t last_level_status;
  };
  std::string vcd_filename_{};
  struct VCDHeaderStruct vcd_header_struct_{};
  struct VCDTimeStampBufferStruct time_stamp_first_buffer_{};
  const uint32_t ktime_stamp_buffer_size_ = 1024;

  std::list<std::pair<std::string, std::unordered_map<std::string, struct VCDSignalStruct>>> vcd_signal_list_;
  std::unordered_map<std::string, struct VCDSignalStatisticStruct> vcd_signal_flip_table_;
  std::unordered_map<std::string, struct VCDSignalStruct> vcd_signal_alias_table_;
  void parse_vcd_header_(const std::string &filename);
  void vcd_delete_time_stamp_buffer_();
};

#endif //EDA_CHALLENGE_PARSER_VCD_PARSER_H_
