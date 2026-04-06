/* save_database.c - saves and reloads a database of questions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "structures.h"
#include "load_pdf.h"
#include "system_commands.h"
#include "question_database.h"

int write_textlist(char **buffer, TextList *text_list, int length) {
    /* This function encodes all the components of a text list struct, and adds it to a growing buffer */
    int text_length = strlen(text_list->text); /* Get length the of text by use of the strlen function */
    *buffer = realloc(*buffer, length + sizeof(int) * 3 + text_length); /* create space for everything */
    memcpy(*buffer + length, &text_list->pos_x, sizeof(int)); length += sizeof(int); /* Add this thing */
    memcpy(*buffer + length, &text_list->pos_y, sizeof(int)); length += sizeof(int); /* This thing too */
    memcpy(*buffer + length, &text_length, sizeof(int)); length += sizeof(int); /* quite important too */
    memcpy(*buffer + length, text_list->text, text_length); length += text_length; /* encodes contents */
    return length; /* This line is quite important so that the next round doesn't overwrite this stuff */ 
}

int write_question(char **buffer, PaperQuestion *question, int length) {
    /* Writes out a question to a database file */
    *buffer = realloc(*buffer, length + sizeof(int) * 3 + sizeof(question_type)); /* Allocate space for question details */

    // Write out those details
    memcpy(*buffer + length, &question->index, sizeof(int)); length += sizeof(int); /* Add in question index */
    memcpy(*buffer + length, &question->depth, sizeof(int)); length += sizeof(int); /* Add in question depth */
    memcpy(*buffer + length, &question->type, sizeof(question_type)); length += sizeof(question_type); /* Add in question type */
    memcpy(*buffer + length, &question->marks, sizeof(int)); length += sizeof(int); /* Add in question marks */

    // Paste in question text
    int text_length = sizeof(question->text); /* Get text length */
    *buffer = realloc(*buffer, length + sizeof(int) + text_length); /* Add this thing */
    memcpy(*buffer + length, &text_length, sizeof(int)); length += sizeof(int); /* write out the length of text */
    memcpy(*buffer + length, question->text, text_length); length += text_length; /* write out the question text */

    // Now paste out question contents
    int num_contents = 0; TextList *text_object = question->contents; /* Stuff for loading in question contents */
    if (text_object) do num_contents++; while (text_object = text_object->next); /* Get the number of contents */
    *buffer = realloc(*buffer, length + sizeof(int)); /* Add more space for number of contents */
    memcpy(buffer + length, &num_contents, sizeof(int)); length += sizeof(int); /* Put in the number of textlists in there */
    text_object = question->contents; if (text_object) do /* Loop through all the text objects */
        length = write_textlist(buffer, text_object, length) while (text_object = text_object->next); /* And write them in */
    
    // Now the subquestions.
    int num_subquestions = 0; PaperQuestion *subquestion = question->subquestions; /* Stuff for loading subquestions */
    if (subquestion) do num_subquestions++; while (subquestion = subquestion->next); /* Get number of subquestions */
    *buffer = realloc(*buffer, length + sizeof(int)); /* Add more space for number of subquestions */
    memcpy(buffer + length, &num_subquestions, sizeof(int)); length += sizeof(int); /* Put the number of subquestions here */
    subquestion = question->subquestions; if (subquestion) do /* Loop through all the subquestion s*/
        length = write_question(buffer, subquestion, length); /* Write out the subquestion (YEAAHHH RECURSION ;] ) */

    return length;
}
    

int save_database(QuestionDatabase *database, char *file_path) {
    /* Save all the questions into this database */
    
    char *buffer = (char*)malloc(16); /* Allocate the buffer to put questions in */
    int length = 16; /* Current length of buffer */
    memcpy(buffer, "questiondatabase", 16); /* Copy in a bit of a check string in there */

    QuestionMetadata *metadata = database->metadata; /* Start from the beginning */
    if (metadata) do { /* Loop through all the questions */
        buffer = realloc(buffer, length + 128); /* add space for the question subject */
        memcpy(buffer, metadata->subject->name, 128); length += 128; /* Copy in the length of the buffer */
        length = write_question(&buffer, metadata->question, length); /* Write in the question */
    } while (metadata = metadata->next); /* Move to the next question */

    FILE *file_handle = fopen(file_path, "w"); /* Open the file */
    for (int i = 0; i < length; i++) fputc(buffer[i]); /* Write out the file */
    fclose(file_handle);
    return length; /* As is traditional */
}
