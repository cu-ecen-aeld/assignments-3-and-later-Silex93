/****************************************************************************
 *  Author:     Daniel Mendez
 *  Course:     ECEN 5713
 *  Project:    Assignment_3 Part 1
 *
 ****************************************************************************/


#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syslog.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "systemcalls.h"


/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd) {
    //Ensure cmd is not null
    if (cmd == NULL) {
        return false;
    }
    //Perform the system call
    int res = system(cmd);

    if (res != 0) {
        printf("Error code: %d\n", errno);
        return false;
    }
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...) {
    openlog(NULL, 0, LOG_USER);

    va_list args;
    va_start(args, count);
    char *command[count + 1];
    int i;
    for (i = 0; i < count; i++) {
        command[i] = va_arg(args,
        char *);

    }
    command[count] = NULL;

    char *path = command[0];

    //Ensure that the path is not null or empty
    if (path == NULL || strlen(path) == 0) {
        syslog(LOG_ERR, "AESD: Path is empty or NULL\n");
        return false;
    }

    //Check if the path is valid - MOdified based on CHATGPT output
    struct stat buffer;
    if (stat(path, &buffer) != 0) {
        syslog(LOG_ERR, "AESD: Path is invalid\n");
        return false;
    }

    //Create new process
    pid_t child_pid = fork();

    if (child_pid == -1) {
        syslog(LOG_ERR, "AESD: Failed to fork a child process\n");
        exit(EXIT_FAILURE);
        return false;
    } else if (child_pid == 0) {

        // Execute the command and check if the process returns okay
        if (execv(path, command) == -1) {
            syslog(LOG_ERR, "AESD: Program failed to execute\n");
            exit(EXIT_FAILURE);
        }
        // syslog(LOG_INFO, "AESD: Child Process Completed\n");
        //exit(EXIT_SUCCESS);
    } else {
        // Wait for the child process to complete
        int status;
        // if (waitpid(child_pid, &status, 0) == -1) {
        if (wait(&status) == -1) {
            syslog(LOG_ERR, "AESD: Wait failed!\n");
            return false;
        }

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                return false;
            }
        } else {
            return false; // Command execution failed
        }
    }

    va_end(args);
    return true;
}
/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
//Reference used: https://stackoverflow.com/questions/13784269/redirection-inside-call-to-execvp-not-working/13784315#13784315
bool do_exec_redirect(const char *outputfile, int count, ...) {
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_INFO, "AESD: BEGIN LOG");
    int fd;
    va_list args;
    va_start(args, count);
    char *command[count + 1];
    int i;
    for (i = 0; i < count; i++) {
        command[i] = va_arg(args,
        char *);
    }
    command[count] = NULL;


    char *path = command[0];

    //Ensure that the paths are not null or empty
    if (path == NULL || strlen(path) == 0 || outputfile == NULL || strlen(outputfile) == 0) {
        syslog(LOG_ERR, "AESD: Paths are empty or NULL\n");
        return false;
    }

    //Check if the paths are valid - Modified based on CHATGPT output
    struct stat buffer;
    if (stat(path, &buffer) != 0 || stat(outputfile, &buffer) != 0) {
        syslog(LOG_ERR, "AESD: Path is invalid\n");
        return false;
    }

    //Check that the file was opened correctly
    fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) {
        syslog(LOG_ERR, "AESD: Failed to open redirection file \n");
        return false;
    }
    //Create new process
    pid_t child_pid = fork();

    if (child_pid == -1) {
        syslog(LOG_ERR, "AESD: Failed to fork a child process\n");
        return false;
    } else if (child_pid == 0) {

        //Duplicate the file descriptor
        if (dup2(fd, 1) < 0) {
            syslog(LOG_ERR, "AESD: Failed to duplicate file descriptor\n");
            exit(EXIT_FAILURE);
            //return false;
        }

        syslog(LOG_INFO, "AESD: Command[0]: %s, Command[1]: %s\n", command[0], command[1]);
        // Execute the command and check if the process returns okay
        if (execv(path, command) == -1) {
            syslog(LOG_ERR, "AESD: Program failed to execute\n");
            exit(EXIT_FAILURE);
        }

        //Close the original fd
        close(fd);
    } else {
        close(fd);
        // Wait for the child process to complete
        int status;
        if (waitpid(child_pid, &status, 0) == -1) {
            syslog(LOG_ERR, "AESD: Wait failed!\n");
            return false;
        }
        //Check the return status of the child process
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                return false;
            }

        } else {
            exit(EXIT_FAILURE);
            return false; // Command execution failed
        }

    }
    va_end(args);
    closelog();
    return true;


}
