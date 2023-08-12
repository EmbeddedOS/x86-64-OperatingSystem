#pragma once

#include <stdint.h>

#define ASSERT(exp) do {if (!(exp)) { error_point(__FILE__, __LINE__);} } while(0)

void error_point(const char *file, uint64_t line);
void panic(const char *msg);