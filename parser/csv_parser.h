//
// Created by ubuntu on 22-11-9.
//

#ifndef EDA_CHALLENGE_PARSER_CSV_PARSER_H_
#define EDA_CHALLENGE_PARSER_CSV_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include "hopscotch_map.h"

struct CSVSignalStatisticStruct {
  int tc, t1, t0, tx, tg;
  float sp;
};
struct CSV_VCDSignalStruct {
  unsigned int vcd_signal_width;
  std::string vcd_signal_label;
  int declare_width_start = 0;
};

class CSVParser {
 public:
  explicit CSVParser(const std::string &filename) {
      fp_ = fopen64(filename.c_str(), "r");
      std::cout << "\nOpen file: " << filename << "\n";
  }
  ~CSVParser() {
      fclose(fp_);
  }
  void parse_csv();
  void get_vcd_scope(const std::string &vcd_filename);
  void csv_find_vcd();
  struct CSVSignalStatisticStruct *find_signal(const std::string &signal_label);
 private:
  FILE *fp_{};
  tsl::hopscotch_map<std::string, struct CSVSignalStatisticStruct> csv_signal_table_;
  tsl::hopscotch_map<std::string, struct CSV_VCDSignalStruct> csv_vcd_signal_table_;
  std::unordered_map<std::string, struct CSVSignalStatisticStruct> merge_csv_vcd_table_;
};

#endif //EDA_CHALLENGE_PARSER_CSV_PARSER_H_
