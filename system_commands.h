/* Header file for system_commands.c */
#ifndef SYSTEMCOMMANDS_H
#define SYSTEMCOMMANDS_H
#undef WINDOWS /* This controls whether we compile for windows or linux */
#define DEBUG

// All functions defined below
void path_convert_to_windows(char *path); /* Replaces all "/"s with "\"s */
char *path_join(char *path_1, char *path_2); /* Join two paths together */
char *get_temp_folder_path(char *folder_name);
char *get_file_name(char *path);
char *get_data_file_path(char *file_name);

// Inline functions
int convert_pdf_to_html(char *pdf_path, char *html_path);
int convert_html_to_pdf(char *html_path, char *pdf_path);

// Temporary Folder
#ifndef WINDOWS /* Compiling for linux */
    #define TMP_FOLDER "/tmp/" /* I think this is universal for most all systems */
#else /* Compiling on windows */
    #define TMP_FOLDER  (path_join(getenv("USERPROFILE"), "AppData\\Local\\Temp\\")) /* I dislike it how windows makes it all so cryptic */
#endif
#ifdef DEBUG
    #define DATA_FOLDER "/home/aaranyak/school_dp_1/computer_science/ia/data_folder"
#else
    #ifndef WINDOWS /* Compiling for linux */
        #define DATA_FOLDER "/usr/share/mockpapergen/"
    #else 
        #define DATA_FOLDER  (path_join(getenv("USERPROFILE"), "AppData\\Local\\mockpapergen\\")) /* I dislike it how windows makes it all so cryptic */
    #endif
#endif

#endif

// Here, port the random function for windows
#ifdef WINDOWS /* Only on windows */
    #include <glib.h>
    #include<windows.h>
    #define random() (g_random_int()) /* Here is the random function, since windows doesn't like the original one */
    #define srandom(x) (g_random_set_seed((x))) /* And the one to init the seed */
#endif
