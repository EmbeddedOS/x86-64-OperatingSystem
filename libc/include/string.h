#pragma once

#include <stddef.h>

/**
 * @brief   Copies the character `c` (an unsigned char) to the first `size`
 *          characters of the string pointed to, by the argument `str`.
 *
 * @param[in] str           - Pointer to the block of memory to fill.
 * @param[in] c             - The value to be set.
 * @param[in] size          - The number of bytes to be set to the value.
 * @return    This function returns a pointer to the memory area str.
 */
void *memset(void *str, int c, size_t size);

/**
 * @brief   Copies `n` characters from memory area `s` to memory area `d`.
 *
 * @param[in] d             - Pointer to the destination array where the content
 *                            is to be copied, type-casted to a pointer of type
 *                            void*.
 * @param[in] s             - Pointer to the source of data to be copied, type-
 *                            casted to a pointer of type void*.
 * @param[in] n             - The number of bytes to be copied.
 * @return    This function returns a pointer to destination.
 */
void *memcpy(void *d, const void *s, size_t n);

/**
 * @brief   Copies `n` characters from `str2` to `str1`, but for overlapping
 *          memory blocks, memmove() is a safer approach than memcpy().
 *
 * @param[in] str1          - Pointer to the destination array where the content
 *                            is to be copied, type-casted to a pointer of type
 *                            void*.
 * @param[in] str2          - Pointer to the source of data to be copied, type-
 *                            casted to a pointer of type void*.
 * @param[in] n             - The number of bytes to be copied.
 * @return    This function returns a pointer to destination.
 */
void *memmove(void *str1, const void *str2, size_t n);

/**
 * @brief   Compares the first `n` bytes of memory area `str1` and memory area
 *          `str2`.
 *
 * @param[in] str1          - Pointer to a block of memory.
 * @param[in] str2          - Pointer to a block of memory.
 * @param[in] n             - The number of bytes to be compared.
 * @return    If Return value < 0 then it indicates str1 is less than str2.
 *            If Return value > 0 then it indicates str2 is less than str1.
 *            If Return value = 0 then it indicates str1 is equal to str2.
 */
int memcmp(const void *str1, const void *str2, size_t n);

/**
 * @brief   Computes the length of the string `s` up to, but not including the
 *          terminating null character.
 *
 * @param[in] s             - String whose length is to be found.
 * @return    This function returns the length of string.
 */
size_t strlen(const char *s);

/**
 * @brief   Computes the length of the string `s` up to, but not including the
 *          terminating null character, but at most `count`.
 *
 * @param[in] s             - String whose length is to be found.
 * @param[in] count         - Max length to count.
 * @return    This function returns the length of string if length < `count`
 *            otherwise it return `count`.
 */
size_t strnlen(const char *s, size_t count);

/**
 * @brief   Searches for the first occurrence of the character `c` (an unsigned 
 *          char) in the string pointed to by the argument `s`.
 *
 * @param[in] s             - String to be scanned.
 * @param[in] c             - Character to be searched in str.
 * @return    Returns a pointer to the first occurrence of the character `c` in
 *            the string `str`, or NULL if the character is not found.
 */
char *strchr(const char *s, int c);

/**
 * @brief   Copies the string pointed to, by `s` to `d`.
 *
 * @param[in] d             - Pointer to the destination array where the content
 *                            is to be copied.
 * @param[in] s             - String to be copied.
 * @return    Returns a pointer to the destination string `d`.
 */
char *strcpy(char *d, const char *s);

/**
 * @brief   Compares at most the first `c` bytes of `s1` and `s2`.
 *
 * @param[in] s1            - First string to be compared.
 * @param[in] s2            - Second string to be compared.
 * @param[in] n             - Maximum number of characters to be compared.
 * @return    If Return value < 0 then it indicates `s1` is less than `s2`.
 *            If Return value > 0 then it indicates `s2` is less than `s1`.
 *            If Return value = 0 then it indicates `s1` is equal to `s2`.
 */
int strncmp(const char *s1, const char *s2, int c);

/**
 * @brief   copies up to n characters from the string pointed to, by `s` to `d`.
 *          In a case where the length of `s` is less than that of `n`, the
 *          remainder of `d` will be padded with null bytes.
 *
 * @param[in] d             - Pointer to the destination array where the content
 *                            is to be copied.
 * @param[in] s             - String to be copied.
 * @param[in] n             - Number of characters to be copied from source.
 * @return    This function returns the pointer to the copied string.


 */
char *strncpy(char *d, const char *s, size_t n);