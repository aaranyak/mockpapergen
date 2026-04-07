/* Header file for question_database.c */
#ifndef QUESTION_DATABASE_H
#define QUESTION_DATABASE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "structures.h"
#include "load_pdf.h"
#include "system_commands.h"

typedef struct question_metadata_s QuestionMetadata; /* OHH RECURSION!!!! */
typedef struct subject_info_s SubjectInfo; /* inside recursion */
typedef struct keyword_data_s KeywordData; /* inside recursion **/
typedef struct database_s QuestionDatabase;

typedef struct question_metadata_s {
    // Below is this specific thing
    QuestionMetadata *next; /* Make it singly linked (sorry, future self if you have to face the repercussions of this decision) */
    PaperQuestion *question; /* Link to the correct one */

    // Now the metadata regarding type
    SubjectInfo *subject; /* The subject the question is of */
    int categories[32]; /* Heuristics regarding type of question*/
    int main_category; /* The major category */

    int is_used; /* This is for checking questions */

} QuestionMetadata; /* Here, question-metadata */

typedef struct keyword_data_s { /* Hash table entry */
    KeywordData *next; /* Hash collision resolver */

    char keyword[32]; /* This is a keyword */
    uint32_t hash; /* Quicker matching */
    int category; /* Category index */
} KeywordData;

typedef struct subject_info_s { /* Subject content heuristics */
    SubjectInfo *next; /* LEENKEED LEEEST YAAAAAAA */
    QuestionDatabase *database; /* Link */
    char name[128]; /* May be excessive (probably is) */

    // Below goes the EEEN FOOO
    int num_categories; /* Categories of questions */
    char categories[32][64]; /* Names of categories */
    int table_size; KeywordData **hash_table; /* Array of linked lists */
    QuestionMetadata **category_indices[32]; /* List of arrays */
    int index_lengths[32]; /* Length of the arrays */

} SubjectInfo; /* Here */

typedef struct database_s {
    int size; /* Number of stuffs */
    PaperQuestion *questions; /* Linked list of paper questions (don't worry I'll be doing some kind of a metadata somethingish) */
    QuestionMetadata *metadata; /* List of question metadatas, that contains info */
    // Now create all the hash tables used for indexing
    SubjectInfo *subjectlist; /* List of subject datas */

    uint32_t hash_keys[256]; /* Justincase */

} QuestionDatabase; /* Here */
uint32_t hash_string(char *string, uint32_t hash_keys[256]);
QuestionDatabase *init_database();
SubjectInfo *add_subject(QuestionDatabase *database, char *subject_name, int table_size);
int load_subject_data(QuestionDatabase *database, char *file_path, int table_size);
int get_category_from_table(char *word, SubjectInfo *subject);
void tag_categories(PaperQuestion *question, QuestionMetadata *metadata);
int database_add_question(QuestionDatabase *database, QuestionMetadata *last_question, PaperQuestion *original, char *subject);
int load_paper_into_database(QuestionDatabase *database, ParsedPaper *paper);
#endif
