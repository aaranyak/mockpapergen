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


void test_print_stuff(ParsedPaper *parsed_paper) {
    printf("Paper Metadata:\n    Subject: %s\n    Target Exam: %s\n    Name: %s\n    Time Limit: %d minutes\n", parsed_paper->subject, parsed_paper->target_exam, parsed_paper->name, parsed_paper->time); /* If you are still sane right now you are not working hard enough */
    PaperQuestion *current = parsed_paper->questions; /* Print all the superquestions one by one */
    for (printf("\n"); current; current = current->next) { /* Loop through all the supers */
        printf("Question %d: %s (%d marks)\n", current->index + 1, current->text, current->marks); /* Print the question */
        if (current->subquestions) for (PaperQuestion *subquestion = current->subquestions; subquestion; subquestion = subquestion->next) {
            printf("    Subquestion %d: %s (%d marks)\n", subquestion->index + 1, subquestion->text, subquestion->marks); /* SUBQUESTION */
            if (subquestion->subquestions) for (PaperQuestion *subsub = subquestion->subquestions; subsub; subsub = subsub->next)
                printf("        SubSubquestion %d: %s (%d marks)\n", subsub->index + 1, subsub->text, subsub->marks); /* META */
        }
    }
}

int main(int argc, char **argv) {
    /* This is just a main function for executing the main thing */
    srandom(time(NULL)); /* Well, you have to do this */
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
    return 0;
}
