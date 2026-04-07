/* Header file for gui.c */
#ifndef GUI_H
#define GUI_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "structures.h"
#include "load_pdf.h"
#include "system_commands.h"

#define APP_ID "io.github.aaranyak.mockpapergen.application"

int launch_gui(int argc, char **argv);
void app_start(GtkApplication *app, gpointer *user_data);
typedef struct gui_data_s {
    QuestionDatabase *database;
    GtkWidget *database_info;
    int hash_table_size; /* This */

} GuiData; /* Stores gui data */
void update_database_info(QuestionDatabase *database, GtkWidget *database_info);
void save_callback(GtkButton *button, GuiData *gui_data);
void load_paper_callback(GtkButton *button, GuiData *gui_data);
#endif
