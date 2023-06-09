#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>

#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    if (cmd == NULL) {
        return false;
    }

    int result = system(cmd);
    return result == 0 ? true : false;
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

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
	printf("%s", command[i]);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    int child_pid = fork();
    if (child_pid == -1) {
    	perror("Creating fork");
	return false;
    }
    
    if (child_pid == 0) {
        // Child process
	int result = execv(command[0], command);
	if (result == -1) {
            perror("Executing command");
            return false;
        } 
    } else {
        // Parent process
	int status = 0;
        pid_t wait_result = waitpid(child_pid, &status, 0);
	if (wait_result != child_pid) {
            perror("Parent: ");
            return false;
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
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    openlog(NULL, 0, LOG_USER);

    va_list args;
    va_start(args, count);
    char * command[count+1];
    char * cmd_in[count];
    int i;

    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
	if (i > 0) {
	    cmd_in[i - 1] = command[i];
            syslog(LOG_ERR, "arg %i %s", i - 1, cmd_in[i - 1]);
	} else {
            syslog(LOG_ERR, "cmd %s", command[i]);
	}
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0755);
    if (fd < 0) {
	syslog(LOG_ERR, "Opening file %s", outputfile);
        perror("Opening file");
	return false;
    }

    if (dup2(fd, 1) < 0) { 
        perror("dup2"); 
        return false; 
    }
    //bool result = do_exec(count, args);

    int child_pid = fork();
    if (child_pid == -1) {
        perror("Creating fork");
        return false;
    }

    if (child_pid == 0) {
        // Child process
        int result = execv(command[0], command);
        if (result == -1) {
            perror("Executing command");
            return false;
        }
    } else {
        // Parent process
        int status = 0;
        pid_t wait_result = waitpid(child_pid, &status, 0);
        if (wait_result != child_pid) {
            perror("Parent: ");
            return false;
        }
    }


    close (fd);
    va_end(args);

    closelog();
    return true;
}
