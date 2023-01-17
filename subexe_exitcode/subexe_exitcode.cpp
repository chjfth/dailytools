#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

/*
This program runs a child process, waits for its finish, then prints 
this child process's exit code, from waitpid().

argv[1], argv[2], ...

    becomes child process's 

argv[0], argv[1], ...

*/

int main(int argc, char *const argv[])
{
    pid_t     pid;

    if(argc==1) {
        printf("[ERROR] Need at least one parameter as child process program name.\n");
        return 4;
    }

   
    pid = fork();
    if (pid < 0) {
        printf("[ERROR] fork() error.\n");
    } else if (pid == 0) {
        // child process
       
        char * const *argv_child = argv + 1;
        int err_launch = execvp(argv_child[0], argv_child);
        if(err_launch<0) {
            printf("[CHILD] Fail to exec '%s'.\n", argv_child[0]);
            exit(8);
        }
       
    } else {
        // parent process
       
        int wstatus = -1;
        pid_t ret = waitpid(pid, &wstatus, 0);
       
        if(ret<0) {
            printf("[PARENT] Unexpect: waitpid(%d) failed!\n", pid);
            return 4;
        }
       
        printf("[PARENT] Subprocess exit wstatus is: %d (0x%X)\n", wstatus, wstatus);
    }

    return (0);
}
