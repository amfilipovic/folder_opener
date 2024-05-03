#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// check if the code is being compiled on Windows
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
//Define the maximum path length that the program will handle
#define MAX_PATH_LENGTH 512

// Define ssize_t for Windows
typedef long long ssize_t;
#else
// Include unistd.h for ssize_t on Unix-like systems
#include <unistd.h>
#endif

// Define a structure to hold the messages
typedef struct
{
    char *message5;
    char *message6;
} LanguageMessages;

// Custom getline implementation
ssize_t custom_getline(char **lineptr, size_t *n, FILE *stream)
{
    size_t size = 0;
    if (*lineptr == NULL)
    {
        *lineptr = malloc(128);
        if (*lineptr == NULL)
            return -1;
        size = 128;
    }
    else
    {
        size = *n;
    }

    char ch;
    size_t i = 0;
    while ((ch = fgetc(stream)) != EOF)
    {
        if (i >= size - 1)
        {
            char *new_lineptr = realloc(*lineptr, size * 2);
            if (new_lineptr == NULL)
            {
                return -1;
            }
            *lineptr = new_lineptr;
            size *= 2;
        }
        (*lineptr)[i++] = ch;
        if (ch == '\n' || ch == '\r')
            break;
    }
    if (i == 0)
        return -1;

    (*lineptr)[i] = '\0';
    *n = size;
    return i;
}

// Predefine message placeholders
#define MESSAGE_COUNT 7
char *messages[MESSAGE_COUNT] = {NULL};

// Replace getline with custom_getline
#define getline custom_getline

// Read the language file and load messages
int load_language(const char *language_id, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error! Could not open language file.\n");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    int reading_correct_language = 0;

    while (getline(&line, &len, file) != -1)
    {
        line[strcspn(line, "\n\r")] = '\0';

        if (strncmp(line, "$language_id", 12) == 0)
        {
            reading_correct_language = strstr(line, language_id) != NULL;
        }

        if (reading_correct_language)
        {
            for (int i = 1; i <= MESSAGE_COUNT; i++)
            {
                char message_key[10];
                snprintf(message_key, sizeof(message_key), "$message%d", i);
                if (strncmp(line, message_key, strlen(message_key)) == 0)
                {
                    char *message_value = strchr(line, '\'') + 1;
                    char *end_quote = strrchr(message_value, '\'');
                    *end_quote = '\0';
                    free(messages[i - 1]);
                    messages[i - 1] = strdup(message_value);
                }
            }
        }
    }

    free(line);
    fclose(file);

    // Verify that all messages were loaded
    for (int i = 0; i < MESSAGE_COUNT; i++)
    {
        if (!messages[i])
        {
            printf("Error! Missing message %d for language %s.\n", i + 1, language_id);
            return 1;
        }
    }

    return 0;
}

// Check if the folder exists
int folder_exists(const char *folder)
{
    struct stat info;
    return stat(folder, &info) == 0 && (info.st_mode & S_IFDIR);
}

// Sanitize the input to prevent command injection
int sanitize_input(const char *input)
{
    while (*input)
    {
        if (*input == '&' || *input == '?' || *input == '*' || *input == '|' || *input == '<' || *input == '>' || *input == ';')
        {
            return 1;
        }
        input++;
    }
    return 0;
}

// Open the specified folder
void open_folder(const char *folder, int verbose)
{
    if (sanitize_input(folder))
    {
        if (verbose)
        {
            printf("%s\n", messages[0]);
        }
        return;
    }

    if (!folder_exists(folder))
    {
        if (verbose)
            printf("%s %s\n", messages[1], folder);
        return;
    }

#ifdef _WIN32
    SHELLEXECUTEINFO shExInfo = {0};
    shExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = NULL;
    shExInfo.lpVerb = "open";
    shExInfo.lpFile = folder;
    shExInfo.lpParameters = "";
    shExInfo.lpDirectory = NULL;
    shExInfo.nShow = SW_MAXIMIZE;
    shExInfo.hInstApp = NULL;

    if (!ShellExecuteEx(&shExInfo))
    {
        if (verbose)
            printf("%s %s\n", messages[2], folder);
    }
    else if (verbose)
    {
        printf("%s %s\n", messages[2], folder);
    }
#elif __APPLE__
    char *argv[] = {"/usr/bin/open", (char *)folder, NULL};
    if (fork() == 0)
    {
        execv(argv[0], argv);
        exit(1);
    }
#elif __linux__
    if (fork() == 0)
    {
        execlp("xdg-open", "xdg-open", folder, (char *)NULL);
        exit(1);
    }
#else
    if (verbose)
        printf("%s\n", messages[2]);
#endif
}

// Program entry point
int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    int verbose = 1;
    char *language_id = "EN";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-language") == 0 && i + 1 < argc)
        {
            language_id = argv[++i];
            break;
        }
    }

    if (load_language(language_id, "folder_opener.languages") != 0)
    {
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-silent") == 0)
        {
            verbose = 0;
        }
        else if (strcmp(argv[i], "-verbose") == 0)
        {
            verbose = 1;
        }
        else if (strcmp(argv[i], "-language") == 0 && i + 1 < argc)
        {
            i++;
        }
        else
        {
            printf("%s %s\n", messages[4], argv[i]);
            return 1;
        }
    }

    FILE *file = fopen("folder_opener.folders", "r");
    if (!file)
    {
        if (verbose)
            printf("%s\n", messages[5]);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    int any_folders = 0;
    while (getline(&line, &len, file) != -1)
    {
        line[strcspn(line, "\n\r")] = '\0';
        if (line[0] == '\0')
            continue;
        open_folder(line, verbose);
        any_folders = 1;
    }

    free(line);
    fclose(file);

    if (!any_folders && verbose)
    {
        printf("%s\n", messages[6]);
    }

    for (int i = 0; i < MESSAGE_COUNT; i++)
    {
        free(messages[i]);
    }

    return 0;
}
