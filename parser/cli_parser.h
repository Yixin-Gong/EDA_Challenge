/**************************************************************************//**
  \file     cli_parser.cc
  \brief    Command line input validity check headers.
  \author   Yanzhen Zhu
  \version  V1.0.1
  \date     21. November 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_PARSER_CLI_PARSER_H_
#define EDA_CHALLENGE_PARSER_CLI_PARSER_H_

#include <string>

/*!
  \struct CLITimeStruct
  \brief  Structs for command line input time
 */
struct CLITimeStruct {
  uint64_t begin_time;          ///< Start time
  std::string begin_time_unit;  ///< Start time unit
  uint64_t end_time;            ///< End time
  std::string end_time_unit;    ///< End time unit
};

/*!  \brief CLI parser class, which contains all the command line parsing functions. */
class CLIParser {
 public:
  CLIParser(std::string filepath,
            const std::string &begin_time,
            const std::string &end_time,
            std::string scope,
            std::string output,
            bool using_gui,
            bool using_glitch,
            bool using_multi);
  bool using_gui();
  CLITimeStruct *get_time_range();

  /*!  \brief  Determine whether the file path is available.
   *   \return Is the file path available.
   */
  bool valid_file() const {
      return valid_filename_;
  }

  /*!  \brief  Determine whether the time range is available.
   *   \return Is the time range available.
   */
  bool valid_time() const {
      return valid_time_rage_;
  }

  /*!  \brief  Determine if the scope information is entered.
   *   \return Is the scope information entered.
   */
  bool valid_scope() const {
      return valid_scope_;
  }

  /*!  \brief  Get the path of the input VCD file.
   *   \return VCD file path.
   */
  std::string get_filename() {
      return filepath_;
  }

  /*!  \brief  Get the output folder path.
   *   \return Output folder path.
   */
  std::string get_output() {
      return output_;
  }

  /*!  \brief  Get the name of the specified scope.
   *   \return Name of the specified scope.
   */
  std::string get_scope() {
      if (scope_[0] == '/')
          scope_.erase(0, 1);
      return scope_;
  }

  /*!  \brief  Determine if glitch information is output.
   *   \return Is the glitch information been output.
   */
  bool using_glitch() const {
      return using_glitch_;
  }

  /*!  \brief  Determining whether to use multithreading.
   *   \return Is the multithreading been used.
   */
  bool using_multithread() const {
      return using_multi_;
  }

 private:
  /*! \brief The original information of the vcd file path. */
  std::string filepath_;

  /*! \brief The original information of the begin time. */
  std::string begin_time_;

  /*! \brief The original information of the end time. */
  std::string end_time_;

  /*! \brief The original information of the scope name. */
  std::string scope_;

  /*! \brief The original information of the output folder path. */
  std::string output_;

  /*! \brief Whether to open the GUI interface. */
  bool using_gui_;

  /*! \brief Whether to output glitch information. */
  bool using_glitch_;

  /*! \brief Whether to use multithreading. */
  bool using_multi_;

  /*! \brief Is the entered file name available. */
  bool valid_filename_;

  /*! \brief Is the entered time range available. */
  bool valid_time_rage_;

  /*! \brief Is the entered scope name available. */
  bool valid_scope_;

  /*! \brief Parsed input time range information. */
  CLITimeStruct cli_time_struct_;
};

#endif //EDA_CHALLENGE_PARSER_CLI_PARSER_H_
