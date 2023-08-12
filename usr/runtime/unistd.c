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
