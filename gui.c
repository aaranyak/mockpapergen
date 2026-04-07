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
    SubjectInfo *current_subject = database->subjectlist; int subject_index = 0; /* For looping help */
    GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW (database_info)); /* Get the tree model */
    if (current_subject) do { /* Loop through all the subjects */
        char whatever_path[8]; sprintf(whatever_path, "%d", subject_index); /* Get the subject path */
        GtkTreeIter subject; gtk_tree_model_get_iter_from_string(tree_model, &subject, whatever_path); /* Get the iter */
        int total_questions = 0, total_marks = 0, marks = 0; /* For later I guess */
        for (int i = 0; i < current_subject->num_categories; i++) { /* Loop through all the categories */
            sprintf(whatever_path, "%d:%d", subject_index, i); /* Get the better whateverpath */
            GtkTreeIter category; gtk_tree_model_get_iter_from_string(tree_model, &category, whatever_path); /* Get the iter */
            int num_questions = current_subject->index_lengths[i]; marks = 0; /* Get the number of questions */
            for (int j = 0; j < num_questions; j++) marks += current_subject->category_indices[i][j]->question->marks; /* Calculate marks */
            total_questions += num_questions; total_marks += marks; /* Do this stuff */
            gtk_tree_store_set(GTK_TREE_STORE (tree_model), &category, 1, num_questions, 2, marks, -1); /* Set this stuff */
        }
        gtk_tree_store_set(GTK_TREE_STORE (tree_model), &subject, 1, total_questions, 2, total_marks, -1); /* Set this stuff */
        subject_index++;
    } while (current_subject = current_subject->next);
    gtk_widget_show_all(database_info); /* Update this */
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



void configure_tree_view(GuiData *data) {
    /* Configures the gtk tree view */
    GtkWidget *tree_view = data->database_info; /* This is important */
    QuestionDatabase *database = data->database; /* Get the database */
    GtkCellRenderer *renderer; /* Get a "cell renderer" whatever it may be */

    // The first column
    renderer = gtk_cell_renderer_text_new(); /* Renders, columns I believe */
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (tree_view), -1, "Category", renderer, "text", 0, NULL); /* Create the first column */
    // The second column
    renderer = gtk_cell_renderer_text_new(); /* Renders, columns I believe */
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (tree_view), -1, "Questions", renderer, "text", 1, NULL); /* Create the second column */
    // The second column
    renderer = gtk_cell_renderer_text_new(); /* Renders, columns I believe */
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (tree_view), -1, "Marks", renderer, "text", 2, NULL); /* Create the secondcolumn */

    // Create the model
    GtkTreeStore *tree_store = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT); /* Create the tree store */
    GtkTreeIter subject, category; /* For manipulating data */

    SubjectInfo *current_subject = database->subjectlist; /* Set this to this */
    if (current_subject) do { /* Loop through all the subjects */
        gtk_tree_store_append(GTK_TREE_STORE (tree_store), &subject, 0); /* Append a new column */
        int total_questions = 0, total_marks = 0, marks = 0; /* Counting stuff */
        for (int i = 0; i < current_subject->num_categories; i++) { /* Loop through all the categories */
            gtk_tree_store_append(GTK_TREE_STORE (tree_store), &category, &subject); /* Add this column */
            int num_questions = current_subject->index_lengths[i]; /* Get the number of questions */
            total_questions += num_questions; marks = 0; /* Make sure of this */
            for (int j = 0; j < num_questions; j++) marks += current_subject->category_indices[i][j]->question->marks; /* Do the math */
            total_marks += marks; /* This too */
            gtk_tree_store_set(tree_store, &category, 0, current_subject->categories[i], 1, num_questions, 2, marks, -1); /* Set vals */
        }
        gtk_tree_store_set(tree_store, &subject, 0, current_subject->name, 1, total_questions, 2, total_marks, -1); /* Set vals */
    } while (current_subject = current_subject->next); /* Loop through all the subjects */

    gtk_tree_view_set_model(GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (tree_store)); /* Set this */

}

void update_checkbox_info(GtkToggleButton *button, int *target) {
    /* This updates the button thing */
    *target = gtk_toggle_button_get_active(button);
}

void update_category_selector(GtkComboBox *subject_choice, GuiData *data) {
    /* Update the category set */
    int subject_index = gtk_combo_box_get_active(subject_choice); /* Do this */
    QuestionDatabase *database = data->database; /* I know that feels odd but bear with me */
    SubjectInfo *subject = database->subjectlist; for (int i = 0; i < subject_index; i++) subject = subject->next; /* Get the subject */
    for (int i = 0; i < subject->num_categories; i++) data->categories[i] = 1; /* Set all categories to 1 */
    GtkWidget *outside_box = data->categories_box; /* Set this up */
    gtk_widget_destroy((GtkWidget*)gtk_container_get_children(GTK_CONTAINER (outside_box))->data); /* Destroy the older stuff */
    GtkWidget *inside_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); /* Create a vertcically oriented box */
    for (int i = 0; i < subject->num_categories; i++) { /* Add the selectors */
        GtkWidget *check_button = gtk_check_button_new_with_label(subject->categories[i]); /* Create a new check button */
        GtkWidget *padding_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Padding Box */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check_button), 1); /* Make this active */
        g_signal_connect(check_button, "toggled", G_CALLBACK (update_checkbox_info), &(data->categories[i])); /* Connect signal */
        gtk_box_pack_start(GTK_BOX (inside_box), padding_box, 0, 0, 5); /* Add check button */
        gtk_box_pack_start(GTK_BOX (padding_box), check_button, 1, 1, 10); /* Add check button */
    }
    gtk_box_pack_start(GTK_BOX (outside_box), inside_box, 0, 0, 0); /* Put the inside box in the outside box */
    gtk_widget_show_all(outside_box); /* Update this */
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
    gtk_window_set_default_size(GTK_WINDOW (window), 300,700); /* For now */


    // Layout Stuff
    GtkWidget *big_column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); /* Create the big layout box */ 
    GtkWidget *top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create the button row */
    GtkWidget *info_scroller = gtk_scrolled_window_new(0, 0); /* Set these to null for now */
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create this row */
    GtkWidget *settings_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create this row */
    GtkWidget *categories_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create this row */
    GtkWidget *categories_inside = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); /* Inside of categories */
    GtkWidget *generate_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Outer padding box for generate button */
    GtkWidget *generate = gtk_button_new_with_label("Generate Paper"); /* Paper Generation Button */

    GtkWidget *total_marks = gtk_spin_button_new_with_range(1, 1000, 1); /* Spin button for calculating marks */
    GtkWidget *subject_choice = gtk_combo_box_text_new(); /* This is a simpler version of the famed combo box */
    for (SubjectInfo *subject = database->subjectlist; subject; subject = subject->next) gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (subject_choice), subject->name); /* Add a combo box text element */

    // Content Stuff
    GtkWidget *database_info = gtk_tree_view_new(); /* Nothing in here yet */
    GtkWidget *save_button = gtk_button_new_with_label("Save Database State "); /* For saving the database state */
    GtkWidget *paper_button = gtk_button_new_with_label("Load Question Paper"); /* For loading in questions */
    
    // Setup info
    gui_data->database = database; gui_data->database_info = database_info; gui_data->hash_table_size = 128; /* Setup info */
    gui_data->categories_box = categories_box; gui_data->num_marks = total_marks; gui_data->subject_chosen = subject_choice; /* Use this to refer to them later */
    gtk_widget_set_size_request(info_scroller, -1, 150); /* Make this a little big */
    configure_tree_view(gui_data);

    // Connect signals
    g_signal_connect(save_button, "clicked", G_CALLBACK (save_callback), gui_data); /* This */
    g_signal_connect(paper_button, "clicked", G_CALLBACK (load_paper_callback), gui_data); /* And this */
    g_signal_connect(subject_choice, "changed", G_CALLBACK (update_category_selector), gui_data); 

    // Pack Stuff
    gtk_box_pack_start(GTK_BOX (big_column), top_row, 0, 0, 20); /* Pack this box in */
    gtk_box_pack_start(GTK_BOX (big_column), info_box, 0, 0, 20); /* Pack this in too */
    gtk_box_pack_start(GTK_BOX (top_row), save_button, 1, 1, 20); /* This one as well */
    gtk_box_pack_start(GTK_BOX (top_row), paper_button, 1, 1, 20); /* This one as well */
    gtk_box_pack_start(GTK_BOX (info_box), info_scroller, 1, 1, 20); /* And lastly this */
    gtk_container_add(GTK_CONTAINER (info_scroller), database_info); /* This one mainly */
    gtk_box_pack_start(GTK_BOX (big_column), settings_box, 0, 0, 20); /* Another one, */
    gtk_box_pack_start(GTK_BOX (settings_box), total_marks, 1, 1, 20); /* This one too */
    gtk_box_pack_start(GTK_BOX (settings_box), subject_choice, 1, 1, 20); /* This one too */
    gtk_box_pack_start(GTK_BOX (big_column), categories_box, 1, 1, 20); /* Another one, */
    gtk_box_pack_start(GTK_BOX (categories_box), categories_inside, 0, 0, 20); /* Another one, */
    gtk_box_pack_start(GTK_BOX (big_column), generate_box, 0, 0, 20);
    gtk_box_pack_start(GTK_BOX (generate_box), generate, 1, 1, 20); /* Add generate button */
    gtk_container_add(GTK_CONTAINER (window), big_column); /* Don't forget to put everything in */

    gtk_widget_show_all(window); /* Enable this */
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
