#include <stdlib.h>
#include <stddef.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define CAPACITY_INCR_RATIO 1.5
#define DEFAULT_CAPACITY 10

static void resize(Arr* arr, const size_t new_cap) {
    arr->items = realloc(arr->items, new_cap * sizeof(void*));
    arr->cap = new_cap;
}

Arr *Arr_new() {
    return Arr_newn(DEFAULT_CAPACITY);
}

Arr *Arr_newn(const size_t cap) {
    Arr *arr = malloc(sizeof(Arr));
    arr->len = 0;
    arr->cap = cap;
    arr->items = malloc(cap * sizeof(void*));
    return arr;
}

size_t Arr_add(Arr *arr, void *ptr) {
    if (arr->cap == arr->len) {
        resize(arr, arr->cap * CAPACITY_INCR_RATIO);
    }
    (arr->items)[arr->len] = ptr;
    arr->len += 1;
    return arr->len;
}

void *Arr_get(Arr *arr, size_t idx) {
    if (idx >= arr->len)
        return NULL;
    return arr->items[idx];
}

void Arr_free(const Arr *arr) {
    free(arr->items);
    free((void*)arr);
}

char *dyn_strcpy(const char *s) {
    char *cpy = calloc(strlen(s) + 1, sizeof(char));
    strcpy(cpy, s);
    return cpy;
}

/* Like strchr but looks for the first occurence of one of the chars.  
 * *chars must be a null-terminated string.
 * */
char *strchrs(char *str, const char *chars) {
    while (*str != '\0') {
        if (strchr(chars, *str))
            return str;
        ++str;
    }
    return NULL;
}

/*
int main(int argc, char **argv) {
    Arr *arr = Arr_new();
    {
        int *x = malloc(sizeof(int));
        *x = 50;
        Arr_add(arr, x);
    }
    int *x = Arr_get(arr, 0);
    printf("%d\n", *x);
    Arr_free(arr);
}
*/