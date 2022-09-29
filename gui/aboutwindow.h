//
// Created by ubuntu on 22-9-30.
//

#ifndef EDA_CHALLENGE_GUI_ABOUTWINDOW_H_
#define EDA_CHALLENGE_GUI_ABOUTWINDOW_H_

#include <gtkmm.h>
#include <iostream>

class AboutWindow : public Gtk::ApplicationWindow {
 public:
  explicit AboutWindow(const std::string &software_version);
  ~AboutWindow() override;

 private:
  Gtk::Box *box_{};
  Glib::RefPtr<Gtk::Label> software_version_;
  Glib::RefPtr<Gtk::Label> git_hash_value_;
  Glib::RefPtr<Gtk::Label> compile_time_;
  Glib::RefPtr<Gtk::Label> gtk_version_;
  Glib::RefPtr<Gtk::Label> gtkmm_version_;
  Glib::RefPtr<Gtk::Builder> ui_;
};

#endif //EDA_CHALLENGE_GUI_ABOUTWINDOW_H_
