/* generate_paper.c - Generates a question paper from all this stuff */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>
#include "structures.h"
#include "load_pdf.h"
#include "system_commands.h"
#include "question_database.h"

#define ERROR_THRESHOLD 42

int get_questions_for_paper(QuestionMetadata *question_list[128], QuestionDatabase *database, SubjectInfo *subject, int num_categories, int *categories, int marks) {
    /* Get the list of questions that will be put in the question paper */

    int current_marks = 0; /* Count number of marks */
    int num_questions = 0; /* see how they line up! */
    int errors = 0; /* This is so that this doesn't get stuck in a loop if not enough questions are found */ 
    while (current_marks < marks && errors < ERROR_THRESHOLD) { /* Until we've found enough questionsto fill in the blanks */
        int category = categories[random() % num_categories]; /* Get a random category */
        errors++; if (subject->index_lengths[category] == 0) continue; errors--; /* Done with this thing */
        QuestionMetadata *metadata = subject->category_indices[category][random() % subject->index_lengths[category]]; /* random question */
        errors++; if (metadata->is_used) continue; /* If this is a used question try again */
        errors = 0; /* Not a used question */
        question_list[num_questions] = metadata; num_questions++; current_marks += metadata->question->marks; /* Update values */
        metadata->is_used = 1; /* Update this */
    }
    for (int i = 0; i < num_questions; i++) question_list[i]->is_used = 0; /* Update this */
    
    if (errors) return 0; /* We couldn't find questions */

    return num_questions; /* We are done finding questions */
}

void write_question_html(xmlNode *document_body, PaperQuestion *question, int index, int depth) {
    /* Write in a question into the html file */
    
    // Create the html node for the big box
    xmlNode *big_box = xmlNewChild(document_body, 0, "div", ""); /* This is a big div with nothing in it */
    xmlNewProp(big_box, "class", "bigbox"); /* Set this to a bigbox type object */
    
    // Create the question text
    xmlNode *question_box = xmlNewChild(big_box, 0, "div", ""); /* This is a small div with some stuff that'll be put in it */
    char question_class[3][16] = {"numberquestion", "letterquestion", "romanquestion"}; /* These are the three classes so far */
    xmlNewProp(question_box, "class", question_class[depth - 1]); /* Set the proper type of indentation */
    // Create label text
    char label_text[8]; /* This is the text of the question number thingy */
    char roman_numerals[10][8] = {"(i) ", "(ii) ", "(iii) ", "(iv) ", "(v) ", "(vi) ", "(vii) ", "(viii) ", "(ix) ", "(x) "}; 
    switch (depth) { /* As of now only three depths done */
        case 1: /* This is a number */
            sprintf(label_text, "%d. ", index + 1); break; /* This is a simple number thing */
        case 2: /* This is a letter */
            strcpy(label_text, "a) "); label_text[0] += index; break; /* With the power of ASCII, calculate letter */
        case 3: /* This is a roman numeral */
            strcpy(label_text, roman_numerals[index]); break; /* Let's hope they don't invent roman numbers bigger than ten */
    }
    xmlNewTextChild(question_box, 0, "b", label_text); /* Add the label text into this */
    xmlNewTextChild(question_box, 0, "span", question->text); /* I don't have time to figure out fancy stuff so this is enough */

    // Add question contents
    if (question->contents) {
        xmlNode *content_box = xmlNewChild(big_box, 0, "div", ""); /* Add this thing here */
        xmlNewProp(content_box, "class", "contentbox"); /* Make this a contentbox */
        
        TextList *content = question->contents; /* Get the first content */
        int max_height = 0; char style_this[64]; /* Used for correcting size of the outer box */
        int offset = content->pos_y; /* This is the start that we offset from */
        do { /* Loop through all the contents */
            xmlNode *content_object = xmlNewTextChild(content_box, 0, "p", content->text); /* Add in the text here */
            xmlNewProp(content_object, "class", "contents"); /* Make it a content class */
            sprintf(style_this, "left: %dpt; top: %dpt;", content->pos_x, content->pos_y - offset); /* Add the style */
            xmlNewProp(content_object, "style", style_this); /* add the style to it */
            if (content->pos_y - offset > max_height) max_height = content->pos_y - offset; /* Calculate max height */
        } while (content = content->next); /* Add in the content */
        sprintf(style_this, "height: %dpt", max_height + 30); xmlNewProp(content_box, "style", style_this); /*  Set the height and add it */
    }

    // Add subquestions.
    PaperQuestion *subquestion = question->subquestions; /* For looping stuff */
    if (question->subquestions) do /* Loop through all the subquestions */
        write_question_html(document_body, subquestion, subquestion->index, depth + 1); /* Add the subquestion */
    while (subquestion = subquestion->next); /* Until the end */

    // Add question marks

    // Here you go - ????????????????????????????????????????????????????????????????

    // Ok just kidding not those kind of question marks
    
    if (!question->subquestions) { /* If there are no subquestions */
        char marks_text[8]; sprintf(marks_text, "[%d]", question->marks); /* Get the marks of the question */
        xmlNode *marks_object = xmlNewTextChild(big_box, 0, "div", marks_text); /* Add the marks thing */
        xmlNewProp(marks_object, "class", "marksbox"); /* Add the style */ 
    }
}

int generate_paper(QuestionDatabase *database, SubjectInfo *subject, int num_categories, int *categories, int marks, char *file_path) {
    /* This function generates a question paper from the database */

    QuestionMetadata *question_list[128]; /* Time to load in the questions */
    int num_questions = get_questions_for_paper(question_list, database, subject, num_categories, categories, marks); /* Get questions */
    if (!num_questions) return 0; /* If there aren't enough questions early return */

    // Create the xml tree
    xmlDoc *html_file = htmlReadFile("/home/aaranyak/school_dp_1/computer_science/ia/file_out.html", 0, /* Parse html file */
            HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET); /* Get the template for the paper */
    
    xmlNode *document_root = xmlDocGetRootElement(html_file); /* Get the root node of the html tree */
    xmlNode *document_body = document_root->children; /* This is the <body> element of the html file */
    while (strcmp(document_body->name, "body")) document_body = document_body->next; /* Get it by looping through the <html>'s children */
    
    // Now to add some Questions!
    for (int index = 0; index < num_questions; index++)
        /* Add questions to the html file */
        write_question_html(document_body, question_list[index]->question, index, 1); /* Add in a question from the database */ 
    
    // Now save the file

    // Get temporary file location
    char *temporary_file = get_temp_folder_path("papergen_html_out.html"); /* Get a file in the temp directory */
    htmlSaveFileFormat(temporary_file, html_file, 0, 1); /* Yes add indents */
    convert_html_to_pdf(temporary_file, file_path); /* Convert the html to pdf */
    remove(temporary_file); free(temporary_file); /* Do some quick clean-up */

    return num_questions;
}
