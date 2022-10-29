/**************************************************************************//**
  \file     main.cc
  \brief    This source file is the program entry file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#if (defined(IS_NOT_RUNNING_GOOGLE_TEST) || defined(IS_NOT_RUNNING_GUI))
#include <iostream>
#include <unistd.h>
#ifndef IS_NOT_RUNNING_GUI
#include "mainwindow.h"
#endif
#include "vcd_parser.h"
#include "cli_parser.h"
#include "tclap/CmdLine.h"
#else
#include "gtest/gtest.h"
#endif

/*!
    \brief      Program entry
    \param[in]  argc: Number of command-line arguments sent to the main function at program runtime
    \param[in]  argv: Store an array of pointers to string parameters
    \return     Program execution results
*/
int main(int argc, char **argv) {
#if (defined(IS_NOT_RUNNING_GOOGLE_TEST) || defined(IS_NOT_RUNNING_GUI))
    const std::string software_version = "1.0.1";
    char buffer[512];
    std::string software_path = "./";
    if (getcwd(buffer, 512) != nullptr)
        software_path = buffer;
    try {
        TCLAP::CmdLine cmd("This software is VCD file parsing and statistics software, optimized for large files."
                           " You can visit https://github.com/ZhuYanzhen1/EDA_Challenge to get more information about this software.",
                           ' ', software_version);

        /* Define value arguments and add it to the command line */
        TCLAP::ValueArg<std::string> filename_arg("f", "file", "The vcd file to be parsed",
                                                  false, "", "string");
        TCLAP::ValueArg<std::string> begin_time_arg("b", "begin",
                                                    "Specify the start time of the signal to be counted",
                                                    false, "", "string");
        TCLAP::ValueArg<std::string> end_time_arg("e", "end",
                                                  "Specify the termination time of the signal to be counted",
                                                  false, "", "string");
        TCLAP::ValueArg<std::string> scope_arg("s", "scope",
                                               "Specify the module where the signal to be counted is located",
                                               false, "", "string");
        TCLAP::ValueArg<std::string> output_arg("o", "output", "Location of the output csv file",
                                                false, software_path + "/output.csv", "string");
        TCLAP::SwitchArg using_gui_switch("g", "gui", "Whether to display the gui interface",
                                          cmd, false);
        cmd.add(filename_arg);
        cmd.add(begin_time_arg);
        cmd.add(end_time_arg);
        cmd.add(scope_arg);
        cmd.add(output_arg);

        /* Parse the argv array */
        cmd.parse(argc, argv);

        /* Get the value parsed by each arg */
        CLIParser cli_parser(filename_arg.getValue(),
                             begin_time_arg.getValue(),
                             end_time_arg.getValue(),
                             scope_arg.getValue(),
                             output_arg.getValue(),
                             using_gui_switch.getValue());

        if (cli_parser.using_gui()) {
#ifndef IS_NOT_RUNNING_GUI
            argc = 1;
            auto app = Gtk::Application::create(argc, argv, "eda.challenge");
            auto *main_window = new MainWindow(app, software_version, &cli_parser);
            return app->run(*main_window);
#else
            std::cout << "You cannot use gui in this version.\n";
#endif
        } else {
            clock_t startTime, endTime;
            startTime = clock();
            uint64_t timestamp = 200;
            VCDParser parser(cli_parser.get_filename());
            parser.get_vcd_value_change_time();
            parser.get_vcd_scope();
            parser.get_vcd_signal_flip_info();
            parser.printf_source_csv(cli_parser.get_output());
            endTime = clock();
            std::cout << "\nRunning time is:" << (double) (endTime - startTime) / CLOCKS_PER_SEC << "s\n";
        }
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return 0;
#else
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#endif
}
