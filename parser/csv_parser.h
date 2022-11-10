//
// Created by ubuntu on 22-11-9.
//

#ifndef EDA_CHALLENGE_PARSER_CSV_PARSER_H_
#define EDA_CHALLENGE_PARSER_CSV_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include "hopscotch_map.h"

struct CSVSignalStatisticStruct {
  int tc, t1, t0, tx, tg;
  float sp;
};
struct VCDSignalStruct {
  unsigned int vcd_signal_width;
  std::string vcd_signal_label;
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
  void printf_csv();

 private:
  FILE *fp_{};
  tsl::hopscotch_map<std::string, struct CSVSignalStatisticStruct> csv_signal_table_;
  tsl::hopscotch_map<std::string, struct VCDSignalStruct> vcd_signal_table_;
  tsl::hopscotch_map<std::string, struct CSVSignalStatisticStruct> merge_csv_vcd_table_;
};

#endif //EDA_CHALLENGE_PARSER_CSV_PARSER_H_
