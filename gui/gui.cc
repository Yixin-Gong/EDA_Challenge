/**************************************************************************//**
  \file     gui.cc
  \brief    This document describes the interface of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#include "gui.h"
#include <gtk/gtk.h>
#include "parser.h"

static GObject *quit_button, *open_button, *parse_button;
static GObject *status_label;
static std::string filepath;

static void parsebtn_cb(GtkWindow *window) {
    VCDParser *parser = new VCDParser(filepath);
    delete parser;
}

static void on_open_response(GtkDialog *dialog, int response) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        g_autoptr(GFile) file = gtk_file_chooser_get_file(chooser);
        gtk_label_set_label(GTK_LABEL(status_label), g_file_get_basename(file));
        filepath = g_file_get_parse_name(file);
    }
    gtk_window_destroy(GTK_WINDOW (dialog));
}

static void openfile_cb(GtkWindow *window) {
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choose File", window, action, "_Cancel",
                                                    GTK_RESPONSE_CANCEL, "_Choose", GTK_RESPONSE_ACCEPT, NULL);
    gtk_widget_show(dialog);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "*.vcd");
    gtk_file_filter_add_pattern(filter, "*.vcd");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filter);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), "beamdata1.BeamData");
    g_signal_connect (dialog, "response", G_CALLBACK(on_open_response), NULL);
}

static void gui_app_active(GtkApplication *app, gpointer user_data) {
    /* Construct a GtkBuilder instance and load our UI description */
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/ui/mainwindow.ui", nullptr);

    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(builder, "mainwindow");
    gtk_window_set_application(GTK_WINDOW (window), app);

    status_label = gtk_builder_get_object(builder, "status_label");
    quit_button = gtk_builder_get_object(builder, "quit_btn");
    g_signal_connect_swapped (quit_button, "clicked", G_CALLBACK(gtk_window_close), window);
    open_button = gtk_builder_get_object(builder, "openfile_btn");
    g_signal_connect_swapped (open_button, "clicked", G_CALLBACK(openfile_cb), window);
    parse_button = gtk_builder_get_object(builder, "parse_btn");
    g_signal_connect_swapped (parse_button, "clicked", G_CALLBACK(parsebtn_cb), window);

    gtk_widget_show(GTK_WIDGET (window));

    /* We do not need the builder anymore */
    g_object_unref(builder);
}

int gui_display_main_window(int argc, char **argv) {
    GtkApplication *app;
    app = gtk_application_new("eda.statistic.tools", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK(gui_app_active), NULL);
    int status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);
    return status;
}
