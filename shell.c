/**
 * This program implements a shell.
 *
 * The shell features the ability to run commands in the background and
 * keeps a history of previous commands.
 *
 * @author Ryan Strauss
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"

#define MAX_CMD_LENGTH 1000
#define HISTORY_SIZE 10

typedef struct {
    char command[MAX_CMD_LENGTH];
    unsigned int command_id;
} record_t;

typedef struct {
    record_t buffer[HISTORY_SIZE];
    unsigned int next_id, next_write;
} history_t;

history_t history;

/**
 * Resets the history buffer.
 */
void history_reset() {
    history.next_write = 0;
    history.next_id = 1;
    for (int i = 0; i < HISTORY_SIZE; ++i)
        history.buffer[i].command_id = 0;
}

/**
 * Adds a new item to the history.
 *
 * @param command the command that is being added to the history
 */
void history_add(char command[MAX_CMD_LENGTH]) {
    record_t *record = &history.buffer[history.next_write++];
    record->command_id = history.next_id++;
    strcpy(record->command, command);
    if (history.next_write == HISTORY_SIZE)
        history.next_write = 0;
}

/**
 * Searches the history for a given command ID, and if a command with a matching ID is found, that command
 * is copied to the provided destination.
 *
 * If a matching command is not found, an empty string is copied to the destination.
 *
 * @param command_id the command id to search for
 * @param dest where the command will be copied to
 */
void history_lookup(int command_id, char *dest) {
    int max = history.next_id - 1;
    int min = max - HISTORY_SIZE + 1;

    if (command_id < min || command_id > max) {
        *dest = '\0';
        return;
    }

    int index = ((history.next_write - 1) - (max - command_id) + HISTORY_SIZE) % HISTORY_SIZE;
    strcpy(dest, history.buffer[index].command);
}

/**
 * Prints the history to the console.
 */
void print_history() {
    int oldest = history.next_write % HISTORY_SIZE;
    int index = oldest;
    while (index < oldest + HISTORY_SIZE) {
        if (history.buffer[index % HISTORY_SIZE].command_id) {
            printf("  %d  %s",
                   history.buffer[index % HISTORY_SIZE].command_id,
                   history.buffer[index % HISTORY_SIZE].command);
        }
        index++;
    }
}


/**
 * Frees a 2D char array from the heap.
 *
 * @param args the array to be freed
 */
void free_args(char **args) {
    int index = 0;
    while (args[index] != NULL) {
        free(args[index]);
        index++;
    }
    free(args);
}

/**
 * Gets user input and stores in a char array.
 *
 * @param input the char array where the input will be stored
 */
void get_user_input(char *input) {
    char *result = fgets(input, MAX_CMD_LENGTH, stdin);
    if (result != input) {
        printf("Command could not be read. Exiting...\n");
        fflush(stdout);
        exit(-2);
    }
}

/**
 * Handler for SIGCHLD.
 *
 * @param sig the signal
 */
void sigchild_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * Executes the given command.
 *
 * @param args the arguments of the command to be executed
 * @param background indicates whether or not the command should be executed in the background
 */
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("Child process could not be created.\n");
        fflush(stdout);
        exit(-2);
    }
    if (!pid) {
        // This is the child process; we execute the command here
        if (execvp(*args, args) == -1) {
            printf("Could not execute command.\n");
            fflush(stdout);
            exit(-2);
        }
    } else if (!background) {
        // This is the parent process; if not running the command in the background, we wait for it to finish
        waitpid(pid, NULL, 0);
    }
}

int main() {
    // Register handler for SIGCHLD signals
    signal(SIGCHLD, sigchild_handler);

    // Initialize the history
    history_reset();

    char user_input[MAX_CMD_LENGTH];
    int background;
    char **args;

    while (1) {
        // Print the prompt
        printf("catshell> ");
        fflush(stdout);

        // Get user input and parse it
        get_user_input(user_input);

        // Ignore blank input
        if (*user_input == '\0')
            continue;

        // Check for command in history
        if (*user_input == '!') {
            history_lookup(atoi(user_input + 1), user_input);
            if (*user_input == '\0') {
                printf("Event not found.\n");
                fflush(stdout);
                continue;
            }
        }

        // Parse the command
        args = parse_command(user_input, &background);

        // Check for exit command
        if (!strcmp(*args, "exit")) {
            free_args(args);
            break;
        }
        // Check for history command
        if (!strcmp(*args, "history")) {
            history_add(user_input);
            print_history();
        } else {
            // Add the command to the history then fork a child and execute it
            history_add(user_input);
            execute_command(args, background);
        }
        // Free the arguments from the heap
        free_args(args);
    }

    return EXIT_SUCCESS;
}