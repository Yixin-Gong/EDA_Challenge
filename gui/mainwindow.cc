/**************************************************************************//**
  \file     gui.cc
  \brief    This document describes the interface of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#include "mainwindow.h"
#include <utility>

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(Glib::RefPtr<Gtk::Application> app, const std::string &software_version) {
    this->app = std::move(app);
    this->software_version_ = software_version;
    ui_ = Gtk::Builder::create_from_resource("/ui/mainwindow.ui");
    if (ui_) {
        ui_->get_widget<Gtk::Box>("box", box_);
        status_label_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("status_label"));
        plot_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("plot_btn"));
        open_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("open_btn"));
        parse_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("parse_btn"));
        about_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("about_btn"));
        quit_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("quit_btn"));
        plot_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::plot_button_clicked));
        about_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::about_button_clicked));
        parse_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::parse_button_clicked));
        open_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::open_button_clicked));
        quit_btn_->signal_clicked().connect([this]() { this->app->quit(); });
        add(*box_);
    }
    set_resizable(false);
    set_title("Main Window");
    set_default_size(600, 360);
    show_all();
}

void MainWindow::plot_button_clicked() {

}

void MainWindow::about_button_clicked() {
    about_window_ = new AboutWindow(software_version_);
    about_window_->show();
}

void MainWindow::parse_button_clicked() {
    if (parser_ != nullptr) {
        parser_->get_vcd_value_change_time();
        parser_->get_vcd_value_from_time(0);
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
    }
}
