#include <unistd.h>
#include <syscall.h>

int write(int fd, const char *buf, size_t count)
{
    return syscall3((int64_t)SYS_WRITE,
                    (int64_t)fd,
                    (int64_t)buf,
                    (int64_t)count);
}

int read(int fd, char *buf, size_t count)
{
    return syscall3((int64_t)SYS_READ,
                    (int64_t)fd,
                    (int64_t)buf,
                    (int64_t)count);
}

unsigned int sleep(unsigned int seconds)
{
    return syscall1((int64_t)SYS_SLEEP,
                    (int64_t)seconds);
}

void exit(void)
{
    syscall0((int64_t)SYS_EXIT);
}

int wait(void)
{
    return syscall0((int64_t)SYS_WAIT);
}

int mem(void)
{
    return syscall0((int64_t)SYS_MEMINFO);
}