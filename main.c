/* mockpapergen - This tool allows you to extract questions from past papers, store them, and generate papers using them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "system_commands.h"
#include "structures.h"
#include "load_pdf.h"
#include "question_database.h"
#include "gui.h"

void savethis() {

    QuestionDatabase *database = init_database(); /* Make the database */
    load_subject_data(database, "/home/aaranyak/school_dp_1/computer_science/ia/subject_data_test.txt", 128); /* This stuff */
    char file_path[] = "/home/aaranyak/school_dp_1/computer_science/ia/sample.pdf";
    xmlDocPtr html_file = get_pdf_as_html(file_path);
    ParsedPaper *parsed_paper = parse_question_paper(html_file); /* Test this */
    int errors = load_paper_into_database(database, parsed_paper); /* Load paper */

    // Time to LOAD
    char file_path_2[] = "/home/aaranyak/school_dp_1/computer_science/ia/sample_2.pdf";
    html_file = get_pdf_as_html(file_path_2); /* Load file */
    parsed_paper = parse_question_paper(html_file); /* parse other file */
    errors += load_paper_into_database(database, parsed_paper); /* Hopefully nothing breaks */
    printf("Errors - %d\n", errors);
    SubjectInfo *computer_science = database->subjectlist; /* Since we've only loaded one */
    for (int i = 0; i < computer_science->num_categories; i++) { /* Loop through all the categories */
        int current_count = 0; QuestionMetadata *current = database->metadata; /* Going to loop now */
        if (current) do if (current->main_category == i) current_count++; while (current = current->next); /* NYOOOOOOM */
        printf("Category \"%s\" - %d\n", computer_science->categories[i], current_count); /* Get this */
    }
}

int main(int argc, char **argv) {
    /* This is just a main function for executing the main thing */
    srandom(time(NULL)); /* Well, you have to do this */
    return launch_gui(argc, argv);
}
