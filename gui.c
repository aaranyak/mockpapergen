/* gui.c - the simple ui for this system */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "system_commands.h"
#include "structures.h"
#include "load_pdf.h"
#include "question_database.h"
#include "save_database.h"
#include "gui.h"
#include "generate_paper.h"

void update_database_info(QuestionDatabase *database, GtkWidget *database_info) {
    /* Updates the database info */
    char final_text[1024]; final_text[0] = 0; /* For now */

    SubjectInfo *subject = database->subjectlist; /* Get the current subject */
    if (subject) do { /* Loop through all the subjects */
        char current_thing[256]; /* Sprintf into this */
        sprintf(current_thing, "%s\n", subject->name); /* Add the subject name */
        strncat(final_text, current_thing, 1024); /* Add it */
        for (int i = 0; i < subject->num_categories; i++) { /* Loop through all the categories */
            int num_questions = subject->index_lengths[i]; /* Check this */
            sprintf(current_thing, "    %s - %d\n", subject->categories[i], num_questions); /* add this */
            strncat(final_text, current_thing, 1024); /* Again do this */
        }
    } while (subject = subject->next); /* Loop through all the subjects */
    gtk_label_set_text(GTK_LABEL (database_info), final_text); gtk_widget_show_all(database_info); /* Update the text */
}

void save_callback(GtkButton *button, GuiData *gui_data) {
    /* Opens file chooser and loads in new subject */
    QuestionDatabase *database = gui_data->database; /* Unpack this bit because it's easier that way */
    save_database(database, "/home/aaranyak/school_dp_1/computer_science/ia/question_database"); /* Save the database */
}
void load_paper_callback(GtkButton *button, GuiData *gui_data) {
    /* Opens file chooser and loads in new paper */
    QuestionDatabase *database = gui_data->database; /* Unpack this bit because it's easier that way */
    GtkWidget *main_window = gtk_widget_get_toplevel(GTK_WIDGET (button)); /* Get the window for popup access */
    GtkFileChooserNative *file_chooser = gtk_file_chooser_native_new("Open File", GTK_WINDOW (main_window), GTK_FILE_CHOOSER_ACTION_OPEN, 0, 0); /* popup the file dialog thingy */
    gint result = gtk_native_dialog_run(GTK_NATIVE_DIALOG (file_chooser)); /* Run the file chooser */
    if (result != GTK_RESPONSE_ACCEPT) {g_object_unref(file_chooser); return;}
    char *file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (file_chooser)); /* Get this */

    // Load in the paper
    xmlDocPtr html_file = get_pdf_as_html(file_path); /* Load in the html file */
    ParsedPaper *parsed_paper = parse_question_paper(html_file); /* Test this */
    load_paper_into_database(database, parsed_paper); /* Load paper */
    delete_question_list(parsed_paper->questions); free(parsed_paper); /* Do the clean up */

    update_database_info(database, gui_data->database_info); /* Updates that thing */
    g_object_unref(file_chooser); /* Free stuff */
}

void app_start(GtkApplication *app, gpointer *user_data) {
    /* App launch callback */
    // Create a window, launch a database and load it in.    
    QuestionDatabase *database = init_database(); /* Make the database */
    GuiData *gui_data = (GuiData*)malloc(sizeof(GuiData)); /* Add this thing */
    
    // Add in function for automatically loading database later.
    load_subject_data(database, "/home/aaranyak/school_dp_1/computer_science/ia/subject_data_test.txt", 128); /* Load subject data */
    read_database(database, "/home/aaranyak/school_dp_1/computer_science/ia/question_database"); /* Load in database */

    GtkWidget *window = gtk_application_window_new(app); /* Create a new window */
    gtk_window_set_title(GTK_WINDOW (window), "Question Paper Database");
    gtk_window_set_default_size(GTK_WINDOW (window), 300,500); /* For now */


    // Layout Stuff
    GtkWidget *big_column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); /* Create the big layout box */ 
    GtkWidget *top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create the button row */
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create this row */
    // Content Stuff
    GtkWidget *database_info = gtk_label_new(""); /* Nothing in here yet */
    GtkWidget *save_button = gtk_button_new_with_label("Save Database State "); /* For saving the database state */
    GtkWidget *paper_button = gtk_button_new_with_label("Load Question Paper"); /* For loading in questions */
    
    // Setup info
    gui_data->database = database; gui_data->database_info = database_info; gui_data->hash_table_size = 128; /* Setup info */
    update_database_info(database, database_info); /* And do this too */

    // Connect signals
    g_signal_connect(save_button, "clicked", G_CALLBACK (save_callback), gui_data); /* This */
    g_signal_connect(paper_button, "clicked", G_CALLBACK (load_paper_callback), gui_data); /* And this */

    // Pack Stuff
    gtk_box_pack_start(GTK_BOX (big_column), top_row, 0, 0, 20); /* Pack this box in */
    gtk_box_pack_start(GTK_BOX (big_column), info_box, 0, 0, 20); /* Pack this in too */
    gtk_box_pack_start(GTK_BOX (top_row), save_button, 1, 1, 20); /* This one as well */
    gtk_box_pack_start(GTK_BOX (top_row), paper_button, 1, 1, 20); /* This one as well */
    gtk_box_pack_start(GTK_BOX (info_box), database_info, 1, 1, 20); /* And lastly this */
    gtk_container_add(GTK_CONTAINER (window), big_column); /* Don't forget to put everything in */

    gtk_widget_show_all(window); /* Enable this */

    // Testing addendum
    int indexarray[4] = {0, 1, 2, 4};
    generate_paper(database, database->subjectlist, 4, indexarray, 60, "/home/aaranyak/school_dp_1/computer_science/ia/test_paper.html");
}



int launch_gui(int argc, char **argv) {
    /* Launch the GUI of the application */
    GtkApplication *app; /* Application object used for creating windows */
    app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN); /* Create a new app */
    g_signal_connect(app, "activate", G_CALLBACK(app_start), NULL); /* Connect app to start callback  */
    int status = g_application_run(G_APPLICATION(app), argc, argv); /* Run application */
    g_object_unref(app); /* After done */

    return status;
}
