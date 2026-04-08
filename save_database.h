/* Header file for save_database.c */
#ifndef SAVE_DATABASE_H
#define SAVE_DATABASE_H
int write_question(char **buffer, PaperQuestion *question, int length);
int read_database(QuestionDatabase *database, char *file_path);
int save_database(QuestionDatabase *database, char *file_path);
int read_question(char *file_data, int read_head, PaperQuestion *question, int file_length);
int read_textlist(char *file_data, int read_head, TextList *text_list, int file_length);
#endif
