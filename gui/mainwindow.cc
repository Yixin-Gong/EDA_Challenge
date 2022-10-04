/**************************************************************************//**
  \file     gui.cc
  \brief    This document describes the interface of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#include "mainwindow.h"

MainWindow::~MainWindow() = default;

void MainWindow::plot_button_clicked() {
    std::valarray<double> x_va(1000), y_va(1000);
    for (unsigned int i = 0; i < 1000; i++)
        x_va[i] = 4 * M_PI * i / 999;
    y_va = sin(x_va);
    auto plot_data = Gtk::manage(new Gtk::PLplot::PlotData2D(x_va, y_va, curve_color_,
                                                             Gtk::PLplot::LineStyle::CONTINUOUS, 2.5));
    plot_ = Gtk::manage(new Gtk::PLplot::Plot2D(*plot_data, x_title_, y_title_, plot_title_));
    canvas_.add_plot(*plot_);
    plot_->hide_legend();
}

void MainWindow::about_button_clicked() {
    about_window_ = new AboutWindow(software_version_);
    about_window_->show();
}

void MainWindow::parse_button_clicked() {
    if (parser_ != nullptr) {
        parser_->get_vcd_value_change_time();
        parser_->get_vcd_value_from_time(350);
    }
}

void MainWindow::open_button_clicked() {
    Gtk::FileChooserDialog dialog("Please choose a file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Open", Gtk::RESPONSE_OK);

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("VCD files");
    filter_text->add_pattern("*.vcd");
    dialog.add_filter(filter_text);

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        vcd_file_name_ = dialog.get_filename();
        std::string label_text = vcd_file_name_.substr(vcd_file_name_.find_last_of('/') + 1, vcd_file_name_.length());
        parser_ = new VCDParser(vcd_file_name_);
        struct VCDHeaderStruct *vcdheader = parser_->get_vcd_header();
        label_text =
            "File: " + label_text + "    " + std::to_string(vcdheader->vcd_time_scale) + vcdheader->vcd_time_unit;
        status_label_->set_label(label_text);
        to_label_->set_label(vcdheader->vcd_time_unit + " to");
        unit_label_->set_label(vcdheader->vcd_time_unit);
        x_title_ = "Time(" + vcdheader->vcd_time_unit + ")";
    }
}
