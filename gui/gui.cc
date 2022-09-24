/**************************************************************************//**
  \file     gui.cc
  \brief    This document describes the interface of the main form.
  \author   Lao·Zhu
  \version  V1.0.1
  \date     23. September 2022
 ******************************************************************************/

#include "gui.h"
#include <gtk/gtk.h>

static void quit_cb(GtkWindow *window) {
    gtk_window_close(window);
}

static void gui_app_active(GtkApplication *app, gpointer user_data) {
    /* Construct a GtkBuilder instance and load our UI description */
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/ui/mainwindow.ui", nullptr);

    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(builder, "mainwindow");
    gtk_window_set_application(GTK_WINDOW (window), app);

    GObject *button = gtk_builder_get_object(builder, "quit_btn");
    g_signal_connect_swapped (button, "clicked", G_CALLBACK(quit_cb), window);

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
