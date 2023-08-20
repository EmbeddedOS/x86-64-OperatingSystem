#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void) 
{
    int pid = fork();
    if (pid == 0) {
        exec("process4.bin");

    } else {
        printf("this is old process, we fork successfully new process: %d\n",
            (int64_t)pid);
        printf("waiting for child exit. \n");
        wait(pid);
    }
}