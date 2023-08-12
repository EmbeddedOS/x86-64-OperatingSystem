#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

/* Private define ------------------------------------------------------------*/
#define PRINT_MAX_BUFFER_SIZE       1024

/* Private type --------------------------------------------------------------*/

/* Private variable ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief    This helper function copy `str` string to the `buffer`, start at
 *           `pos` of the buffer.
 *
 * @param[in] buffer        - Buffer to write string.
 * @param[in] pos           - Position of the buffer to write the string.
 * @param[in] str           - String to be copied.
 * @return    This function return number of characters that are wrote to the
 *            `Buffer`.
 */
static int WriteStringToBuffer(char *buffer, int pos, const char *str);

/**
 * @brief    This helper function write a `integer` hex number to the `buffer`,
 *           start at `pos` of the buffer.
 *
 * @param[in] buffer        - Buffer to write string.
 * @param[in] pos           - Position of the buffer to write the string.
 * @param[in] integer       - Hex number to be wrote.
 * @return    This function return number of characters that are wrote to the
 *            `Buffer`.
 */
static int WriteHexToBuffer(char *buffer, int pos, uint64_t integer);

/**
 * @brief    This helper function write a `integer` unsigned decimal number to
 *           the `buffer`, start at `pos` of the buffer.
 *
 * @param[in] buffer        - Buffer to write string.
 * @param[in] pos           - Position of the buffer to write the string.
 * @param[in] integer       - Unsigned integer number to be copied.
 * @return    This function return number of characters that are wrote to the
 *            `Buffer`.
 */
static int WriteUDecimalToBuffer(char *buffer, int pos, uint64_t integer);

/**
 * @brief    This helper function write a `integer` decimal number to the
 *           `buffer`, start at `pos` of the buffer.
 *
 * @param[in] buffer        - Buffer to write string.
 * @param[in] pos           - Position of the buffer to write the string.
 * @param[in] integer       - Integer number to be copied.
 * @return    This function return number of characters that are wrote to the
 *            `Buffer`.
 */
static int WriteDecimalToBuffer(char *buffer, int pos, int64_t integer);

/* Public function -----------------------------------------------------------*/
int printf(const char *format, ...)
{
    char buffer[PRINT_MAX_BUFFER_SIZE] = {0};
    int buffer_size = 0;
    int64_t integer = 0;
    char *string = NULL;
    va_list args;

    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] != '%')
        {  
            /* Regular character will be copied to the buffer. */
            buffer[buffer_size++] = format[i];
        } else {
            switch (format[++i]) {
                case 'x': {
                    integer = va_arg(args, int64_t);
                    buffer_size += WriteHexToBuffer(buffer,
                                                        buffer_size,
                                                        (uint64_t)integer);
                }
                break;
                case 'u': {
                    integer = va_arg(args, int64_t);
                    buffer_size += WriteUDecimalToBuffer(buffer,
                                                            buffer_size,
                                                            (uint64_t)integer);
                }
                break;
                case 'd': {
                    integer = va_arg(args, int64_t);
                    buffer_size += WriteDecimalToBuffer(buffer,
                                                            buffer_size,
                                                            integer);
                }
                break;
                case 's': {
                    string = va_arg(args, char *);
                    buffer_size += WriteStringToBuffer(buffer,
                                                            buffer_size,
                                                            string);
                }
                break;
                default:
                    /* If specifies is not supported, we copy the character `%`
                     * to the buffer, decrement index and continue the loop. 
                     */
                    buffer[buffer_size++] = '%';
                    i--;
            }
        }
    }

    buffer_size = write(1, buffer, buffer_size);

    va_end(args);

    return buffer_size;
}

/* Private function ----------------------------------------------------------*/
static int WriteStringToBuffer(char *buffer, int pos, const char *str) 
{
    int index = 0;
    for (index = 0; str[index] != '\0'; index++)
    {
        buffer[pos] = str[index];
        pos++;
    }

    return index;
}

static int WriteHexToBuffer(char *buffer, int pos, uint64_t integer)
{
    char digits_buffer[25] = {0};
    char digits_map[16] = "0123456789ABCDEF";
    int size = 0;

    do {
        digits_buffer[size++] = digits_map[integer % 16];
        integer /= 16;
    } while (integer != 0);

    buffer[pos++] = '0';
    buffer[pos++] = 'x';

    for (int i = size - 1; i >= 0; i--) {
        buffer[pos++] = digits_buffer[i];
    }

    return size + 2;
}

static int WriteUDecimalToBuffer(char *buffer, int pos, uint64_t integer)
{
    char digits_buffer[25] = {0};
    char digits_map[10] = "0123456789";
    int size = 0;

    do {
        digits_buffer[size++] = digits_map[integer % 10];
        integer /= 10;
    } while (integer != 0);

    for (int i = size - 1; i >= 0; i--) {
        buffer[pos++] = digits_buffer[i];
    }

    return size;
}

static int WriteDecimalToBuffer(char *buffer, int pos, int64_t integer)
{
    int size = 0;

    if (integer < 0) {
        integer = -integer;
        buffer[pos++] = '-';
        size = 1;
    }

    size += WriteUDecimalToBuffer(buffer, pos, (uint64_t)integer);
    return size;
}
