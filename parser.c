/**
 * A string tokenization library.
 *
 * @author RR
 * @author Ryan Strauss
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define END_OF_STRING -1


/**
 * Handler in the event of a heap allocation error.
 */
void handle_heap_error() {
    perror("Error --- out of heap space. Exiting...\n");
    exit(-1);
}

/**
 * Returns the number of tokens in the supplied string.
 * @param cmd_line the string to be tokenized
 * @return The number of white-space separated tokens that were found in cmdLine.
 */
int count_tokens(const char *cmd_line) {
    if (strlen(cmd_line) == 0)
        return 0;

    int num_tokens;
    if (isspace(cmd_line[0]))
        num_tokens = 0;
    else
        num_tokens = 1;

    int i = 1;
    while (cmd_line[i] != '\0') {
        if ((!isspace(cmd_line[i])) && (isspace(cmd_line[i - 1])))
            num_tokens++;
        i++;
    }

    return num_tokens;
}

/**
 * Returns the next token that can be read parsed from the supplied string.
 * @param cmd_line the string to be tokenized
 * @param start the index position in cmdLine from which to start scanning
               for the next token
 * @return The ending index of the next token. Returns an END_OF_STRING special
       value in case no more tokens are left to read. Further, the value
       pointed to by the input parameter start may be modified in case some
       white space characters need to be consumed, before the start of the
       next token.
 */
int get_next_token(const char *cmd_line, int *start) {
    int i = *start;

    while ((cmd_line[i] != '\0') && (isspace(cmd_line[i]))) {
        i++;
    }

    if (cmd_line[i] == '\0') // no more tokens
        return END_OF_STRING;

    *start = i;
    while ((cmd_line[i] != '\0') && (!isspace(cmd_line[i])))
        i++;

    return i;
}

char **parse_command(const char *cmd_line, int *bg) {
    int num_tokens = count_tokens(cmd_line);
    *bg = 0;
    char **args = (char **) malloc(sizeof(char *) * (num_tokens + 1));
    if (!args)
        handle_heap_error();
    int start = 0;
    int end;
    int token_length;
    for (int i = 0; i < num_tokens; i++) {
        end = get_next_token(cmd_line, &start);
        token_length = end - start;
        args[i] = (char *) malloc(sizeof(char) * (token_length + 1));
        if (!args[i])
            handle_heap_error();
        strncpy(args[i], (cmd_line + start), token_length);
        args[i][token_length] = '\0';
        start = end;
    }

    if ((num_tokens > 0) && (strcmp(args[num_tokens - 1], "&") == 0)) {
        *bg = 1;
        args = realloc(args, sizeof(char *) * num_tokens);
        num_tokens--;
    } else if (num_tokens > 0) {
        int length = strlen(args[num_tokens - 1]);
        if (args[num_tokens - 1][length - 1] == '&') {
            *bg = 1;
            args[num_tokens - 1] = realloc(args[num_tokens - 1], sizeof(char) * length);
            args[num_tokens - 1][length - 1] = '\0';
        }
    }

    args[num_tokens] = NULL;

    return args;
}


