/* Header file containing important data structures */
#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>

// Paper parsing stuff
typedef struct text_list_s TextList; /* FD for linked list */
typedef struct paper_question_s PaperQuestion; /* FD ditto */
typedef struct parsed_paper_s { /* This is the result from parsing a paper */
    // First some metadata
    char subject[32]; /* String containing subject */
    char target_exam[32]; /* Target exam date */
    char name[128]; /* Paper name */
    int time; /* in minutes */
    // Actual content
    int num_questions; /* Number of questions */
    PaperQuestion *questions; /* Actual list of questions */
} ParsedPaper; /* This will contain stuff */

typedef struct text_list_s { /* This is a list of texts */
    char *text; /* This is the text contained by the thing */
    int pos_x; /* X position */
    int pos_y; /* Y position */
    int page_index; /* Page index */
    TextList *next; /* Pointer to next in the list */
    int sort_score; /* Used for sorting the thing */
} TextList;

typedef enum question_type_e { /* Question types */
    super, /* Superquestion (only subquestions) */
    short_answer, /* Normal saq */
    multiple_choice,
} question_type; /* Question typedef */

typedef struct paper_question_s { /* This holds the structure of a question */
    // Math
    int index; /* The index of the question in current tree */
    int depth; /* Subquestion order */
    // Stuffs
    question_type type; /* Obvious */
    int marks; /* This is number of marks */
    char *text; /* The main question */

    TextList *contents; /* Contents of the question */
    // Meta
    PaperQuestion *subquestions; /* Subquestions of this one */
    PaperQuestion *next; /* Next few questions */
} PaperQuestion; /* Typedef */

TextList *append_text_list(TextList *previous, char *text, int pos_y, int pos_x, int page_index);
void delete_text_list(TextList *root);
TextList *copy_text_list(TextList *original);
void delete_question_list(PaperQuestion *question);
PaperQuestion *copy_question(PaperQuestion *original);
#endif
