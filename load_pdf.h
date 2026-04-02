/* header file for load_pdf.c */
#ifndef LOAD_PDF_H
#define LOAD_PDF_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "structures.h"
xmlDocPtr get_pdf_as_html(char *path);
ParsedPaper *parse_question_paper(htmlDocPtr html_file);
TextList *extract_page_text(xmlNode *page, int page_index);
int parse_paper_metadata(ParsedPaper *parsed_paper, xmlNode *document_body);
char *find_text_at_position(TextList *list_head, int pos_x, int pos_y, int margin_x, int margin_y);
TextList *text_list_merge_sort(TextList *text_list, int length);
TextList *get_paper_contents(xmlNode *document_body);
int is_number(char *string, int length);
int stop_question_text(TextList *thing);
int deal_with_subquestions(PaperQuestion *superquestion, TextList *question_text);
int parse_questions(ParsedPaper *paper, TextList *contents);
TextList *get_question_contents(PaperQuestion *question, TextList *iterator);
int second_order_subquestions(PaperQuestion *superquestion, TextList *question_text);
#define in_range(val, min, max) (((val) >= (min)) && ((val) <= (max))) /* Quick help to check if value is in range */
#endif
