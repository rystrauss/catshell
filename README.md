# Catshell

An implementation of a shell that mimics the behavior of the Bourne Again Shell (`bash`).

This shell, named `catshell`, has the following built in commands:

 * `history`: displays a history of the 10 previous commands
 * `!<num>`: executes the command in the history with the ID `<num>`
 * `exit`: exits the shell
 
 For any other command, the shell will fork a child process then execute the command in the child.
 
 The shell also supports running commands in the background. This is done by putting an `&` the end of the command.