#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <csse2310a1.h>

#define INVALID_COMMAND_LINE_ARGUMENT_4 \
    "Usage: uqwordladder [--start startWord] [--end destWord] " \
    "[--limit stepLimit] [--len length] [--dictionary dictfilename]"
#define EXIT_STATUS_4 4

#define MIN__WORD_LENGTH 2
#define MAX__WORD_LENGTH 9
#define DEFAULT_WORD_LENGTH 4
#define WORD_LENGTH_CONFLICT_12 \
    "uqwordladder: Word length conflict - lengths should be consistent"
#define EXIT_STATUS_12 12
#define WORD_LENGTH_OUTSIDE_RANGE_5 \
    "uqwordladder: Word lengths should be from 2 to 9 (inclusive)"
#define EXIT_STATUS_5 5

#define EXIT_GAME_WON 0
#define EXIT_GIVE_UP 8 
#define EXIT_ATTEMPTS_OVER 8 

typedef struct {
    char *startWord;
    char *destWord;
    char *dictionary;
    char **dictionaryElement; 
    int dictionaryLines;
    int len;
    int limit;
} cmdArgs;

void valid_integer(char *str)
{
    float digit;
    int stringLength = strlen(str);
    if (stringLength == 0) {
        fprintf(stderr, "%s\n",INVALID_COMMAND_LINE_ARGUMENT_4); 
        exit(EXIT_STATUS_4);
    }

    for (int i = 0; i < stringLength; i++) {
        if (isdigit(str[i]) == 0) {
            fprintf(stderr, "%s\n",INVALID_COMMAND_LINE_ARGUMENT_4); 
            exit(EXIT_STATUS_4);
        }
       
    }

    digit = atof(str);
    if (digit <= 0) {
        fprintf(stderr, "%s\n",INVALID_COMMAND_LINE_ARGUMENT_4); 
        exit(EXIT_STATUS_4);
    }

}

void command_line_arguments(int argc, char *argv[], cmdArgs *arguments) {
    arguments->startWord = NULL;
    arguments->destWord = NULL;
    arguments->dictionary = NULL;
    arguments->len = -1;
    arguments->limit = -1;

    bool startWordSupplied = false;
    bool destWordSupplied = false;
    bool lenSupplied = false;
    bool limitSupplied = false;
    bool dictSupplied = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--start") == 0) {
            if (!startWordSupplied && ++i < argc) {
                arguments->startWord = argv[i]; 
            } else {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }
            startWordSupplied = true;

        } else if (strcmp(argv[i], "--end") == 0) {
            if (!destWordSupplied && ++i < argc) {
                arguments->destWord = argv[i];
            } else {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }
            destWordSupplied = true; 

        } else if (strcmp(argv[i], "--len") == 0) {
            if (!lenSupplied && ++i < argc) {
                valid_integer(argv[i]);
                arguments->len = atoi(argv[i]);
            } else {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }
            lenSupplied = true;

        } else if (strcmp(argv[i], "--limit") == 0) {
            if (!limitSupplied && ++i < argc) {
                valid_integer(argv[i]);
                arguments->limit = atoi(argv[i]);
            } else {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }
            limitSupplied = true;

        } else if (strcmp(argv[i], "--dictionary") == 0) {
            if (!dictSupplied && ++i < argc) {
                arguments->dictionary = argv[i];
            } else {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }
            dictSupplied = true;

        } else {
            fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
            exit(EXIT_STATUS_4);
        }
    }
}

void word_length_valid(char *str) {
    if (strlen(str) < MIN__WORD_LENGTH || strlen(str) > MAX__WORD_LENGTH) {
        fprintf(stderr, "%s\n", WORD_LENGTH_OUTSIDE_RANGE_5);
        exit(EXIT_STATUS_5);
    }
}

void length_valid(int length) {
    if (length < MIN__WORD_LENGTH || length > MAX__WORD_LENGTH) {
        fprintf(stderr, "%s\n", WORD_LENGTH_OUTSIDE_RANGE_5);
        exit(EXIT_STATUS_5);
    }
}

void string_length_comparison(char *str, int length) {
    if (strlen(str) != length) {
        fprintf(stderr, "%s\n", WORD_LENGTH_CONFLICT_12);
        exit(EXIT_STATUS_12);
    }
}

void string_string_comparison(char *string1, char *string2) {
    if (strlen(string1) != strlen(string2)) {
        fprintf(stderr, "%s\n", WORD_LENGTH_CONFLICT_12);
        exit(EXIT_STATUS_12);
    }
}

void open_file(char *filename, char **file_pointer, int *numLines) {
    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "%s: File not found\n", filename);
        exit(EXIT_STATUS_4);
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    *numLines = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        *file_pointer = realloc(*file_pointer, (*numLines + 1) * sizeof(char *));
        (*file_pointer)[*numLines] = strdup(line);
        (*numLines)++;
    }
    free(line);
    fclose(file);
}

void line_start_end(char *line, char **start, char **end) {
    char *token;
    token = strtok(line, " \n");
    *start = strdup(token);
    token = strtok(NULL, " \n");
    *end = strdup(token);
}

void start_end_check(cmdArgs *arguments, int numLines, char **file_pointer) {
    bool startFound = false;
    bool endFound = false;

    for (int i = 0; i < numLines; i++) {
        char *line = strdup(file_pointer[i]);
        char *start;
        char *end;
        line_start_end(line, &start, &end);
        if (strcmp(start, arguments->startWord) == 0) {
            startFound = true;
        } else if (strcmp(start, arguments->destWord) == 0) {
            endFound = true;
        }
        free(line);
        free(start);
        free(end);
    }

    if (!startFound) {
        fprintf(stderr, "uqwordladder: Start word '%s' not in dictionary\n", arguments->startWord);
        exit(EXIT_STATUS_4);
    }

    if (!endFound) {
        fprintf(stderr, "uqwordladder: End word '%s' not in dictionary\n", arguments->destWord);
        exit(EXIT_STATUS_4);
    }
}

int main(int argc, char *argv[]) {
    cmdArgs arguments;
    command_line_arguments(argc, argv, &arguments);

    if (arguments.startWord == NULL || arguments.destWord == NULL || arguments.dictionary == NULL) {
        fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
        exit(EXIT_STATUS_4);
    }

    length_valid(arguments.len);

    open_file(arguments.dictionary, &arguments.dictionaryElement, &arguments.dictionaryLines);

    for (int i = 0; i < arguments.dictionaryLines; i++) {
        word_length_valid(arguments.dictionaryElement[i]);
        string_length_comparison(arguments.dictionaryElement[i], arguments.len);
    }

    string_string_comparison(arguments.startWord, arguments.destWord);

    start_end_check(&arguments, arguments.dictionaryLines, arguments.dictionaryElement);

    exit(EXIT_GAME_WON);
}
