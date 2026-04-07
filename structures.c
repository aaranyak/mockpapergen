/* structures.c - for manipulating the basic data structures of this program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/tree.h>
#include "system_commands.h"
#include "structures.h"

TextList *append_text_list(TextList *previous, char *text, int pos_x, int pos_y, int page_index) {
    /* Append a text list node onto a linked list */

    TextList *text_list = (TextList*)malloc(sizeof(TextList)); /* Create one in memory */
    text_list->pos_y = pos_y; text_list->pos_x = pos_x; /* Set coordinates */
    text_list->next = 0; /* Set the next to 0 */
    char *text_buffer = (char*)malloc(strlen(text) + 1); /* Create a buffer for the text list */
    strcpy(text_buffer, text); /* Copy text into text buffer */
    text_list->text = text_buffer; /* Add it on the component */
    if (previous) previous->next = text_list; /* Append it onto the buffer */
    text_list->page_index = page_index; /* BEHOLD! */
    return text_list; /* for convenience */
}

void delete_question_list(PaperQuestion *question) {
    /* Deletes a question list in it's entirity */
    PaperQuestion *current = question, *next; /* Somestuff I'm just mindlessly copying the stuff I wrote a while back */
    while (current) { /* I'm not exactly sane right now you see */
        next = current->next; /* Save this stuff before it goes away */
        if (current->contents) delete_text_list(current->contents); /* Properly free this */
        free(current->text); /* Free this */
        if (current->subquestions) delete_question_list(current->subquestions); /* Except for this function being recursive (FD above) */
        free(current); /* Get rid of it good */
        current = next; /* this is some line of code that is important for some reason I can't think right now why */
    }
}

TextList *copy_text_list(TextList *original) {
    /* Well, someone has to do it */
    TextList *new = (TextList*)malloc(sizeof(TextList)); /* Here, another textlist */
    new->next = 0; new->sort_score = 0; new->pos_x = original->pos_x; new->pos_y = original->pos_y; /* Copy vals */
    new->text = (char*)malloc(strlen(original->text) + 1); memcpy(new->text, original->text, strlen(original->text) + 1); /* Copy text */
    return new; /* Just realised this is totally not compatible with c++ */
} 

void delete_text_list(TextList *root) {
    /* Free the entire linked list of text lists */
    TextList *current = root, *next; /* Iterators */
    while (current) { /* Loop through the linked list */
        next = current->next; /* Get the next in line */
        free(current->text); free(current); /* Free this one */
        current = next; /* Move onto the next in line */
    }
}

PaperQuestion *copy_question(PaperQuestion *original) {
    /* Copies a question (and everything under it) */
    PaperQuestion *question = (PaperQuestion*)malloc(sizeof(PaperQuestion)); /* Create this thing */
    question->index = original->index; question->depth = original->depth; question->type = original->type; question->marks = original->marks;
    question->text = (char*)malloc(strlen(original->text) + 1); strcpy(question->text, original->text); /* Copy this thing in */
    question->next = 0; question->subquestions = 0; question->contents = 0; /* As of now we aren't putting anything in it anymore */ 

    // Copy textlist
    if (original->contents) {
        question->contents = copy_text_list(original->contents); TextList *last_one = question->contents; /* Set this up */
        TextList *this_one = original->contents; while (this_one = this_one->next) { /* Copy and add */
            last_one->next = copy_text_list(this_one); /* Add the next one */
            last_one = last_one->next; /* And move on with this one */
        }    
    } // Copy the subquestions
    if (original->subquestions) {
        question->subquestions = copy_question(original->subquestions); PaperQuestion *last_question = question->subquestions; /* There */
        PaperQuestion *this_question = original->subquestions; while (this_question = this_question->next) { /* Copy and add */
            last_question->next = copy_question(this_question); /* Add the next one */
            last_question = last_question->next; /* Move on */
        }
    }
    return question; /* Now be done with it */
}
