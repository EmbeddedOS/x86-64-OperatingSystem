#pragma once
#include <stdint.h>
#include <stddef.h>

int open(const char* filename);
int close(int fd);
int write(int fd, const char *buf, size_t count);
int read(int fd, char *buf, size_t count);
unsigned int sleep(unsigned int seconds);
void exit(void);
int wait(int pid);
int mem(void);
int fork(void);
int exec(const char* filename);
