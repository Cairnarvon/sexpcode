#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <search.h>

#include "sexpcode.h"


#ifndef MAX_ITERS
#define MAX_ITERS 3
#endif

#ifndef MAX_DEFINES
#define MAX_DEFINES 1024
#endif

#ifndef MAX_BUN_TAG_LENGTH
#define MAX_BUN_TAG_LENGTH 1024
#endif

#ifndef MAX_ARITY
#define MAX_ARITY 255
#endif


extern sexpcode_func *(*get_func)(char*);


/* Pass 0 */

#define P0_NORMAL               0
#define P0_HAD_BRACE            1
#define P0_GETTING_START_TAG    2
#define P0_HAD_SPACE            3
#define P0_GETTING_END_TAG      4
#define P0_VERBATIM             5

char *pass0(const char *content)
{
    /*
    Pass 0 deals with Bun's verbatim syntax and backslash escaping.
    */
    unsigned char mode = P0_NORMAL;
    char tag[MAX_BUN_TAG_LENGTH + 1], sp = ' ';
    int tagsize = 0;
    FILE *output = tmpfile();

    for (; *content != 0; ++content) {
        if (mode == P0_NORMAL) {
            if (*content == '\\') {
                ++content;
                if (*content == '{')
                    fputc(1, output);
                else if (*content == '}')
                    fputc(2, output);
                else
                    fputc(*content, output);
            } else if (*content == '{')
                mode = P0_HAD_BRACE;
            else {
                fputc(*content, output);
                if (*content == 0)
                    break;
            }

        } else if (mode == P0_HAD_BRACE) {
            if (isalpha(*content)) {
                fputc('{', output);
                mode = P0_NORMAL;
                fputc(*content, output);
            } else if (*content == '{') {
                fputc('{', output);
            } else {
                mode = P0_GETTING_START_TAG;
                tagsize = 0;
                tag[tagsize++] = *content;
            }

        } else if (mode == P0_GETTING_START_TAG) {
            if (isspace(*content)) {
                mode = P0_VERBATIM;
                tag[tagsize] = 0;
            } else if (tagsize < MAX_BUN_TAG_LENGTH) {
                tag[tagsize++] = *content;
            } else {
                fprintf(stderr,
                        "Maximum length for Bun's verbatim tag exceeded.\n");
                fclose(output);
                return NULL;
            }

        } else if (mode == P0_VERBATIM) {
            if (isspace(*content)) {
                sp = *content;
                mode = P0_HAD_SPACE;
            } else if (*content == '{')
                fputc(1, output);
            else if (*content == '}')
                fputc(2, output);
            else
                fputc(*content, output);

        } else if (mode == P0_HAD_SPACE) {
            if (*content == tag[0]) {
                mode = P0_GETTING_END_TAG;
                tagsize = 1;
            } else {
                fputc(sp, output);
                fputc(*content, output);
                mode = P0_VERBATIM;
            }

        } else if (mode == P0_GETTING_END_TAG) {
            if (*content == '}' && tag[tagsize] == 0) {
                mode = P0_NORMAL;
            } else if (*content == tag[tagsize]) {
                ++tagsize;
            } else {
                int i;

                fputc(sp, output);
                for (i = 0; i < tagsize; ++i)
                    fputc(tag[i], output);
                fputc(*content, output);

                mode = P0_VERBATIM;
            }

        }

    }

    fputc(0, output);

    if (mode == P0_NORMAL) {
        int output_size = ftell(output);
        char *ret;

        ret = malloc(output_size);
        rewind(output);
        fread(ret, 1, output_size, output);
        fclose(output);

        return ret;
    } else {
        fprintf(stderr, "Didn't finish pass 0 in normal state.\n");
        fclose(output);
        return NULL;
    }
}


/* Pass 1 */

typedef struct {
    char *func;
    int iters;
    char *args;
} popped_function;

typedef struct {
    char *expr;
    int at;
} fundef;

int do_pass1(char*);
int substitute_def(char*);
int parse_expression(char*, char*, char**, int);
popped_function *pop_function(char**);
char *pop_arg(char**);

int at;

fundef *defs[MAX_DEFINES];
int defs_ptr = 0;

FILE *output;

char *pass1(char *content)
{
    /*
    Pass 1 expands composed and iterated functions and substitutes definitions,
    leaving the code in normal form.
    */
    int ret, i;
    
    output = tmpfile();

    hcreate(MAX_DEFINES);

    ret = do_pass1(content);
    fputc(0, output);

    hdestroy();
    for (i = 0; i < defs_ptr; ++i)
        free(defs[i]);

    if (ret) {
        fclose(output);
        return NULL;
    } else {
        int output_size = ftell(output);
        content = malloc(output_size);
        rewind(output);
        fread(content, 1, output_size, output);
        fclose(output);

        return content;
    }
}

int do_pass1(char *content)
{
    char *func_expr, *subject;
    int open_braces, fs;

    /* Scan to the first { */
    for (; *content != '{'; ++content) {
        if (!*content)  /* No SexpCode here */
            return 0;
        if (*content == '}') {
            fprintf(stderr, "Unexpected }.\n");
            return 1;
        }
        fputc(*content, output);
    }

    /* Extract the function expression */
    func_expr = ++content;

    for (open_braces = 0; !(isspace(*content) && open_braces == 0); ++content) {
        switch (*content) {
        case '{':
            ++open_braces;
            break;
        case '}':
            --open_braces;
            break;
        case 0:
            /* Premature EOS */
            fprintf(stderr, "Function expression ended abruptly.\n");
            return 1;
        }
    }
    *content = 0;

    /* Extract the subject of function application */
    subject = ++content;
    for (open_braces = 0; !(*content == '}' && open_braces == 0); ++content) {
        switch (*content) {
        case '{':
            ++open_braces;
            break;
        case '}':
            --open_braces;
            break;
        case 0:
            /* Premature EOS */
            fprintf(stderr, "Unclosed statement.\n");
            return 1;
        }
    }
    *content = 0;

    if (strcmp("define", func_expr) == 0) {
        /* Function definition */
        ENTRY r, *e;
        fundef *fd;

        r.key = pop_arg(&subject);

        if (r.key == NULL || (r.key[0] == '\'' && r.key[1] == '{') ||
            *subject == 0) {
            fprintf(stderr, "Malformed definition.\n");
            return 1;
        }
       
        fd = malloc(sizeof(fundef));
        defs[defs_ptr++] = fd;

        fd->expr = subject;
        fd->at = at++;
        r.data = fd;

        /* hsearch is dumb */
        if ((e = hsearch(r, FIND)) != NULL) {
            ((fundef*)e->data)->expr = subject;
        } else if (hsearch(r, ENTER) == NULL) {
            fprintf(stderr, "Definition table is full.\n");
            return 1;
        }

    } else if (strcmp("undefine", func_expr) == 0) {
        /* Function undefinition */
        ENTRY r, *e;

        r.key = subject;

        if ((e = hsearch(r, FIND)) != NULL)
            ((fundef*)e->data)->expr = NULL;

    } else {
        /* Normal function expression */
        fs = parse_expression(func_expr, NULL, &subject, at);
        if (fs < 0)
            return 1;

        if (do_pass1(subject))
            return 1;

        for (; fs > 0; --fs)
            fputc('}', output);
    }

    return do_pass1(content + 1);
}

int parse_expression(char *func_expr, char *args, char **content, int curat)
{
    popped_function *p;
    char *subject = *content;
    int i, verbatim = 0, fs = 0;
    ENTRY e, *r;

    while ((p = pop_function(&func_expr)) != NULL) {
        if (p->iters > MAX_ITERS || p->iters < 0) {
            fprintf(stderr, "Invalid number of iters: %d.\n", p->iters);
            return -1;
        }

        e.key = p->func;

        if (p->args != NULL) {
            /* Second argument syntax; recurse */
            for (i = 0; i < p->iters; ++i) {
                int j = parse_expression(p->func, p->args, &subject, curat);

                if (j < 0)
                    return -1;

                fs += j;
            }

        } else if (strcmp("verbatim", p->func) == 0) {
            /* Ugly hack for composed verbatim FIXME */
            verbatim = 1;

        } else if ((r = hsearch(e, FIND)) != NULL &&
                   ((fundef*)r->data)->expr != NULL &&
                   ((fundef*)r->data)->at < curat) {
            /* Definition substitition; recurse */
            int j;
            char *expr = malloc(strlen(((fundef*)r->data)->expr) + 1);

            strcpy(expr, ((fundef*)r->data)->expr);
            curat = at;

            j = parse_expression(expr,
                                 args,
                                 content,
                                 ((fundef*)r->data)->at);
            free(expr);

            if (j < 0)
                return -1;

            fs += j;

        } else {
            /* Normal function */
            sexpcode_func *f = get_func(p->func);

            if (f == NULL) {
                fprintf(stderr, "Unknown function: %s.\n", p->func);
                return -1;
            }

            for (i = 0; i < p->iters; ++i) {
                int j;

                fprintf(output, "{%s ", p->func);

                for (j = 0; j < f->arity; ++j) {
                    char *arg;

                    if ((arg = pop_arg(&args)) == NULL)
                        if ((arg = pop_arg(&subject)) == NULL) {
                            fprintf(stderr, "Not enough args to %s\n", p->func);
                            return -1;
                        }

                    fprintf(output, "%s ", arg);
                }

                ++fs;
            }

            *content = subject;
        }

        free(p);
    }

    if (verbatim) {
        fprintf(output, "{verbatim ");
        ++fs;
    }

    return fs;
}

popped_function *pop_function(char **func_expr)
{
    char *expr = *func_expr, *rest = *func_expr;
    popped_function *r;;

    if (!*expr)
        return NULL;

    r = malloc(sizeof(popped_function));
    r->iters = 1;

    if (*expr == '{') {
        int open_braces;

        ++rest;
        ++expr;

        r->func = expr;

        /* Scan to of expression, balancing braces */
        for (open_braces = 0; !(isspace(*rest) && open_braces == 0); ++rest) {
            switch (*rest) {
            case '{':
                ++open_braces;
                break;
            case '}':
                --open_braces;
                break;
            case 0:
                fprintf(stderr, "Partial function expression ran off end.\n");
                free(r);
                return NULL;
            }
        }
        *rest = 0;
        r->args = ++rest;

        /* Scan to end of arglist, balancing braces */
        for (open_braces = 0; !(*rest == '}' && open_braces == 0); ++rest) {
            switch (*rest) {
            case '{':
                ++open_braces;
                break;
            case '}':
                --open_braces;
                break;
            case 0:
                fprintf(stderr, "Partial function args ran off end.\n");
                free(r);
                return NULL;
            }
        }
        *rest = 0;
        ++rest;

        if (*rest == '^' || *rest == '*') {
            r->iters = atoi(rest + 1);
            for (; isdigit(*rest); ++rest);
            if (*rest != 0 && *rest != '.') {
                fprintf(stderr, "Faulty iter specification.\n");
                free(r);
                return NULL;
            }
        }

        if (*rest == 0) {
            /* End of expression */
            --rest;
        } else if (*rest == '.') {
            *rest = 0;
        } else {
            fprintf(stderr, "Illegal juxtaposition.\n");
            free(r);
            return NULL;
        }

    } else {
        r->func = expr;
        r->args = NULL;

        for (;; ++rest) {
            if (*rest == '^' || *rest == '*') {
                *rest++ = 0;
                r->iters = atoi(rest);
                for (; isdigit(*rest); ++rest);
                if (*rest != 0 && *rest != '.') {
                    fprintf(stderr, "Faulty iter specification.\n");
                    free(r);
                    return NULL;
                }
            }
            if (*rest == 0) {
                /* End of function expression */
                --rest;
                break;
            } else if (*rest == '.') {
                *rest = 0;
                break;
            }
        }
    }

    *func_expr = ++rest;

    return r;
}

char *pop_arg(char **content)
{
    char *arg = *content, *rest;

    if (arg == NULL)
        return NULL;

    /* Get rid of preceding whitespace */
    for (; *arg && isspace(*arg); ++arg);
    rest = arg;

    if (*arg == '\'' && *(arg + 1) == '{') {
        for (; *rest && *rest != '}'; ++rest);
        if (!*rest) {
            fprintf(stderr, "Arg ran off string.\n");
            return NULL;
        }
        ++rest;
    } else
        for (; *rest && !isspace(*rest); ++rest);

    *rest = 0;
    *content = ++rest;

    return arg;
}


/* Pass 2 */

char *sexpcode_unescape(char*);

char *pass2(char *content)
{
    /*
    Pass 2 translates normal-form SexpCode to output using an external get_func.
    */
    output = tmpfile();

    if (do_sexpcode(content)) {
        fclose(output);
        return NULL;
    } else {
        int output_size = ftell(output) + 1;

        fputc(0, output);
        content = malloc(output_size);
        rewind(output);
        fread(content, 1, output_size, output);
        fclose(output);

        return content;
    }
}

int do_sexpcode(char *content)
{
    int i, open_braces;
    char *args[MAX_ARITY], *start = content, *rest;
    sexpcode_func *f;

    /* Find the first { */
    for (; *content != '{'; ++content) {
        switch (*content) {
        case 0:
            /* No SexpCode here */
            return 0;
        case 1:
            *content = '{';
            if (0)
        case 2:
                *content = '}';
        default:
            fputc(*content, output);
        }
    }

    *content++ = 0;
    start = content;

    /* Find the matching } */
    for (open_braces = 0; open_braces != 0 || *content != '}'; ++content) {
        switch (*content) {
        case '{':
            ++open_braces;
            break;
        case '}':
            --open_braces;
            break;
        case 0:
            fprintf(stderr, "Unclosed expression.\n");
            return 1;
        }
    }

    *content = 0;
    rest = ++content;

    /* Extract function */
    for (content = start; *content && !isspace(*content); ++content);
    
    if (!*content) {
        fprintf(stderr, "{problem}\n");
        return 1;
    }

    *content++ = 0;
    f = get_func(start);

    if (f == NULL) {
        fprintf(stderr, "Not a real function: %s.\n", start);
        return 1;
    }

    /* Extract arguments */
    for (i = 0; i < f->arity; ++i) {
        args[i] = sexpcode_unescape(pop_arg(&content));

        if (args[i] == NULL) {
            fprintf(stderr, "Bad args to %s.\n", start);
            return 1;
        }

        if (args[i][0] == '\'' && args[i][1] == '{') {
            args[i] += 2;
            args[i][strlen(args[i]) - 1] = 0;
        }
    }

    /* Apply */
    f->func(content, args);

    return do_sexpcode(rest);
}

char *sexpcode_unescape(char *content)
{
    char *start = content;

    do {
        if (*content == 1)
            *content = '{';
        if (*content == 2)
            *content = '}';
    } while (*content++);

    return start;
}


/* All together now */

char *sexpcode(const char *input)
{
    char *a, *b;

    if ((a = pass0(input)) == NULL)
        return NULL;

    b = pass1(a);
    if (b == NULL)
        return NULL;

    a = pass2(b);
    return a;
}
