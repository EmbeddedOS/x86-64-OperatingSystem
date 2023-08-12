#pragma once
#include <stdint.h>
#include <stddef.h>

int write(int fd, const char *buf, size_t count);
int read(int fd, char *buf, size_t count);
