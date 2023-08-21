#include <stat.h>
#include <syscall.h>

/* Public function -----------------------------------------------------------*/
int lstat(const char *pathname, stat *statbuf)
{
    return syscall2((int64_t)SYS_LSTAT,
                    (int64_t)pathname,
                    (int64_t)statbuf);
}
