#include <stdio.h>
#include <string.h>

#include "term.h"

extern FILE *output;

/* Arity 0 */
void term_b(char*, char**);
void term_i(char*, char**);
void term_u(char*, char**);
void term_spoiler(char*, char**);
void term_verbatim(char*, char**);

/* Arity 1*/
void term_url(char*, char**);

sexpcode_func term_functions[] = {
    {"b",           0, &term_b},
    {"i",           0, &term_i},
    {"u",           0, &term_u},
    {"spoiler",     0, &term_spoiler},
    {"verbatim",    0, &term_verbatim},
    {"url",         1, &term_url},
};

sexpcode_func *term_get_func(char *funcname)
{
    unsigned int j;

    for (j = 0; j < sizeof(term_functions) / sizeof(sexpcode_func); ++j)
        if (strcmp(funcname, term_functions[j].name) == 0)
            return &term_functions[j];

    return NULL;
}


/* Arity 0 */

void term_b(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "\033[1m");
    do_sexpcode(content);
    fprintf(output, "\033[22m");
}
void term_i(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "\033[3m");
    do_sexpcode(content);
    fprintf(output, "\033[23m");
}
void term_u(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "\033[4m");
    do_sexpcode(content);
    fprintf(output, "\033[24m");
}
void term_spoiler(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "\033[7m");
    do_sexpcode(content);
    fprintf(output, "\033[27m");
}
void term_verbatim(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, sexpcode_unescape(content));
}


/* Arity 1 */

void term_url(char *content, char **args)
{
    do_sexpcode(content);
    fprintf(output, " <%s>", args[0]);
}
