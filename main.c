#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sexpcode.h"
#include "html.h"

#define BUFSIZE 1024 * 4

sexpcode_func *(*get_func)(char*);

int main(int argc, char **argv)
{
    char *buffa = malloc(BUFSIZE), *ret;
    int i = 1, size;

    /* extern get_func will be used to do the actual translation */
    get_func = &html_get_func;

    /* Read input from stdin */
    while ((size = fread(buffa + BUFSIZE * (i - 1), 1,
                         BUFSIZE,
                         stdin)) == BUFSIZE) {
        ++i;
        buffa = realloc(&buffa, BUFSIZE * i);
    }

    /* Pass 0: Bun's verbatim syntax and backslash escaping */
    if ((ret = pass0(buffa)) == NULL)
        return 1;

    free(buffa);
    buffa = ret;

    if (argc > 1 && strcmp(argv[1], "-pass0") == 0) {
        printf(sexpcode_unescape(buffa));
        return 0;
    }

    /* Pass 1: conversion to normal form */
    if ((ret = pass1(buffa)) == NULL)
        return 1;

    free(buffa);
    buffa = ret;

    if (argc > 1 && strcmp(argv[1], "-pass1") == 0) {
        printf(sexpcode_unescape(buffa));
        return 0;
    }

    /* Pass 2: application */
    if ((ret = pass2(buffa)) == NULL)
        return 1;

    free(buffa);

    printf(ret);
    return 0;
}

