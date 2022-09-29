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
#include <iostream>
#include "parser.h"

class MainWindow : public Gtk::ApplicationWindow {
 public:
  explicit MainWindow(Glib::RefPtr<Gtk::Application> app);
  ~MainWindow() override;

 protected:
  Glib::RefPtr<Gtk::Application> app;
  void open_button_clicked();
  void parse_button_clicked();
  void plot_button_clicked();
  void about_button_clicked();

 private:
  Gtk::Box *box_{};
  Glib::RefPtr<Gtk::Label> status_label_;
  Glib::RefPtr<Gtk::Button> plot_btn_;
  Glib::RefPtr<Gtk::Button> open_btn_;
  Glib::RefPtr<Gtk::Button> parse_btn_;
  Glib::RefPtr<Gtk::Button> about_btn_;
  Glib::RefPtr<Gtk::Button> quit_btn_;
  Glib::RefPtr<Gtk::Builder> ui_;
  std::string vcd_file_name_;
  VCDParser *parser_{};
};

#endif //EDA_CHALLENGE_GUI_MAINWINDOW_H_
