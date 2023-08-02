#pragma once

/* NOTE: Currently, we only support %x, %d, %s, %u specifiers.
 */
int printk(const char *format, ...);

int sprintk(char *str, const char *format, ...);