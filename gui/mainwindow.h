/**************************************************************************//**
  \file     gui.h
  \brief    This header file describes the functions of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#ifndef EDA_CHALLENGE_GUI_MAINWINDOW_H_
#define EDA_CHALLENGE_GUI_MAINWINDOW_H_

#include <gtkmm.h>
#include <gtkmm-plplot.h>
#include <iostream>
#include "parser.h"
#include "aboutwindow.h"

class MainWindow : public Gtk::ApplicationWindow {
 public:
  MainWindow(Glib::RefPtr<Gtk::Application> app, const std::string &software_version) : canvas_() {
      this->app = std::move(app);
      this->software_version_ = software_version;
      ui_ = Gtk::Builder::create_from_resource("/ui/mainwindow.ui");
      Gdk::Geometry geometry;
      ui_->get_widget<Gtk::Box>("box", box_);
      ui_->get_widget<Gtk::Grid>("subgrid", grid_);
      geometry.min_aspect = geometry.max_aspect = double(600) / double(400);
      set_geometry_hints(*this, geometry, Gdk::HINT_ASPECT);
      canvas_.set_hexpand(true);
      canvas_.set_vexpand(true);
      plot_button_clicked();
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
      quit_btn_->signal_clicked().connect([this]() { this->app->quit(); });

      add(*box_);
      set_resizable(false);
      set_title("Main Window");
      set_default_size(600, 400);
      show_all();
  }
  ~MainWindow() override;

 protected:
  Glib::RefPtr<Gtk::Application> app;
  void open_button_clicked();
  void parse_button_clicked();
  void plot_button_clicked();
  void about_button_clicked();

 private:
  Gtk::Box *box_{};
  Gtk::Grid *grid_{};
  Glib::RefPtr<Gtk::Label> status_label_;
  Glib::RefPtr<Gtk::Label> unit_label_;
  Glib::RefPtr<Gtk::Label> to_label_;
  Glib::RefPtr<Gtk::Button> plot_btn_;
  Glib::RefPtr<Gtk::Button> open_btn_;
  Glib::RefPtr<Gtk::Button> parse_btn_;
  Glib::RefPtr<Gtk::Button> about_btn_;
  Glib::RefPtr<Gtk::Button> quit_btn_;
  Glib::RefPtr<Gtk::Builder> ui_;
  std::string vcd_file_name_;
  std::string software_version_;
  VCDParser *parser_{};
  AboutWindow *about_window_{};
  Gtk::PLplot::Canvas canvas_;
  Gtk::PLplot::Plot2D *plot_{};
  Glib::ustring x_title_ = "Time(ns)";
  const Glib::ustring y_title_ = "Signal Number";
  const Glib::ustring plot_title_ = "Flip signal number statistics";
  const Gdk::RGBA curve_color_ = Gdk::RGBA("Blue");
};

#endif //EDA_CHALLENGE_GUI_MAINWINDOW_H_
