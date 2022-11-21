/**************************************************************************//**
  \file     misc_feat.h
  \brief    This header file contains the software system information display class.
  \author   Yanzhen Zhu
  \version  V1.0.1
  \date     21. November 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_SYSTEM_MISC_FEAT_H_
#define EDA_CHALLENGE_SYSTEM_MISC_FEAT_H_

#include <string>
#include "cli_parser.h"
#include "vcd_parser.h"

/*! \brief Miscellaneous functional class. */
class MiscFeat {
 public:
  static void display_software_info(const std::string &version);
  static bool file_exists(const std::string &filename);
  static void set_priority_to_max() noexcept;
  static void check_time_range_exists(CLIParser *cli_parser, VCDParser *vcd_parser, uint64_t *begin, uint64_t *end);

 private:
  static bool write_all_bytes_(const char *path, const void *data) noexcept;
};

#endif //EDA_CHALLENGE_SYSTEM_MISC_FEAT_H_
