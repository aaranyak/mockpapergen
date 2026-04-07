/* Header file for generate_paper.c */
#ifndef GENERATE_PAPER_H
#define GENERATE_PAPER_H
int get_questions_for_paper(QuestionMetadata *question_list[128], QuestionDatabase *database, SubjectInfo *subject, int num_categories, int *categories, int marks);
int generate_paper(QuestionDatabase *database, SubjectInfo *subject, int num_categories, int *categories, int marks, char *file_path);
int write_question_html(xmlNode *document_body, PaperQuestion *question, int index, int depth);

#endif
