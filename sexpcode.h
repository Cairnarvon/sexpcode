#ifndef SEXPCODE_H
#define SEXPCODE_H

typedef struct {
    char *name;
    int arity;
    void (*func)(char*, char**);
} sexpcode_func;

char *pass0(const char*);
char *pass1(char*);
char *pass2(char*);

char *sexpcode_unescape(char*);
int do_sexpcode(char*);

char *sexpcode(const char*);

#endif
