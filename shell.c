/**
 * This program implements a shell.
 *
 * @authors Ryan Strauss
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "parser.h"
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMD_LENGTH 1000
#define HISTORY_SIZE 10

typedef struct {
    char command[MAX_CMD_LENGTH];
    int commandID;
} history_t;

typedef struct {
    history_t *bufferStart;
    history_t *bufferEnd;
    history_t *validStart;
    history_t *validEnd;
    history_t buffer[HISTORY_SIZE];
    int size;
} circular_buffer;

circular_buffer history;

/**
 * Frees a 2D array from the heap.
 * @param args the array to be freed
 */
void freeArgs(char **args) {
    int index = 0;
    while (args[index] != NULL) {
        free(args[index]);
        index++;
    }
    free(args);
}

/**
 * Gets user input and stores in a string.
 * @param input the string where input will be stored
 */
void getUserInput(char input[MAX_CMD_LENGTH]) {
    char *returnValue = fgets(input, MAX_CMD_LENGTH, stdin);
    if (returnValue != input) {
        printf("Command can not be read.\n");
        fflush(stdout);
        exit(-2);
    }
}

/**
 * Executes the given command in the foreground.
 * @param args the arguments of the command to be executed
 */
void executeForeground(char **args) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("Child process could not be created.\n");
        fflush(stdout);
        exit(-2);
    }
    if (!pid) {
        if (execvp(args[0], args) == -1) {
            printf("Execution failed.\n");
            fflush(stdout);
            exit(-2);
        }
    } else {
        waitpid(pid, NULL, 0);
    }
}

/**
 * Executes the given command in the background.
 * @param args the arguments of the command to be executed
 */
void executeBackground(char **args) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("Child process could not be created.\n");
        fflush(stdout);
        exit(-2);
    }
    if (!pid) {
        if (execvp(args[0], args) == -1) {
            printf("Execution failed.\n");
            fflush(stdout);
            exit(-2);
        }
    }
}

/**
 * Handler to be used with SIGCHLD signals.
 * @param sig the signal
 */
void handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * Adds the given history_t to history.
 * @param item the item to be added
 */
void addToHistory(history_t item) {
    if (history.size == 0) {
        *history.validStart = item;
    } else if (history.size < HISTORY_SIZE) {
        history.validEnd++;
        *history.validEnd = item;
    } else {
        *history.validStart = item;
        history.validStart++;
        history.validEnd++;
        if (history.validStart > history.bufferEnd) {
            history.validStart = history.bufferStart;
        }
        if (history.validEnd > history.bufferEnd) {
            history.validEnd = history.bufferStart;
        }
    }
    if (history.size < 10) {
        history.size++;
    }
}

/**
 * Prints the current history to the console.
 */
void printHistory() {
    history_t *pos = history.validStart;
    for (int i = 0; i < history.size; ++i) {
        printf("%d %s", pos->commandID, pos->command);
        fflush(stdout);
        pos++;
        if (pos > history.bufferEnd) {
            pos = history.bufferStart;
        }
    }
}

/**
 * Executes the given command from the history.
 * @param commandID the ID of the command to execute
 * @param cmdID the current command ID
 */
void historyExpansion(int commandID, int *cmdID) {
    int background;
    for (int i = 0; i < history.size; ++i) {
        if (history.buffer[i].commandID == commandID) {
            char **args = parseCommand(history.buffer[i].command, &background);
            if (strcmp(history.buffer[i].command, "")) {
                history_t newItem;
                strcpy(newItem.command, history.buffer[i].command);
                newItem.commandID = *cmdID;
                addToHistory(newItem);
            }
            if (!strcmp(args[0], "exit")) {
                freeArgs(args);
                exit(0);
            } else if (!strcmp(args[0], "history")) {
                printHistory();
            } else if (!background) {
                executeForeground(args);
            } else {
                executeBackground(args);
            }
            freeArgs(args);
            (*cmdID)++;
            return;
        }
    }
    printf("Event not found.\n");
    fflush(stdout);
}

/**
 * Implements helper functions to run the shell.
 * @return 0 if program exits normally
 */
int main() {
    signal(SIGCHLD, handler);
    history.bufferStart = history.buffer;
    history.bufferEnd = history.bufferStart + HISTORY_SIZE - 1;
    history.validStart = history.bufferStart;
    history.validEnd = history.bufferStart;
    history.size = 0;
    int commandID = 1;
    while (1) {
        char input[MAX_CMD_LENGTH];
        int background;
        printf("catshell> ");
        fflush(stdout);
        getUserInput(input);
        char **args = parseCommand(input, &background);
        if (args[0] == NULL) {
            continue;
        }
        if (input[0] != '!') {
            history_t newItem;
            strcpy(newItem.command, input);
            newItem.commandID = commandID;
            addToHistory(newItem);
            commandID++;
        }
        if (!strcmp(args[0], "exit")) {
            freeArgs(args);
            return 0;
        } else if (!strcmp(args[0], "history")) {
            printHistory();
        } else if (input[0] == '!') {
            char id[5];
            strcpy(id, args[0] + 1);
            historyExpansion(atoi(id), &commandID);
        } else if (!background) {
            executeForeground(args);
        } else {
            executeBackground(args);
        }
        freeArgs(args);
    }
}