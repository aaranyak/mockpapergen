/* gui.c - Opens the simple gui for the program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gui.h"

void app_start(GtkApplication *app, gpointer *user_data) {
    /* This function is called when the app starts */

}

int launch_gui(int argc, char **argv) {
    /* Launches the gui for the analyser */
    GtkApplication *app; /* This is the app thing for the whatever we're about to launch */
    app = gtk_application_new(APP_ID, 0); /* Create a new application */
    g_signal_connect(app, "activate", G_CALLBACK (app_start), 0); /* Connect the app start signal */
    int status = g_application_run(G_APPLICATION(app), argc, argv); /* Run the app */
    g_object_unref(app); /* Delete this */
    return status; /* So that nothing goes wrong */
}
