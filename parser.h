/**
 * Interface for the string tokenization library.
 *
 * @author RR
 */


/**
 * Returns an array of strings containing tokens extracted from the supplied string.
 *
 * @param cmd_line the string to be tokenized
 * @param background a pointer to a value that is set depending on whether the
 * supplied string describes a command to be executed in
 * "background" mode, i.e., whether the last non-white space
 * character in the command string is an &. If it is an &,
 * then the command is to be run in background mode, and
 * *background is set to 1; otherwise, it is set to 0.
 * @return A NULL-terminated array of char*s, where each char* points to a
 * string containing an extracted token.
 */
char **parse_command(const char *cmd_line, int *bg);
