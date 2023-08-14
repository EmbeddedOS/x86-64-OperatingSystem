#pragma once
#include <stdint.h>
#include <stddef.h>

/* Public function prototype -------------------------------------------------*/
uint8_t InByte(uint16_t port);
uint16_t InWord(uint16_t port);
void OutByte(uint16_t port, uint8_t val);
void OutWord(uint16_t port, uint16_t val);
