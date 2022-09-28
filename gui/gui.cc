/**************************************************************************//**
  \file     gui.cc
  \brief    This document describes the interface of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#include "gui.h"
#include <utility>
#include "parser.h"

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(Glib::RefPtr<Gtk::Application> app) {
    this->app = std::move(app);
    ui_ = Gtk::Builder::create_from_resource("/ui/mainwindow.ui");
    if (ui_) {
        ui_->get_widget<Gtk::Box>("box", box_);
        status_label_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("status_label"));
        plot_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("plot_btn"));
        open_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("open_btn"));
        parse_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("parse_btn"));
        about_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("about_btn"));
        quit_btn_ = Glib::RefPtr<Gtk::Button>::cast_dynamic(ui_->get_object("quit_btn"));
        parse_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::parse_button_clicked));
        open_btn_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::open_button_clicked));
        quit_btn_->signal_clicked().connect([this]() { this->app->quit(); });
        add(*box_);
    }
    set_title("Main Window");
    set_default_size(600, 360);
    show_all();
}

void MainWindow::parse_button_clicked() {
    VCDParser parser(vcd_file_name_);
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
        status_label_->set_label(vcd_file_name_);
    }
}
