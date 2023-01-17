// https://stackoverflow.com/a/50989535/151453
// https://gist.github.com/davmac314/e4243431ed57eb1ae6dfbf885d72f5ba

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

// Test program to ensure OS correctly returns full exit status in si_status member
// of siginfo_t in SIGCHLD handler, as required by POSIX. Linux fails, FreeBSD succeeds.

sig_atomic_t exit_status;

void sigchld_handler(int n, siginfo_t *si, void *v)
{
    exit_status = si->si_status;
}

int main(int argc, char **argv)
{
    assert(sizeof(sig_atomic_t) >= 2); // enough for 0x1234 test value

    struct sigaction act;
    act.sa_sigaction = sigchld_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    
    sigaction(SIGCHLD, &act, NULL);
    
    if (fork() == 0) {
        _exit(0x1234);
    }

    sleep(3);
    printf("exit status was: %x\n", exit_status);
    
    return 0;
}

/* Result: On Ubuntu Linux 22.04, kernel 5.15, the output is:
	
	exit status was: 34
	
  i.e., only lowest 8-bit is preserved.
/*
