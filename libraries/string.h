#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <error.h>

typedef struct {
    error_t error;
    uint32_t size;
    uint32_t capacity;
    char *data;
} string_t;

void *memory_set(void *dest, uint8_t value, uint32_t size);
void *memory_copy(void *dest, const void *source, uint32_t size);
void *memory_copy_reverse(void *dest, const void *source, uint32_t size);
bool memory_is_equal(const void *mem1, const void *mem2, uint32_t size);

string_t string(char *cstring);
string_t string_init(char *data, uint32_t size, uint32_t capacity);

uint32_t string_size(const string_t string);
uint32_t string_cstring_size(const char *cstring);
uint32_t string_capacity(const string_t string);
char *string_data(const string_t string);

char string_at(string_t string, uint32_t index);

string_t *string_clear(string_t *string);
string_t *string_copy(string_t *dest, const string_t source);
string_t *string_copy_cstring(string_t *dest, const char *cstring, uint32_t cstring_size);
string_t *string_append(string_t *dest, const char c);
string_t *string_concatenate(string_t *dest, const string_t source);
string_t *string_concatenate_reverse(string_t *dest, const string_t source);
string_t *string_format(string_t *dest, const string_t format, va_list ap);

bool string_is_equal(const string_t string1, const string_t string2);
bool string_is_valid(const string_t string);

uint32_t string_to_uint(const string_t string);
string_t string_uint_to_string(string_t dest, uint32_t u, uint32_t zero_padding, uint32_t base);
string_t string_int_to_string(string_t dest, int32_t i, uint32_t zero_padding, uint32_t base);

#endif
