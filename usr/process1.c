#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void) 
{
    int pid = fork();
    if (pid == 0) {
        int count = 0;
        while (1)
        {
            count++;
            sleep(300);
            printf("this is new process.\n");
            if (count == 5) {
                printf("exiting new process.\n");
                exit();
            }
        }
    } else {
            sleep(300);
            printf("this is old process, we fork successfully new process: %d\n",
                (int64_t)pid);
            printf("waiting for child exit. \n");
            wait(pid);
            printf("child exited. Cleaned up.\n");
    }


}