/**************************************************************************//**
  \file     gui.cc
  \brief    This document describes the interface of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#include "mainwindow.h"
#include <ctime>

MainWindow::~MainWindow() = default;
MainWindow::MainWindow(Glib::RefPtr<Gtk::Application> app, const std::string &software_version) {
    this->app_ = std::move(app);
    this->software_version_ = software_version;
    initialize_window_();
}

MainWindow::MainWindow(Glib::RefPtr<Gtk::Application> app,
                       const std::string &software_version,
                       const std::string &filename) {
    this->app_ = std::move(app);
    this->software_version_ = software_version;
    vcd_file_name_ = filename;
    initialize_window_();
    parse_file_header_();
}

void MainWindow::plot_button_clicked() {
    std::valarray<double> x_va(1000), y_va(1000);
    for (unsigned int i = 0; i < 1000; i++)
        x_va[i] = 4 * M_PI * i / 999;
    y_va = sin(x_va);
    auto plot_data = Gtk::manage(new Gtk::PLplot::PlotData2D(x_va, y_va, curve_color_,
                                                             Gtk::PLplot::LineStyle::CONTINUOUS, 2.5));
    plot_ = Gtk::manage(new Gtk::PLplot::Plot2D(*plot_data, x_title_, y_title_, plot_title_));
    canvas_.remove_plot(0);
    canvas_.add_plot(*plot_);
    plot_->hide_legend();
}

void MainWindow::about_button_clicked() {
    about_window_ = new AboutWindow(software_version_);
    about_window_->show();
}

void MainWindow::parse_button_clicked() {
    clock_t startTime, endTime;
    startTime = clock();
    if (parser_ != nullptr) {
        parser_->get_vcd_value_change_time();
        parser_->get_vcd_value_from_time_range(748000, 0);
        parser_->get_vcd_scope();
        parser_->get_vcd_signal_flip_info(0, 0);
        std::cout << "Signal ! is " << parser_->get_vcd_signal("!")->vcd_signal_title << "\n";
    }
    endTime = clock();
    std::cout << "\nRunning time is:" << (double) (endTime - startTime) / CLOCKS_PER_SEC << "s\n";
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
        delete parser_;
        vcd_file_name_ = dialog.get_filename();
        parse_file_header_();
    }
}

void MainWindow::initialize_window_() {
    ui_ = Gtk::Builder::create_from_resource("/ui/mainwindow.ui");
    Gdk::Geometry geometry;
    ui_->get_widget<Gtk::Box>("box", box_);
    ui_->get_widget<Gtk::Grid>("subgrid", grid_);
    geometry.min_aspect = geometry.max_aspect = double(600) / double(400);
    set_geometry_hints(*this, geometry, Gdk::HINT_ASPECT);
    canvas_.set_hexpand(true);
    canvas_.set_vexpand(true);

    std::valarray<double> x_va(1000), y_va(1000);
    for (unsigned int i = 0; i < 1000; i++)
        x_va[i] = 4 * M_PI * i / 999;
    y_va = sin(x_va);
    auto plot_data = Gtk::manage(new Gtk::PLplot::PlotData2D(x_va, y_va, curve_color_,
                                                             Gtk::PLplot::LineStyle::CONTINUOUS, 2.5));
    plot_ = Gtk::manage(new Gtk::PLplot::Plot2D(*plot_data, x_title_, y_title_, plot_title_));
    canvas_.add_plot(*plot_);
    plot_->hide_legend();
    grid_->attach(canvas_, 0, 1, 3, 1);

    status_label_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("status_label"));
    unit_label_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("unit_label"));
    to_label_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("to_label"));

    plot_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("plot_btn"));
    open_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("open_btn"));
    parse_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("parse_btn"));
    about_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("about_btn"));
    quit_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("quit_btn"));
    plot_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::plot_button_clicked));
    about_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::about_button_clicked));
    parse_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::parse_button_clicked));
    open_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::open_button_clicked));
    quit_btn_->signal_clicked().connect([this]() { this->app_->quit(); });

    add(*box_);
    set_resizable(false);
    set_title("Main Window");
    set_default_size(600, 400);
    show_all();
}

void MainWindow::parse_file_header_() {
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
