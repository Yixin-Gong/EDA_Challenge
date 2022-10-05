//
// Created by ubuntu on 22-9-30.
//

#include "aboutwindow.h"
#include "gitver.h"

AboutWindow::~AboutWindow() = default;

AboutWindow::AboutWindow(const std::string &software_version) {
    ui_ = Gtk::Builder::create_from_resource("/ui/aboutwindow.ui");
    if (ui_) {
        ui_->get_widget<Gtk::Box>("box", box_);
        software_version_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("software_version"));
        git_hash_value_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("git_hash_value"));
        compile_time_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("compile_time"));
        gtk_version_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("gtk_version"));
        gtkmm_version_ = Glib::RefPtr<Gtk::Label>::cast_dynamic(ui_->get_object("gtkmm_version"));
        add(*box_);
    }
    software_version_->set_label("Software version: " + software_version);
    git_hash_value_->set_label("Git version: " + std::string(GIT_VERSION_HASH));
    compile_time_->set_label("Compile time: " + std::string(__TIMESTAMP__));
    gtk_version_->set_label("GTK library version: " + std::string(GTK_LIBRARY_VERSION));
    gtkmm_version_->set_label("GTKMM library version: " + std::string(GTKMM_LIBRARY_VERSION));
    set_resizable(false);
    set_title("About Window");
    set_default_size(300, 155);
}