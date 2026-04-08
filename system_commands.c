/* system_commands.c - for making the thing properly cross-platform */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system_commands.h"
#include "structures.h"

void path_convert_to_windows(char *path) {
    /* Converts a linux style file path to a windows style one */
    int length = strlen(path); /* The total length of the string */
    for (int i = 0; i < length; i++) { /* Loop through each character in the string */
        if (path[i] == '/') path[i] = '\\'; /* Convert any forward slash into a backslash */
    }
}

char *path_join(char *path_1, char *path_2) {
    /* Joins one path to another */
    int length = strlen(path_1) + strlen(path_2) + 1; /* Current length of the path */
#ifndef WINDOWS
    int add_slash = path_1[strlen(path_1) - 1] != '/'; /* Check if the slash is already there or needs to be added */
#else
    int add_slash = path_1[strlen(path_1) - 1] != '\\'; /* Check if the slash is already there on windows */
#endif
    if (add_slash) length += 1; /* Add a character for the slash */
    char *full_path = (char*)malloc(length); /* Create a buffer to hold the new path */
    strcpy(full_path, path_1); /* Add the first path */
    memcpy(add_slash ? full_path + strlen(path_1) + 1: full_path + strlen(path_1), path_2, strlen(path_2)); /* Add second path */
    if (add_slash) { /* If we need to add a slash */
#ifndef WINDOWS
        full_path[strlen(path_1)] = '/'; /* Add the slash if on linux */
#else
        full_path[strlen(path_1)] = '\\'; /* Add the slash if on windows */
#endif
    } full_path[length - 1] = 0; /* Add a null terminator */
    return full_path; /* Return the final string */
}

char *get_temp_folder_path(char *folder_name) {
    /* Returns a path of a file in the temporary folder */
#ifndef WINDOWS /* If we compile for linux */
    return path_join(TMP_FOLDER, folder_name); /* Just add the folder name to the temporary folder */
#else /* If we compile for windows */
    char *folder_name_windows = (char*)malloc(strlen(folder_name) + 1); /* Buffer for the folder name */
    strcpy(folder_name_windows, folder_name); /* Copy it in here */
    path_convert_to_windows(folder_name_windows); /* Convert the path to a windows path */
    char *temp_folder_path = TMP_FOLDER; /* Get the path to the temporary folder */
    char *full_path = path_join(temp_folder_path, folder_name_windows); /* Load temp folder */
    free(temp_folder_path); free(folder_name_windows); /* Free these, since they involves mallocing */
    return full_path; /* Return the full path */
#endif
}

char *get_file_name(char *path) {
    /* Returns the file name from a path */
    if (!path) return 0; /* Just in case error check */
    int name_index = 0, index = 0, path_length = strlen(path); /* Get the index of where the name begins */
#ifndef WINDOWS
    do if (path[index] == '/') name_index = index + 1; while (path[++index]); /* Find the correct index */
#else
    do if (path[index] == '\\') name_index = index + 1; while (path[++index]); /* Find the correct index */
#endif
    char *file_name = (char*)malloc(path_length - name_index + 1); /* Allocate memory for name */
    memcpy(file_name, path + name_index, path_length - name_index); file_name[path_length - name_index] = 0; /* Copy in the name */
    return file_name;
}

inline int convert_pdf_to_html(char *pdf_path, char *html_path) {
    /* Converts a pdf file to an html */
    char *command = (char*)malloc(strlen(pdf_path) + strlen(html_path) + strlen(MUTOOL_PATH) + 32); /* Allocate memory for the command */
    sprintf(command, "\"%s\" convert -o \"%s\" \"%s\"", MUTOOL_PATH, html_path, pdf_path); /* Set up the command */
    system(command); /* Run the command */
    free(command); /* Clean up */
    return 0;
}

inline int convert_html_to_pdf(char *html_path, char *pdf_path) {
    /* Converts a pdf file to an html */
    char *command = (char*)malloc(strlen(pdf_path) + strlen(html_path) + strlen(MUTOOL_PATH) + 32); /* Allocate memory for the command */
#ifndef WINDOWS /* Compiling for linux */
    sprintf(command, "google-chrome-stable --headless --no-sandbox --disable-gpu --print-to-pdf=\"%s\" \"%s\"", pdf_path, html_path); /* Set up the command */
#else /* Compiling for windows */
    sprintf(command, "start chrome --headless --no-sandbox --disable-gpu --print-to-pdf=\"%s\" \"%s\"", pdf_path, html_path); /* Set up the command */
#endif
    system(command); /* Run the command */
    free(command); /* Clean up */
    return 0;
}
