/* load_pdf.c - Extracts question data from a paper in a pdf form */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "structures.h"
#include "load_pdf.h"
#include "system_commands.h"

void test_print_stuff(ParsedPaper *parsed_paper) {
    printf("Paper Metadata:\n    Subject: %s\n    Target Exam: %s\n    Name: %s\n    Time Limit: %d minutes\n", parsed_paper->subject, parsed_paper->target_exam, parsed_paper->name, parsed_paper->time); /* If you are still sane right now you are not working hard enough */
    PaperQuestion *current = parsed_paper->questions; /* Print all the superquestions one by one */
    for (printf("\n"); current; current = current->next) { /* Loop through all the supers */
        printf("Question %d: %s (%d marks)\n", current->index + 1, current->text, current->marks); /* Print the question */
        if (current->subquestions) for (PaperQuestion *subquestion = current->subquestions; subquestion; subquestion = subquestion->next) {
            printf("    Subquestion %d: %s (%d marks)\n", subquestion->index + 1, subquestion->text, subquestion->marks); /* SUBQUESTION */
            if (subquestion->subquestions) for (PaperQuestion *subsub = subquestion->subquestions; subsub; subsub = subsub->next)
                printf("        SubSubquestion %d: %s (%d marks)\n", subsub->index + 1, subsub->text, subsub->marks); /* META */
        }
    }
}

xmlDocPtr get_pdf_as_html(char *path) {
    /* Gets a pdf file and converts it to html */
    // First get the path of the html file
    char *html_name = get_file_name(path); /* Get the name of the file */
    html_name = (char*)realloc(html_name, strlen(html_name) + 2); /* Add space for writing "html" */
    memcpy(html_name + strlen(html_name) - 3, "html", 5); /* Correct the filename extension */
    char *html_path = get_temp_folder_path(html_name); /* Get a path in the /tmp directory */
    // Do the conversion
    convert_pdf_to_html(path, html_path); /* Run the conversion */

    // Load the html file
    xmlDocPtr html_file = htmlReadFile(html_path, 0, HTML_PARSE_NOBLANKS /* Parses an html file */
            | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET); /* Get the html file */

    // Clean up
    remove(html_path); /* Delete this file */
    free(html_name); free(html_path); /* Clean this stuff up */
    return html_file;
}

TextList *extract_page_text(xmlNode *page, int page_index) {
    /* Extracts the text from a page in the form of a textlist */
    
    xmlNode *current_element = page->children, *inner_span, *text_node; /* Children of the current page */
    TextList *last_text = 0, *current_text, *list_root; /* Use this for appending onto the text list */
    char *style_attribute; float pos_x, pos_y; int string_index, insert_index; char num_string[34];/* Computing some stuffs */
    
    while (current_element) { /* Loop through all the paragraphs (stuffs) in the page */
        if (strcmp(current_element->name, "p")) { /* Check if this is a <p> */
            current_element = current_element->next; /* MOOVE ON!! */
            continue; /* Search only for <p> */
        }
        inner_span = current_element->children; /* The <span> inside the <p> */   
        text_node = inner_span->children; /* The <span> contains some text in a text node */
        style_attribute = xmlGetProp(current_element, "style"); /* Get the style attribute of this node */
        
        // Parse style
        string_index = 0; insert_index = 0;/* For parsing string */
        while (style_attribute[string_index] != ':') string_index++; string_index++;/* Move something to something else and whatever */
        while (style_attribute[string_index] != 'p') num_string[insert_index++] = style_attribute[string_index++]; /* Copy the number */
        num_string[insert_index] = 0; /* Null terminator */ insert_index = 0; /* Reset for next one */
        sscanf(num_string, "%f", &pos_y); /* Read in the number from the string */
        while (style_attribute[string_index] != ':') string_index++; string_index++; /* Move something to something else and whatever */
        while (style_attribute[string_index] != 'p') num_string[insert_index++] = style_attribute[string_index++]; /* Copy the number */
        num_string[insert_index] = 0; /* Null terminator */ insert_index = 0; /* Reset for next one */
        sscanf(num_string, "%f", &pos_x); /* Read in the number from the string */

        current_text = append_text_list(last_text, text_node->content, (int)pos_x, (int)pos_y, page_index); /* Please help I am not sane right now */
        if (!last_text) list_root = current_text; /* this line does something I know what I'm doing please don't doubt that */
        last_text = current_text; /* Now the next element */
        current_element = current_element->next; /* MOOVE ON!! */
    }
    // Ok now we are done 
    return list_root; /* Return the head node of the linked list with all the other list items in tow */
}

char *find_text_at_position(TextList *list_head, int pos_x, int pos_y, int margin_x, int margin_y) {
    /* Finds the text element located around a certain point from a text list */
    for (TextList *current_text = list_head; current_text; current_text = current_text->next) /* Foreach loop for linked lists */
        if (in_range(current_text->pos_x, pos_x - margin_x, pos_x + margin_x) && in_range(current_text->pos_y, pos_y - margin_y, pos_y + margin_y))
            return current_text->text; /* If the thing is around that point return it */
    return 0;
}

int parse_paper_metadata(ParsedPaper *parsed_paper, xmlNode *document_body) {
    /* Parses the question paper's metadata into the parsed paper struct */
    
    // Get the first page
    xmlNode *first_page = document_body->children; /* Get the first pointer to a linked list */
    while (strcmp(first_page->name, "div")) first_page = first_page->next; /* Get it by looping through the <html>'s children */
    
    TextList *page_contents = 0; /* This will make freeing stuff easier later */
    if (!first_page) goto parse_paper_metadata_error; /* If this doesn't work error error error !!! */

    // Now time to start parsing the stuffs in it.
    page_contents = extract_page_text(first_page, 0); /* Extract all the text from the page */

    char *paper_subject = find_text_at_position(page_contents, 56, 235, 10, 10); /* Get the subject of the paper */
    char *target_exam = find_text_at_position(page_contents, 468, 256, 70, 10); /* Get the target exam of the paper */
    char *paper_name = find_text_at_position(page_contents, 56, 256, 10, 10); /* This is the name of the paper */ 
    char *time_text = find_text_at_position(page_contents, 453, 276, 10, 10); /* The time the paper should be done in (convert to min later) */
    if (!(paper_subject && target_exam && paper_name && time_text)) goto parse_paper_metadata_error; /* Error check for this */ 

    // Time to parse the time data
    char number[8], unit[16]; int write_index = 0, read_index = 0, read_num = 0, minutes = 0; /* Some string parsing nessecities */
    while (time_text[read_index] != ' ') number[write_index++] = time_text[read_index++]; /* Read in the first number */
    read_index++; number[write_index] = 0; write_index = 0; sscanf(number, "%d", &read_num); /* Read in the number as an int */
    while (time_text[read_index] != ' ') unit[write_index++] = time_text[read_index++]; /* Read in the unit */
    read_index++; unit[write_index] = 0; write_index = 0; /* I instinctively feel the need to put this line here and IDK why (jk ofcourse) */
    if (!strcmp(unit, "hour") || !strcmp(unit, "hours")) { /* We are not yet done with this */
        minutes += read_num * 60; /* First add the stuff */
        if (read_index != strlen(time_text)) {
            while (time_text[read_index] != ' ') number[write_index++] = time_text[read_index++]; /* Read in the first number */
            read_index++; number[write_index] = 0; write_index = 0; sscanf(number, "%d", &read_num); /* Read in the number as an int */
            minutes += read_num; /* Now we add the rest of the minutes */
        }
    }

    // Time to set the datas
    strcpy(parsed_paper->subject, paper_subject); /* Copy in the subject */
    strcpy(parsed_paper->target_exam, target_exam); /* Copy in the target_exam */
    strcpy(parsed_paper->name, paper_name); /* Copy in the name */
    parsed_paper->time = minutes; /* Set the minutes */

    // Clean up
    delete_text_list(page_contents); /* Here ! */
    return 0; /* Success */

    // Error handling
parse_paper_metadata_error:
    if (page_contents) delete_text_list(page_contents); /* Clean up if necessary */
    return 1;
}

TextList *text_list_merge_sort(TextList *text_list, int length) {
    /* Sorts a linked list text_list using MERGE SORT */
    if (length == 1) { /* If this is of length 1 */
        text_list->next = 0; /* not having this might have been the thing causing all the problems */
        return text_list; /* Bottom of the merge sort tree */
    }

    // First do the splitting of leests
    int halflength = length >> 1; /* Just optimise this */ int index = 0; TextList *second_half = text_list; /* To prevent cursed crashes */
    while (index++ < halflength) second_half = second_half->next; /* Get the middle element */
    TextList *list_1 = text_list_merge_sort(text_list, halflength); /* The first half of the text list */
    TextList *list_2 = text_list_merge_sort(second_half, length - halflength); /* The second half */ 
    index = -1; TextList *last_node = 0; while (++index < length) { /* Time to do the merge step (ooohh this sounds like fun) */
        if (!list_2 || list_1 && list_1->sort_score <= list_2->sort_score) { /* If we should add from list 1 */
            if (!last_node) { /* If the list is empty */
                text_list = list_1; last_node = text_list; /* Create the root node */
            } else { /* If this is not the first */
                last_node->next = list_1; last_node = last_node->next; /* Append this element onto the list, push the last pointer */
            } list_1 = list_1->next; /* Push the list pointer ahead */
        } else { /* Wow dealing with singly-linked-lists is weird but also strangely satisfying and also requires less code */
            if (!last_node) { /* If the list is empty */
                text_list = list_2; last_node = text_list; /* Create the root node */
            } else { /* If this is not the first */
                last_node->next = list_2; last_node = last_node->next; /* append this element onto the list, push the last pointer */
            } list_2 = list_2->next; /* Push the list-pointer ahead */
        }
    }
    last_node->next = 0; /* This is important since the function is recursive */
    return text_list; /* Unlike a regular merge sort, since this is a linked list I needn't duplicate any of the stuff inside */
} // I'm still a little unsure of this non-duplicating idea, and really hope recursion doesn't break it in some unfathomable way. 

TextList *get_paper_contents(xmlNode *document_body) {
    /* Returns a list of paper contents sorted by merge sort */
    
    TextList *root_node, *last_list = 0, *current_list; /* For generating text list */
    xmlNode *current_page = document_body->children; int index = -1; /* Current page, as it may seem */
    do { /* It is what it is, don't ask more questions if you enjoy being in one piece */
        if (strcmp(current_page->name, "div")) continue; /* Only the divs */
        if (++index == 0) continue; /* Check if this is the first page it can't be */
        // This is a true and honest questions page
        current_list = extract_page_text(current_page, index); /* We have the current linked list */
        if (last_list) last_list->next = current_list; /* BEE HOLD (not safe without beekeeping equipment) */
        else root_node = current_list; /* This is the first one so set it to the root node */
        last_list = current_list; while (last_list->next) last_list = last_list->next; /* Get tail node for next append */
    } while (current_page = current_page->next); /* This thing basically extracts text from each page and appends it into one huge linked list */

    // Time to SORT that linked list!!!
    int list_length = 0; current_list = root_node; do list_length++; while (current_list = current_list->next); /* Get length of the leenkd leest */ 
    for (current_list = root_node; current_list; current_list = current_list->next) /* Loop through all the nodes in the list */
        current_list->sort_score = current_list->page_index*800*500 + current_list->pos_y*500 + current_list->pos_x; /* Order to sort in */
    root_node = text_list_merge_sort(root_node, list_length); /* Sort this list */
   
    return root_node; /* Now we have all the paper's text in order */
}

inline int is_number(char *string, int length) {
    /* Checks if a section of a string is a number */
    for (int i = 0; i < length; i++) if (!isdigit(string[i]) && string[i] != ' ') return 0; /* Check if this is a number */
    if (!strcmp(string, " ")) return 0; /* Ignore spaces */
    return 1; /* FINALLY */
}

#define STOP_VALUE_NEXTER 14

inline int stop_question_text(TextList *thing) {
    /* Checks if this is normal text or some kind of key */
    int len = strlen(thing->text); /* Sorry but kinda important */
    if (in_range(strstr(thing->text, "(a)") - thing->text, 0, 2) || in_range(strstr(thing->text, "(i)") - thing->text, 0, 2)) return 1; /* This is a subquestion */
//    if (is_number(thing->text, len) && thing->pos_x < 95) return 2; /* This is the next question (this breaks a bunch of stuff) */
    if (strstr(thing->text,".........")) return 3; /* This is a working space */
    if (in_range(strstr(thing->text, "A") - thing->text, 0, 2) && len < 3) return 4; /* Behold, multiple choice question */
    if (strstr(thing->text, "Complete the table") == thing->text) return 5; /* This is a table question */ 
    if (strstr(thing->text, "Use the terms from the list") == thing->text) return 6; /* This is a fill in the blanks */ 
    if (strstr(thing->text, "Draw one or more lines") == thing->text) return 7; /* This is a fill in the blanks */
    if (strstr(thing->text, "01") == thing->text) return 9; /* This is a code snuppet (that's a real word!!!!!, I swear it is!!!!!!) */
    if (strstr(thing->text, "•") == thing->text) return 10; /* This is a list */
    if (strstr(thing->text, "Working space")) return 11; /* Working space */
    if (thing->text[len - 1] == ']' && (thing->text[len - 3] == '[' || thing->text[len - 4] == '[')) return 12; /* marks thing */
    if (strstr(thing->text, "from the following list of words")) return STOP_VALUE_NEXTER; /* These will be also read now */
    if (strstr(thing->text, "Consider the logic circuit") == thing->text) return STOP_VALUE_NEXTER + 1; /* SEE DYNAMIC */
    if (strstr(thing->text, "Complete the ")) return STOP_VALUE_NEXTER + 2;
    if (strstr(thing->text, "table is given")) return STOP_VALUE_NEXTER + 3;
    if (strstr(thing->text, "following table")) return STOP_VALUE_NEXTER + 4;
    if (strstr(thing->text, " are shown")) return STOP_VALUE_NEXTER + 5;
    if (strstr(thing->text, "Draw ")) return STOP_VALUE_NEXTER + 5;
    return 0; /* This is not a stop text */
}

int check_contents_end(TextList *thing) {
    /* Checks if this is the end of a question contents */
    int len = strlen(thing->text); /* saves typing and runtime */
    if (thing->text[len - 1] == ']' && (thing->text[len - 3] == '[' || thing->text[len - 4] == '[')) return 1; /* marks thing */
    if (in_range(strstr(thing->text, "(a)") - thing->text, 0, 2) || in_range(strstr(thing->text, "(i)") - thing->text, 0, 2)) return 2; /* This is a subquestion */
    return 0;
}

TextList *get_question_contents(PaperQuestion *question, TextList *iterator) {
    /* Get the contents of the question */
    TextList *tl_head = 0, *tl_previous = 0, *tl_current = 0;
    do { /* From here one just keep moving forward */
        /* One by one until you reach the marks */
        if (iterator->text[0] == 0) continue; /* Max safety check for null textboxes */
        if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* This is useless ignore it */
        if (check_contents_end(iterator)) break; /* marks thing */
        tl_current = copy_text_list(iterator); /* Copy the textlist */
        if (!tl_head) tl_head = tl_current; /* Set the list head */
        else tl_previous->next = tl_current;
        tl_previous = tl_current; /* This should fix it */
    } while (iterator = iterator->next); /* I really like this while loop format */
    question->contents = tl_head; /* Append the contents list */
    return iterator; /* Return this now */
}

int second_order_subquestions(PaperQuestion *superquestion, TextList *question_text) {
    /* Deal with subquestions inside subquestions (ooh isn't that soo meta) */

    // [the programmer reflects for a moment, wishing their past self had written a general recursive function for this]
    // [the programmer sighs and temporarily revises their principles on copy-pasting large codeblocks]

    
    char roman_nums[10][8] = {"(i)", "(ii)", "(iii)", "(iv)", "(v)", "(vi)", "(vii)", "(viii)", "(ix)", "(x)"}; /* Ok so I was doing a bunch of googling to see if I could find some roman numeral library for C then I realised that I could simply hardcode it */ 
    char *matchtext = roman_nums[0], *important_info; /* for matching the subquestion number */
    int index = 0; /* Also important for counting */
    PaperQuestion *subquestions = 0, *question, *last_sub = 0; /* List of questions */
    int super_marks = 0; /* Marks of the superquestion */

    do { /* This is kinda what I do for the main questions but subquestions thistime */
        important_info = strstr(question_text->text, matchtext); /* Get the strstr point */
        if (is_number(question_text->text, (question_text->text[1] == 0) ? 1 : 2) && question_text->pos_x < 55) break; /* Just to make this a bit more stable */
        if (!in_range(important_info - question_text->text, 0, 2)) continue; /* Make sure this is a question start */
        int text_count = 0, text_length = 0, count = 0, stop_value = 0, write_index = 0; /* This is actually important somehow */
        text_length += strlen(question_text->text + strlen(matchtext) + 1); /* Add it in the pre-malloc length calc */
        TextList *iterator = question_text; /* Start from here */
        // Time to get all the text in the subquestion
        if (!stop_question_text(iterator)) while (iterator = iterator->next) { /* Start with a skip */
            stop_value = stop_question_text(iterator); /* Check if to stop now */
            if (stop_value && stop_value < STOP_VALUE_NEXTER) break; /* As you should expect to */
            count++; /* As YOU should EXPECT to */
            if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* This is a the that thing */
            text_count++; /* The real one that is being counted */
            text_length += strlen(iterator->text); /* Add in the length of this */
            if (stop_value) break; /* Double check */
        } iterator = question_text; /* Get back */
        char *content = (char*)malloc(text_length + text_count + 2); /* Allocate memory for this */
        memcpy(content, question_text->text + strlen(matchtext) + 1, strlen(question_text->text + strlen(matchtext) + 1)); /* This is the slightly faster way to do this */
        write_index += strlen(question_text->text + strlen(matchtext) + 2) + 1; content[write_index] = ' '; write_index++; /* inc wi */
        for (iterator = iterator->next; count--; iterator = iterator->next) { /* Now we gotto do this all over again */
            if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* Definitely more optimised than a strcmp */
            text_length = strlen(iterator->text); /* Length of text */
            memcpy(content + write_index, iterator->text, text_length); /* Copy in the new text */
            content[write_index + text_length] = ' '; /* Later we've to figure out which ones are a newline */
            write_index += text_length + 1; /* Inc write index */
        }
        content[write_index] = 0; /* I can't believe you forgot that */
        question = (PaperQuestion*)malloc(sizeof(PaperQuestion)); /* Allocate memory for subquestion */
        question->index = index++; /* Add in a new subquestion */
        question->depth = 2; /* This is the second lowest order of question */
        question->contents = 0; /* Set the contents of the question to 0 */
        question->text = content; /* Put the text that we've spend so much while loop to read */
        question->subquestions = 0; question->next = 0; /* Set up the list stuffs */
       
        // Get question details and marks
        TextList *tl_current = get_question_contents(question, iterator); /* This thing is reusable now */
        // Read no. of marks
        char converttoint[20]; int currentlen = strlen(tl_current->text); /* Important stuffs */;
        int startbox = (tl_current->text[currentlen - 3] == '[') ? currentlen - 2 : currentlen - 3; /* Check for num digits */
        memcpy(converttoint, tl_current->text + startbox, currentlen - startbox - 1); /* Length of whatever */
        converttoint[currentlen - startbox - 1] = 0; /* Add null terminator */
        sscanf(converttoint, "%d", &question->marks); /* I can't do the bee hold joke now although I really liked it */

        // Add this to the super
        super_marks += question->marks; /* Add this to the super */
        if (!last_sub) subquestions = question; /* Add it if list empty */
        else last_sub->next = question; /* else append it */
        last_sub = question; /* Otherwise... */
        matchtext += 8; /* Update matchtext (using some simple pointer arithmetic) */
    } while (question_text = question_text->next); /* Love doing do while loops like this */
    superquestion->marks = super_marks; /* Set this in the super */
    superquestion->subquestions = subquestions; /* Add the list */
    return 0;
}

int deal_with_subquestions(PaperQuestion *superquestion, TextList *question_text) {
    /* If you can't understand this by the function name then I'm really bad at naming things */
    char matchtext[4] = "(a)", *important_info; /* This is for matching the subquestion number */
    PaperQuestion *subquestions = 0, *question, *last_sub = 0; /* List of questions */
    int index = 0; /* Also important for counting */
    int super_marks = 0; /* Marks of the superquestion */
    do { /* This is kinda what I do for the main questions but subquestions thistime */
        important_info = strstr(question_text->text, matchtext); /* Get the strstr point */
        if (is_number(question_text->text, (question_text->text[1] == 0) ? 1 : 2) && question_text->pos_x < 55) break; /* Just to make this a bit more stable */
        if (!in_range(important_info - question_text->text, 0, 2)) continue; /* Make sure this is a question start */
        if (in_range(strstr(important_info + 4, "(i)") - important_info - 4, 0, 2)) { /* Skip badly designed questions */
            matchtext[1]++; continue; /* Add another to the matchtext and skip */
        }
        int text_count = 0, text_length = 0, count = 0, stop_value = 0, write_index = 0; /* This is actually important somehow */
        text_length += strlen(question_text->text + 4); /* Add it in the pre-malloc length calc */
        TextList *iterator = question_text; /* Start from here */
        // Time to get all the text in the subquestion
        if (!stop_question_text(iterator)) while (iterator = iterator->next) { /* Start with a skip */
            stop_value = stop_question_text(iterator); /* Check if to stop now */
            if (stop_value && stop_value < STOP_VALUE_NEXTER) break; /* As you should expect to */
            count++; /* As YOU should EXPECT to */
            if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* This is a the that thing */
            text_count++; /* The real one that is being counted */
            text_length += strlen(iterator->text); /* Add in the length of this */
            if (stop_value) break; /* Double check */
        } iterator = question_text; /* Get back */
        char *content = (char*)malloc(text_length + text_count + 2); /* Allocate memory for this */
        memcpy(content, question_text->text + 4, strlen(question_text->text + 4)); /* This is the slightly faster way to do this */
        write_index += strlen(question_text->text + 4); content[write_index] = ' '; write_index++; /* Inc the write index */
        for (iterator = iterator->next; count--; iterator = iterator->next) { /* Now we gotto do this all over again */
            if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* Definitely more optimised than a strcmp */
            text_length = strlen(iterator->text); /* Length of text */
            memcpy(content + write_index, iterator->text, text_length); /* Copy in the new text */
            content[write_index + text_length] = ' '; /* Later we've to figure out which ones are a newline */
            write_index += text_length + 1; /* Inc write index */
        }
        content[write_index] = 0; /* I can't believe you forgot that */
        question = (PaperQuestion*)malloc(sizeof(PaperQuestion)); /* Allocate memory for subquestion */
        question->index = index++; /* Add in a new subquestion */
        question->depth = 2; /* This is the second lowest order of question */
        question->contents = 0; /* Set the contents of the question to 0 */
        question->text = content; /* Put the text that we've spend so much while loop to read */
        question->subquestions = 0; question->next = 0; /* Set up the list stuffs */
       
        // Get question details and marks
        
        if (stop_value != 1) { /* if contain you don't subquestions */        
            TextList *tl_current = get_question_contents(question, iterator); /* This thing is reusable now */

            // Read no. of marks
            question->marks = 0; /* Clear this thing as of now */
            if (check_contents_end(tl_current) == 1) {
                char converttoint[20]; int currentlen = strlen(tl_current->text); /* Important stuffs */;
                int startbox = (tl_current->text[currentlen - 3] == '[') ? currentlen - 2 : currentlen - 3; /* Check for num digits */
                memcpy(converttoint, tl_current->text + startbox, currentlen - startbox - 1); /* Length of whatever */
                converttoint[currentlen - startbox - 1] = 0; /* Add null terminator */
                sscanf(converttoint, "%d", &question->marks); /* I can't do the bee hold joke now although I really liked it */
            } else second_order_subquestions(question, iterator); /* See if this has any higher order subquestions if so parse them */
        } else question->marks = 0; /* To make it a little more normal */
        
        // Get the subsubquestions
        if (stop_value == 1)  /* See if this has any subquestions */
            second_order_subquestions(question, iterator); /* See if this has any higher order subquestions if so parse them */
        super_marks += question->marks; /* Add this to the super */
        if (!last_sub) subquestions = question; /* Add it if list empty */
        else last_sub->next = question; /* else append it */
        last_sub = question; /* Otherwise... */

        matchtext[1]++; /* Inc search index */
    } while (question_text = question_text->next); /* Love doing do while loops like this */
    superquestion->marks = super_marks; /* Set this in the super */
    superquestion->subquestions = subquestions; /* Add the list */
    
    return 0;
}

int parse_questions(ParsedPaper *paper, TextList *contents) {
    /* Extracts the questions one by one from the paper */

    // Ok this will be a difficult one how do we pull this off?
    // First create a something that takes all the headings of main questions
    TextList *current_super = contents; /* We shall be searching for the SUPERQUESTIONS */
    PaperQuestion *superquestions = 0, *superquestion, *last_super = 0; /* List of questions */
    int index = 0; /* Count the questions for some reason. I added the index val in the struct so I guess we have to calculate it now */
    if (current_super) do { /* This way it will be much easier to do continue statements: basically a normal while loop */
        if (!is_number(current_super->text, (current_super->text[1] == 0) ? 1 : 2) || current_super->pos_x > 55) continue; /* Until you've found a superquestion */
        TextList *iterator = current_super; int text_length = 0, count = 0, text_count = 0, stop_value = 0; /* Count number of texts */
        int checklater = strlen(current_super->text) > 5; /* Doublecheck this */
        if (checklater) text_length += strlen(strstr(current_super->text + 1, " ") + 1) + 1; /* Add the front part of the question */
        if (!stop_question_text(iterator)) while (iterator = iterator->next) { /* Loop through the next few stuffs */
            stop_value = stop_question_text(iterator); /* Check if we need to stop */
            if (stop_value && stop_value < STOP_VALUE_NEXTER) break; /* This means that we have reached the end of the question */
            count++; /* Count the text counts blah blah blah !!! */
            if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* This is a the that thing */
            text_count++; /* A little extra work to not have to deal with weird spacing stuffs */
            text_length += strlen(iterator->text); /* Add to the text length */
            if (stop_value) break; /* And break when necessary */
        } iterator = current_super; /* Iterator is SUPER now!!! yayyy!!!! */
        char *question_text = (char*)malloc(text_length + text_count + 2); int write_index = 0; /* Create question text buffer */
        if (checklater) { /* Add the stuff from the text in */
            memcpy(question_text, strstr(current_super->text + 1, " ") + 1, strlen(strstr(current_super->text + 1, " ") + 1)); /* Copy */
            write_index += strlen(strstr(current_super->text + 1, " ") + 1); question_text[write_index] = ' '; write_index++; /* Inc This */
        }
        for (iterator = iterator->next; count--; iterator = iterator->next) { /* Gotto do that loop again */
            if (iterator->text[0] == ' ' && iterator->text[1] == 0) continue; /* More optimised than a strcmp */
            text_length = strlen(iterator->text); /* Length of text */
            memcpy(question_text + write_index, iterator->text, text_length); /* Copy in the new text */
            question_text[write_index + text_length] = ' '; /* Later we've to figure out which ones are a newline */
            write_index += text_length + 1; /* Inc write index */
        }
        question_text[write_index] = 0; /* I can't believe you forgot that */
        superquestion = (PaperQuestion*)malloc(sizeof(PaperQuestion)); /* Create a super object */
        // [this line is intentionally left blank]
        superquestion->index = index++; /* Add in a new superquestion */
        superquestion->depth = 1; /* This is the lowest order of question */
        superquestion->contents = 0; /* Set the contents of the question to 0 */
        superquestion->text = question_text; /* Put the text that we've spend so much while loop to read */
        superquestion->subquestions = 0; superquestion->next = 0; /* Set up the list stuffs */
        
        // Now you'd need to read the rest of the textlist onto the paperquestion (YOU CAN DO IT! BE POSITIVE! YAYYYY!)
        if (stop_value != 1) { /* If this is not a superquestion */        
            TextList *tl_current = get_question_contents(superquestion, iterator); /* Package it into it's own function */

            // Read no. of marks
            superquestion->marks = 0; /* Clear this thing as of now */
            if (check_contents_end(tl_current) == 1) {
                char converttoint[20]; int currentlen = strlen(tl_current->text); /* Important stuffs */;
                int startbox = (tl_current->text[currentlen - 3] == '[') ? currentlen - 2 : currentlen - 3; /* Check for num digits */
                memcpy(converttoint, tl_current->text + startbox, currentlen - startbox - 1); /* Length of whatever */
                converttoint[currentlen - startbox - 1] = 0; /* Add null terminator */
                sscanf(converttoint, "%d", &superquestion->marks); /* I can't do the bee hold joke now although I really liked it */
            } else deal_with_subquestions(superquestion, iterator); /* When in doubt, just call a function that doesn't exist yet */
        } else superquestion->marks = 0; /* Just to make sure it is not weird */
        // Deal with subquestions now 
        if (stop_value == 1) /* Subquestions. I have to deal with subquestions. See how difficult this "seemingly" simple task is? */
            deal_with_subquestions(superquestion, iterator); /* When in doubt, just call a function that doesn't exist yet */
        // Append it to the list
        if (!superquestions) superquestions = superquestion; /* If this be the first, there you go */
        else last_super->next = superquestion; /* Another you go */
        last_super = superquestion; /* Good this is good */
    } while (current_super = current_super->next); /* This makes sure the increment happens */
    paper->questions = superquestions; /* spooderman, spooderman, he can do what a spooder can */

    return 0;
}

ParsedPaper *parse_question_paper(xmlDocPtr html_file) {
    /* Parses a question paper's xml tree and returns the file */

    int error = 0; /* universal error handling gadget */
    xmlNode *document_root = xmlDocGetRootElement(html_file); /* Get the root node of the html tree */
    xmlNode *document_body = document_root->children; /* This is the <body> element of the html file */
    while (strcmp(document_body->name, "body")) document_body = document_body->next; /* Get it by looping through the <html>'s children */

    ParsedPaper *parsed_paper = (ParsedPaper*)malloc(sizeof(ParsedPaper)); /* Create the object containing a parsed paper */
    if (strcmp(document_body->name, "body")) goto parse_question_paper_error;

    // Parse metadata (This section will need to be updated if discrepancies are found)
    error = parse_paper_metadata(parsed_paper, document_body); /* Parse the metadata on the first page */
    if (error) goto parse_question_paper_error; /* I USED A GOTO YOU ANTI GOTO GANG CAN'T STOP ME HAHAHAHA */
    // Clean up

    // Get all the paper text in order
    TextList *paper_text = get_paper_contents(document_body); /* Get the text of the paper in order */
    
    // Time to extract questions from this list.
    parse_questions(parsed_paper, paper_text); /* Pray that this do not segfault */

    delete_text_list(paper_text);

    return parsed_paper; /* Behold, the mighty parsed paper */

    // Deal with unexpected circumstances where things did not go as you wanted them to.

parse_question_paper_error: /* For people reading this from an enlightened future, this is a goto label. */
                            // Us primitive unenlightened hand-coders used to use this to jump to different 
                            // parts of the code without calling a proper subroutine. I know this seems horribly 
                            // disgusting but well, you're the one who's still dependent on this ancient outdated
                            // thing so it's basically your fault you have to deal with this.
    free(parsed_paper); /* Replace this with a proper delete command later */
    return 0; /* Error handling! */
}
