/* Header file for save_database.c */
#ifndef SAVE_DATABASE_H
#define SAVE_DATABASE_H
int write_question(char **buffer, PaperQuestion *question, int length);
int save_database(QuestionDatabase *database, char *file_path);
#endif
