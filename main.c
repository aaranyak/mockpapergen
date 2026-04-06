/* mockpapergen - This tool allows you to extract questions from past papers, store them, and generate papers using them */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include "system_commands.h"
#include "structures.h"
#include "load_pdf.h"
#include "question_database.h"
#include "gui.h"

int main(int argc, char **argv) {
    /* This is just a main function for executing the main thing */
    srandom(time(NULL)); /* Well, you have to do this */
    return launch_gui(argc, argv);
}
