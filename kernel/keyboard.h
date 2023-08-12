#pragma once
#include <stdint.h>

/* Public function prototype -------------------------------------------------*/
void KeyboardHandler(void);
char ReadKeyBuffer(void);

unsigned char InByte(uint16_t port);