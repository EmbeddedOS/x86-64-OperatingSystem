#pragma once

enum SYSCALL {
    SYS_WRITE = 0,
    SYS_SLEEP = 1,
    SYS_EXIT = 2,
    SYS_WAIT = 3,
    SYS_READ = 4,
    SYS_MEMINFO = 5
};

int syscall0(int64_t number);
int syscall1(int64_t number, int64_t p1);
int syscall2(int64_t number, int64_t p1, int64_t p2);
int syscall3(int32_t number, int64_t p1, int64_t p2, int64_t p3);
int syscall4(int32_t number, int64_t p1, int64_t p2, int64_t p3, int64_t p4);
int syscall5(int32_t number, int64_t p1, int64_t p2, int64_t p3, int64_t p4,
                int64_t p5);
