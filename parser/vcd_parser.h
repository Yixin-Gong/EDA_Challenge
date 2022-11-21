/**************************************************************************//**
  \file     vcd_parser.h
  \brief    VCD file parser header file.
  \author   Yanzhen Zhu, Yixin Gong, Zijie Chou
  \version  V1.0.1
  \date     21. November 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_PARSER_VCD_PARSER_H_
#define EDA_CHALLENGE_PARSER_VCD_PARSER_H_

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include "hopscotch_map.h"

/*!
  \struct VCDHeaderStruct
  \brief  Structs of VCD header information
 */
struct VCDHeaderStruct {
  struct tm vcd_create_time;        ///< Creation time of VCD files
  unsigned int vcd_time_scale;      ///< Time scaling factor of VCD files
  std::string vcd_time_unit;        ///< Time units of VCD files
  std::string vcd_comment_str;      ///< Hash value of VCD file
};

/*!
  \struct VCDSignalStruct
  \brief  Structs for signal properties
 */
struct VCDSignalStruct {
  unsigned int vcd_signal_width;    ///< Signal bit width
  std::string vcd_signal_title;     ///< Full name of the signal
  VCDSignalStruct *next_signal;     ///< Pointer to the next signal property structure
  int declare_width_start = 0;      ///< Initial bits of the vector signal
};

/*!
  \struct VCDSignalStatisticStruct
  \brief  Structs for signal statistics
 */
struct VCDSignalStatisticStruct {
  uint64_t total_invert_counter;    ///< Number of signal flips
  uint64_t signal0_time;            ///< Duration of the 0 level
  uint64_t signal1_time;            ///< Duration of the 1 level
  uint64_t signalx_time;            ///< Duration of the x level
  uint64_t last_timestamp;          ///< Timestamp of the last appearance of the signal
  int8_t last_level_status;         ///< Status of the signal last time
  int8_t final_level_status;        ///< Status of the last two times of the signal
  int32_t total_glitch_counter;     ///< Number of signal glitches
};

/*!
  \struct VCDGlitchStruct
  \brief  Structs for signal glitch
 */
struct VCDGlitchStruct {
  std::string all_module_signal;    ///< Modular path of the signal
  int declare_width_start = 0;      ///< Initial bits of the vector signal
};

/*!  \brief VCD parser class, which contains all the VCD file parsing functions. */
class VCDParser {
 public:

  /*!  \brief Constructor for the VCD parser class, which opens the VCD file and parses the VCD header. */
  explicit VCDParser(const std::string &filename) {
      static char filebuffer[1024 * 1024 * 128] = {0};
      fp_ = fopen64(filename.c_str(), "r");
      std::cout << "\nOpen file: " << filename << "\n";
      parse_vcd_header_();
      setbuffer(fp_, filebuffer, sizeof(filebuffer));
  }

  /*!  \brief The destructor function of the VCD parser class, which clears all the hash tables to free memory. */
  ~VCDParser() {
      fclose(fp_);
      vcd_signal_list_.clear();
      vcd_signal_flip_table_.clear();
      vcd_signal_alias_table_.clear();
      signal_glitch_position_.clear();
  }

  /*!  \brief  Returns the information in the VCD file header.
   *   \return VCD file header information.
   */
  struct VCDHeaderStruct *get_vcd_header() {
      return &vcd_header_struct_;
  }

  /*!  \brief  Returns a statistical hash table of all signals, used only in regression tests.
   *   \return Hash table of all signals.
   */
  tsl::hopscotch_map<std::string, struct VCDSignalStatisticStruct> *get_signal_flip_table() {
      return &vcd_signal_flip_table_;
  }

  void get_vcd_scope();
  void get_vcd_scope(bool enable_gitch);
  void get_vcd_scope(const std::string &module_label);
  void get_vcd_scope(const std::string &module_label, bool enable_gitch);
  void get_vcd_signal_flip_info();
  void get_vcd_signal_flip_info(bool enable_gitch);
  void get_vcd_signal_flip_info(const std::string &module_label);
  void get_vcd_signal_flip_info(const std::string &module_label, bool enable_gitch);
  void get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time);
  void get_vcd_signal_flip_info(uint64_t begin_time, uint64_t end_time, bool enable_gitch);
  void get_vcd_signal_flip_info(const std::string &module_label, uint64_t begin_time, uint64_t end_time);
  void get_vcd_signal_flip_info(const std::string &module_label, uint64_t begin_time,
                                uint64_t end_time, bool enable_gitch);
  void printf_source_csv(const std::string &filepath);
  void printf_glitch_csv(const std::string &filepath);
  VCDSignalStatisticStruct *get_signal_flip_info(const std::string &signal_alias);
  void get_total_flips_in_time_range(uint64_t begin_time, uint64_t end_time,
                                     std::vector<double> *x_value,
                                     std::vector<double> *y_value);

 private:

  /*! \brief File stream structure for VCD files. */
  FILE *fp_;

  /*! \brief File header information for VCD files. */
  struct VCDHeaderStruct vcd_header_struct_{};

  /*! \brief Module hierarchy list for VCD signals. */
  std::list<std::pair<std::string, tsl::hopscotch_map<std::string, struct VCDSignalStruct>>> vcd_signal_list_;

  /*! \brief Hash table for VCD signal statistics. */
  tsl::hopscotch_map<std::string, struct VCDSignalStatisticStruct> vcd_signal_flip_table_;

  /*! \brief Hash table of signals to be counted when counting by module. */
  tsl::hopscotch_map<std::string, int8_t> vcd_signal_alias_table_;

  /*! \brief Hash table for storing the moment of glitch of the signal. */
  tsl::hopscotch_map<std::string, std::list<uint64_t>> signal_glitch_position_;

  /*! \brief Signal name hierarchy information for signals glitch. */
  tsl::hopscotch_map<std::string, struct VCDGlitchStruct> signal_glitch_table_;

  /*! \brief Total duration of the VCD file. */
  uint64_t total_time_{};

  /*! \brief Location of the second timestamp in the file. */
  long second_position_{};

  void parse_vcd_header_();
  static void vcd_statistic_signal_(uint64_t current_timestamp,
                                    struct VCDSignalStatisticStruct *signal,
                                    char current_level_status, const std::string &signal_alias);
  static void vcd_statistic_signal_(uint64_t current_timestamp,
                                    struct VCDSignalStatisticStruct *signal,
                                    tsl::hopscotch_map<std::string, int8_t> *burr_hash_table,
                                    char current_level_status, const std::string &signal_alias);
  void initialize_vcd_signal_flip_table_();
  void initialize_vcd_signal_flip_table_(bool enable_gitch);
  void initialize_vcd_signal_flip_table_(const std::string &module_label);
  void initialize_vcd_signal_flip_table_(const std::string &module_label, bool enable_gitch);
  void vcd_signal_flip_post_processing_(uint64_t timestamp);
  void vcd_signal_flip_post_processing_(uint64_t timestamp, tsl::hopscotch_map<std::string, int8_t> *burr_hash_table);
  void vcd_statistic_glitch_(tsl::hopscotch_map<std::string, int8_t> *burr_hash_table, uint64_t current_timestamp);
};

#endif //EDA_CHALLENGE_PARSER_VCD_PARSER_H_
