/**************************************************************************//**
  \file     system.cc
  \brief    This header file contains the software system information display class.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_SYSTEM_SYSTEM_H_
#define EDA_CHALLENGE_SYSTEM_SYSTEM_H_

#include <string>
#include "cli_parser.h"
#include "vcd_parser.h"

class SystemInfo {
 public:
  static void DisplayCompileInfo(const std::string &version);
  static bool FileExists(const std::string &filename);
  static void set_priority_to_max() noexcept;
  static void check_time_range_exists(CLIParser *cli_parser, VCDParser *vcd_parser, uint64_t *begin, uint64_t *end);

 private:
  static bool write_all_bytes_(const char *path, const void *data) noexcept;
};

#endif //EDA_CHALLENGE_SYSTEM_SYSTEM_H_
