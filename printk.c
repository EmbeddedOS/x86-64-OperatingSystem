#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "printk.h"
/* Private define ------------------------------------------------------------*/
#define LINE_SIZE                   (80 * 2)
#define VGA_BASE                    0xB80000
#define PRINT_MAX_BUFFER_SIZE       1024

/* Private type --------------------------------------------------------------*/
struct ScreenBuffer {
    char *buffer;
    int column;
    int row;
};

/* Private variable ----------------------------------------------------------*/
static struct ScreenBuffer screen_buffer = {
    .buffer = 0xB80000,
    .column = 0,
    .row = 0
};

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief    This helper function copy `str` string to the `buffer`, start at
 *           `pos` of the buffer.
 *
 * @param[in] buffer        - Buffer to write string.
 * @param[in] str           - String to be copied.
 * @param[in] pos           - Position of the buffer to write the string.
 * @return    This function return number of characters that are wrote to the
 *            `Buffer`.
 */
static int write_string_to_buffer(char *buffer, int pos, const char *str);

/* Public function -----------------------------------------------------------*/
int printk(const char *format, ...)
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

                }
                break;
                case 'u': {
                    
                }
                break;
                case 'd': {

                }
                break;
                case 's': {
                    string = va_arg(args, char *);
                    buffer_size += write_string_to_buffer(buffer,
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

    va_end(args);
}

/* Private function ----------------------------------------------------------*/
static int write_string_to_buffer(char *buffer, int pos, const char *str) 
{
    int index = 0;
    for (index = 0; str[index] != '\0'; index++)
    {
        buffer[pos] = str[index];
        pos++;
    }

    return index;
}