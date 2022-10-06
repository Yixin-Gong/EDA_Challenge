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
  MainWindow(Glib::RefPtr<Gtk::Application> app, const std::string &software_version);
  MainWindow(Glib::RefPtr<Gtk::Application> app, const std::string &software_version, const std::string &filename);
  ~MainWindow() override;

 protected:
  Glib::RefPtr<Gtk::Application> app_;
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
  void initialize_window_();
  void parse_file_header_();
};

#endif //EDA_CHALLENGE_GUI_MAINWINDOW_H_
