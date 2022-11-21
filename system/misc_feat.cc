/**************************************************************************//**
  \file     misc_feat.cc
  \brief    This source file implements the function of software system information display.
  \author   Yanzhen Zhu
  \version  V1.0.1
  \date     21. November 2022
 ******************************************************************************/

#include "misc_feat.h"
#include <iostream>
#include <sys/stat.h>
#include "gitver.h"
#include <sched.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <csignal>
#include <unistd.h>

/*!
     \brief Show software system compilation time and version.
*/
void MiscFeat::display_software_info(const std::string &version) {
    std::cout << "Compiler version: " << CXX_COMPILER_VERSION << "\n";
    std::cout << "GTK library version: " << GTK_LIBRARY_VERSION << "\n";
    std::cout << "GTKMM library version: " << GTKMM_LIBRARY_VERSION << "\n";
    std::cout << "Git version: " << GIT_VERSION_HASH << "\n";
    std::cout << "Compile time: " << __TIMESTAMP__ << "\n";
    std::cout << "Software version: " << version << "\n";
}

/*!
     \brief  Determine whether a file exists.
     \return Whether a file exists
*/
bool MiscFeat::file_exists(const std::string &filename) {
    struct stat buffer{};
    return (stat(filename.c_str(), &buffer) == 0);
}

/*!
     \brief  Append information to a file.
     \return Is the write file successful
*/
bool MiscFeat::write_all_bytes_(const char *path, const void *data) noexcept {
    FILE *f = fopen(path, "wb+");
    if (nullptr == f)
        return false;
    fwrite((char *) data, 4, 1, f);
    fflush(f);
    fclose(f);
    return true;
}

/*!
     \brief Set the priority of the process to the highest.
*/
void MiscFeat::set_priority_to_max() noexcept {
    char path_[260];
    snprintf(path_, sizeof(path_), "/proc/%d/oom_adj", getpid());

    char level_[] = "-17";
    write_all_bytes_(path_, level_);

    setpriority(PRIO_PROCESS, 0, -20);

    struct sched_param param_{};
    param_.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(getpid(), SCHED_RR, &param_);
}

/*!
     \brief      Check if the time units are equal and calculate the corresponding timestamp range.
     \param[in]  cli_parser: Command line parser structure pointer
     \param[in]  vcd_parser: VCD parser structure pointer
     \param[out] begin: Converted starting timestamp
     \param[out] end: Converted ending timestamp
*/
void MiscFeat::check_time_range_exists(CLIParser *cli_parser, VCDParser *vcd_parser, uint64_t *begin, uint64_t *end) {
    if ((cli_parser->get_time_range()->end_time_unit != vcd_parser->get_vcd_header()->vcd_time_unit) ||
        (cli_parser->get_time_range()->begin_time_unit != vcd_parser->get_vcd_header()->vcd_time_unit)) {
        std::cout << "The time units you entered do not match the vcd file.\n";
        exit(8);
    }
    *begin = (cli_parser->get_time_range()->begin_time) / (vcd_parser->get_vcd_header()->vcd_time_scale);
    *end = (cli_parser->get_time_range()->end_time) / (vcd_parser->get_vcd_header()->vcd_time_scale);
}
