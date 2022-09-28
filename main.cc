/**************************************************************************//**
  \file     main.cc
  \brief    This source file is the program entry file.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     20. September 2022
 ******************************************************************************/

#ifdef IS_NOT_RUNNING_GOOGLE_TEST
#include <iostream>
#include "gui.h"
#include "parser.h"
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
#ifdef IS_NOT_RUNNING_GOOGLE_TEST
    try {
        TCLAP::CmdLine cmd("This software is VCD file parsing and statistics software, optimized for large files."
                           " You can visit https://github.com/ZhuYanzhen1/EDA_Challenge to get more information about this software.",
                           ' ', "1.0.1");

        /* Define value arguments and add it to the command line */
        TCLAP::ValueArg<std::string> filename_arg("f", "file", "The vcd file to be parsed",
                                                  false, "", "string");
        TCLAP::SwitchArg using_gui_switch("g", "gui", "Whether to display the gui interface",
                                          cmd, false);
        cmd.add(filename_arg);

        /* Parse the argv array */
        cmd.parse(argc, argv);

        /* Get the value parsed by each arg */
        std::string filepath = filename_arg.getValue();
        bool using_gui_flag = using_gui_switch.getValue();

        if (using_gui_flag) {
            std::cout << "Starting software with GUI ...\n";
            argc = 1;
            auto app = Gtk::Application::create(argc, argv, "eda.challenge");
            MainWindow main_window(app);
            return app->run(main_window);
        } else {
            if (filepath.empty()) {
                std::cout << "Please input VCD file path with -f <file path>\n";
                return 1;
            } else if (filepath.find(".vcd") == std::string::npos) {
                std::cout << "Please input the VCD file with the .vcd extension\n";
                return 2;
            }
            std::cout << "No gui with file: " << filepath << "\n";
            VCDParser parser(filepath);
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
