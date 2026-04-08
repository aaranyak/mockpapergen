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
    int text_length = strlen(question->text); /* Get text length (I had accidentally typed sizeof instead of strlen what an idiot :))*/
    *buffer = realloc(*buffer, length + sizeof(int) + text_length); /* Add this thing */
    memcpy(*buffer + length, &text_length, sizeof(int)); length += sizeof(int); /* write out the length of text */
    memcpy(*buffer + length, question->text, text_length); length += text_length; /* write out the question text */

    // Now paste out question contents
    int num_contents = 0; TextList *text_object = question->contents; /* Stuff for loading in question contents */
    if (text_object) do num_contents++; while (text_object = text_object->next); /* Get the number of contents */
    *buffer = realloc(*buffer, length + sizeof(int)); /* Add more space for number of contents */
    memcpy(*buffer + length, &num_contents, sizeof(int)); length += sizeof(int); /* Put in the number of textlists in there */
    text_object = question->contents; if (text_object) do /* Loop through all the text objects */
        length = write_textlist(buffer, text_object, length); while (text_object = text_object->next); /* And write them in */
    
    // Now the subquestions.
    int num_subquestions = 0; PaperQuestion *subquestion = question->subquestions; /* Stuff for loading subquestions */
    if (subquestion) do num_subquestions++; while (subquestion = subquestion->next); /* Get number of subquestions */
    *buffer = realloc(*buffer, length + sizeof(int)); /* Add more space for number of subquestions */
    memcpy(*buffer + length, &num_subquestions, sizeof(int)); length += sizeof(int); /* Put the number of subquestions here */
    subquestion = question->subquestions; if (subquestion) do /* Loop through all the subquestion s*/
        length = write_question(buffer, subquestion, length); /* Write out the subquestion (YEAAHHH RECURSION ;] ) */
            while (subquestion = subquestion->next); /* Move on to the next subquestion */
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
        memcpy(buffer + length, metadata->subject->name, 128); length += 128; /* Copy in the length of the buffer */
        length = write_question(&buffer, metadata->question, length); /* Write in the question */
    } while (metadata = metadata->next); /* Move to the next question */

    FILE *file_handle = fopen(file_path, "wb"); /* Open the file */
    for (int i = 0; i < length; i++) fputc(buffer[i], file_handle); /* Write out the file */
    fclose(file_handle);
    return length; /* As is traditional */
}

int read_textlist(char *file_data, int read_head, TextList *text_list, int file_length) {
    /* Read the contents of a text list encoded into the actual struct */
    text_list->next = 0; text_list->page_index = 0; /* To make sure nothing breaks */
    memcpy(&text_list->pos_x, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Read in x position */
    memcpy(&text_list->pos_y, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Read in y position */
    int text_length; memcpy(&text_length, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Now readin the text length */
    text_list->text = (char*)malloc(text_length + 1); /* Create space for text */
    memcpy(text_list->text, file_data + read_head, text_length); read_head += text_length; /* Read in the text */
    text_list->text[text_length] = 0; /* Add null terminator */
    return read_head; /* To ensure that this thing keeps moving ahead */
}

int read_question(char *file_data, int read_head, PaperQuestion *question, int file_length) {
    /* This thing reads in question data and can be used recursively */
    
    // Listen you can always handle length-checks later as of now just load in whatever if it's badly formatted let it segfault
    memcpy(&question->index, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Load in question index */
    memcpy(&question->depth, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Load in question depth */
    memcpy(&question->type, file_data + read_head, sizeof(question_type)); read_head += sizeof(question_type); /* Load in question type */
    memcpy(&question->marks, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Load in question marks */

    // Now load in question text
    int text_length; memcpy(&text_length, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Get length of text */
    question->text = (char*)malloc(text_length + 1); /* Get enough space to put in text stuff */
    memcpy(question->text, file_data + read_head, text_length); read_head += text_length; question->text[text_length] = 0; /* text+NT */

    // Load in question contents
    question->contents = 0; /* This too, yknow */
    int num_contents; memcpy(&num_contents, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Get number of contents */
    TextList *current_content, *last_content = 0; while (num_contents--) { /* Loop through all the contents and add them one by one */
        current_content = (TextList*)malloc(sizeof(TextList)); /* Create the content space */
        read_head = read_textlist(file_data, read_head, current_content, file_length); /* Read in the text list contents */
        if (last_content) last_content->next = current_content; /* Add it to the previous one if not first */
        else question->contents = current_content; /* Otherwise directly put it */
        last_content = current_content; /* Push the append pointer forward (metaphorically) */
    }

    // Load in subquestions
    question->subquestions = 0; /* And this one! */
    int subquestions; memcpy(&subquestions, file_data + read_head, sizeof(int)); read_head += sizeof(int); /* Get number of subquestions */
    PaperQuestion *current_subquestion, *last_subquestion = 0; while (subquestions--) { /* Until we've checked this many subquestions */
        current_subquestion = (PaperQuestion*)malloc(sizeof(PaperQuestion)); /* Create a question struct */
        read_head = read_question(file_data, read_head, current_subquestion, file_length); /* Read in the subquestion */
        if (last_subquestion) last_subquestion->next = current_subquestion; /* append if not first */
        else question->subquestions = current_subquestion; /* Otherwise just assign */
        last_subquestion = current_subquestion; /* That same metaphor from above */
    }

    question->next = 0; /* This is to prevent recursion from breaking it */
    return read_head; /* ditto */
}

int read_database(QuestionDatabase *database, char *file_path) {
    /* Read from a saved database */
    FILE *file_handle = fopen(file_path, "rb"); /* Open the file path */
    int length = 0; while (!feof(file_handle)) {length++; fgetc(file_handle);} /* Get the file length */
    char *file_data = (char*)malloc(length); length--; /* Create the place to put the file data */
    rewind(file_handle); for (int i = 0; i < length; i++) file_data[i] = fgetc(file_handle); /* Load in the file data */
    fclose(file_handle); /* Close the file */
    int read_head = 16; /* Skip the test string (why would we check it anyway) */
    
    char subject[128]; PaperQuestion *current_question; QuestionMetadata *last_question = 0; /* For looping the loop */
    while (read_head < length) { /* Until the end of the file has been reached */
        memcpy(subject, file_data + read_head, 128); read_head += 128; /* Read in subject title */
        current_question = (PaperQuestion*)malloc(sizeof(PaperQuestion)); /* Temporary for loading in question */
        read_head = read_question(file_data, read_head, current_question, length); /* Read in current question */
        database_add_question(database, last_question, current_question, subject); /* Load the question into the database */
        delete_question_list(current_question); /* Which is technically a linked list of one element so this function is compatible */
        last_question = last_question ? last_question->next : database->metadata; /* Hmm Hmm Hmmy Hmm */
    }
    
    free(file_data); /* for now... */
    return length;
}
