/* question_database.c - Handles saving a paper questions into the database */
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

uint32_t hash_string(char *string, uint32_t hash_keys[256]) {
    /* Generates a hash (simple thing IDK how safe it is) for a string */
    uint32_t hash = 0; /* start with this */
    for (int i = 0; i < strlen(string); i++) /* Loop through the string */
        hash ^= hash_keys[(uint8_t)string[i]] * hash_keys[i % 256]; /* Some weird string stuff */ 
    return hash; /* Let's see how this hashing algorithm fares for us */
}

QuestionDatabase *init_database() {
    /* Creates an empty database with all of the stuff as 0 */
    QuestionDatabase *database = (QuestionDatabase*)malloc(sizeof(QuestionDatabase)); /* Create the outside dabba */
    database->questions = 0; /* Init questions list to 0 */
    database->metadata = 0; /* Init the metadata list to 0 */
    database->size = 0; /* Number of stuffs (questions/metadata) */
    database->subjectlist = 0; /* Here you go */

    for (int i = 0; i < 256; i++) /* Generate 256 random numbers */
        database->hash_keys[i] = random() | (random() << 31); /* Generate random uint32 */

    return database; /* Ok just init the stuff as 0 */
}

SubjectInfo *add_subject(QuestionDatabase *database, char *subject_name, int table_size) {
    /* This adds a subject, with a name and a hash table */ 
    SubjectInfo *subject_info = (SubjectInfo*)malloc(sizeof(SubjectInfo)); /* Create the main thing */
    subject_info->next = 0; /* This is the last in the list */
    subject_info->database = database; /* Simple */
    strncpy(subject_info->name, subject_name, 127); /* Ok. There is no buffer overflow risk here. You happy? */
    subject_info->num_categories = 0; /* As of now */
    memset(subject_info->categories, 0, 32 * 64); /* Fill in this one with zeroes */
    subject_info->table_size = table_size; subject_info->hash_table = (KeywordData**)malloc(sizeof(KeywordData*) * table_size); /*malloc*/
    memset(subject_info->hash_table, 0, sizeof(KeywordData*) * table_size); /* There you go */
    
    if (!database->subjectlist) database->subjectlist = subject_info; /* BEE HOLD ! */
    else { /* Get last and append */
        SubjectInfo *current = database->subjectlist; /* Get this one */
        while (current->next) current = current->next; /* Get last one */
        current->next = subject_info; /* Upend */
    }
    return subject_info; /* Ok make it easier for me later */
}

void add_keyword(SubjectInfo *subject_info, char *keyword, char *category) {
    /* Add a keyword into the subject info */
    int index = -1; for (int i = 0; i < subject_info->num_categories; i++) if (!strcmp(subject_info->categories[i], category)) index = i;
    if (index == -1) return; /* porcupine */
    KeywordData *keyword_data = (KeywordData*)malloc(sizeof(KeywordData)); /* This is something I have to do a lot, you see */    
    keyword_data->next = 0; /* Last as of now */
    strncpy(keyword_data->keyword, keyword, 31); /* Strncpy this */
    keyword_data->hash = hash_string(keyword, subject_info->database->hash_keys); /* Hash this in this place */
    keyword_data->category = index; /* Put it here */
    
    // Add it to the hash table
    KeywordData *current_one = subject_info->hash_table[keyword_data->hash % subject_info->table_size]; /* See if anything is there */
    if (current_one) { /* We need to handle an index collision */
        while (current_one->next) current_one = current_one->next; /* Get the last one on the list */
        current_one->next = keyword_data; /* append it instead of just putting it there */
    } else subject_info->hash_table[keyword_data->hash % subject_info->table_size] = keyword_data; /* Put this at this point */
    return;
}

int load_subject_data(QuestionDatabase *database, char *file_path, int table_size) {
    /* Loads the question data from a file */

    FILE *data_file = fopen(file_path, "r"); /* Open the file and get a handle */
    int file_length = 0; for (;!feof(data_file); fgetc(data_file)) file_length++; rewind(data_file); /* I know for loops look gross but listen, this way I get to avoid having to put braces in my code, or use some cursed conditions in a do while */
    char *file_contents = (char*)malloc(file_length); for (int i = 0; i < file_length; i++) file_contents[i] = fgetc(data_file);
    fclose(data_file); /* Finally we are done with the file handle */
    
    // You've to finish the database today start programming FAST
    char subject_name[128]; int read_head = 0, write_head = 0; /* set up for reading in subject name */
    do subject_name[write_head++] = file_contents[read_head++]; while (file_contents[read_head] != '\n'); /* Load the subject in */
    subject_name[write_head] = 0; write_head = 0; read_head++; /* Move onto next part */

    SubjectInfo *subject_info = add_subject(database, subject_name, table_size);;

    // Now load in all the that stuffs
    while (read_head < file_length) { /* I guess I could have done this otherwise as well but so what whatever */
        char category[64]; do category[write_head++] = file_contents[read_head++]; while (file_contents[read_head] != ':'); /* Load cat */
        category[write_head] = 0; write_head = 0; read_head++; /* Move on to the next one */
        strncpy(subject_info->categories[subject_info->num_categories], category, 63); /* I should get used to making this memory safe */
        subject_info->num_categories++; /* bee hold (not safe without beekeeping equipment) */
        while (file_contents[read_head] != '\n') { /* For each of the keywords */
            char keyword[32]; do keyword[write_head++] = file_contents[read_head++]; while (file_contents[read_head] != ','); /* Load key */
            keyword[write_head] = 0; write_head = 0; read_head++; /* Move on to the next one */
            add_keyword(subject_info, keyword, category); /* Add it in */
        }
        read_head++;
        if (file_contents[read_head] == '\n') break;
    }
    free(file_contents); /* I can't believe you forgot that */
    return 0;
}

int get_category_from_table(char *word, SubjectInfo *subject) {
    /* Search for a keyword in a table */
    uint32_t hash = hash_string(word, subject->database->hash_keys); /* Hash the string */
    int category = -1; /* For now */
    KeywordData *keyword = subject->hash_table[hash % subject->table_size]; /* Get the position in table */
    if (keyword) do if (keyword->hash == hash) category = keyword->category; while (keyword = keyword->next); /* Try to get the category */
    return category; /* Get the category */
}

void tag_categories(PaperQuestion *question, QuestionMetadata *metadata) {
    /* Use the keywords hash table to figure out the categories of this thing */
    
    // First for single words
    char load_word[32]; char letter; int read_head = 0, write_head = 0, tag_category = 0;/* Use this stuff to load stuff in */
    int length = strlen(question->text); /* Loopthrough it here */
    while (read_head < length) { /* Until we've reached the end of the text */
        letter = question->text[read_head++]; /* Get the current char */
        if (letter == ' ' && write_head != 0) { /* We have finished loading a char word */
            load_word[write_head] = 0; write_head = 0; /* Load word into the that thing */
            tag_category = get_category_from_table(load_word, metadata->subject); /* Try to look up in the keyword hash table */
            if (tag_category != -1) metadata->categories[tag_category]++; /* Add to the tag heuristic */
        } if ((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z')) /* If the character is alphabetic */
            load_word[write_head++] = (letter > 'Z') ? letter : letter + 32; /* connvert it to lowercase and add it */
    } read_head = 0; write_head = 0; /* Reset both of these */

    // Time to load in lists of two words
    int last_index = 0;
    while (read_head < length) { /* This is gong to be tough */
        letter = question->text[read_head++]; /* Get the character */
        if (letter == ' ' && write_head != 0) { /* Now we need to differenciate */
            if (last_index) { /* we are done but we need to go back */
                load_word[write_head] = 0; write_head = 0; /* Load word into the that thing */
                tag_category = get_category_from_table(load_word, metadata->subject); /* Check the hash table */
                if (tag_category != -1) metadata->categories[tag_category]++; /* Add to the tag heuristic */
                read_head = last_index; /* Go back to this one */
                last_index = 0; /* Now I hope it does not infinite loop */
            } else { /* We are just in the middle */
                last_index = read_head; /* Hereyougo */
                load_word[write_head++] = ' '; /* There you go! */
            }
        } if ((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z')) /* If the character is alphabetic */
            load_word[write_head++] = (letter > 'Z') ? letter : letter + 32; /* connvert it to lowercase and add it */
    }
    // Now for getting in all the subquestions
    PaperQuestion *subquestion = question->subquestions; /* Get the subquestion */
    if (subquestion) do tag_categories(subquestion, metadata); while (subquestion = subquestion->next); /* handle all the subquestions */
}

int database_add_question(QuestionDatabase *database, QuestionMetadata *last_question, PaperQuestion *original, char *subject) {
    /* Adds a question to the database */
    PaperQuestion *question = copy_question(original); /* Make a copy of the original question */
    QuestionMetadata *metadata = (QuestionMetadata*)malloc(sizeof(QuestionMetadata)); /* Create the metadata object */
    metadata->next = 0; metadata->question = question; metadata->subject = database->subjectlist; /* Set up this metadata */
    if (database->subjectlist) do // The following: 
        if (!strcmp(subject, metadata->subject->name)) break; // only 
            while (metadata->subject = metadata->subject->next); /* Oooh funky indentation! */
    if (!metadata->subject) goto database_add_question_error; /* Error handling */

    // and now for something completely different
    for (int i=0;i<metadata->subject->num_categories;i++) metadata->categories[i] = 0; /* Could have used a memset, should have used one */
    tag_categories(question, metadata); /* Compute all the tags in it */
    int max_number = 0, max_index = -1; for (int i = 0; i < metadata->subject->num_categories; i++) /* Search for most prominent category */
        if (metadata->categories[i] > max_number) { /* If this exceeds the previous */
            max_number = metadata->categories[i]; max_index = i; /* Update the values */
        }
    metadata->main_category = max_index; /* Get the primary category */

    // Ok just add them on.
    if (last_question) { /* If this is not the last */
        last_question->next = metadata; /* There you go */
        last_question->question->next = question; /* Append the question onto this */
    } else { /* Just directly append */
        database->questions = question; /* Append the question */
        database->metadata = metadata; /* And finally we are done */
    }

    return 0;
database_add_question_error:
    delete_question_list(question); free(metadata); /* Do these */
    return 1;
}

int load_paper_into_database(QuestionDatabase *database, ParsedPaper *paper) {
    /* Go question by question and load it into the database */

    QuestionMetadata *last_question = database->metadata; PaperQuestion *this_question = paper->questions; /* For loading stuff */
    if (last_question) while (last_question->next) last_question = last_question->next; /* Get the last one */
    int errors = 0; /* Count errors */
    do { /* Loop through all the questions */
        errors += database_add_question(database, last_question, this_question, paper->subject); /* Add a new question in here */
        last_question = (last_question) ? last_question->next : database->metadata; /* Do this thing (obviously) */
    } while (this_question = this_question->next); /* Now our loop is done */
    
    return errors; /* Unless error we'll see later */
}
