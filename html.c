#include <stdio.h>
#include <string.h>

#include "html.h"

extern FILE *output;

/* Arity 0 */
void html_b(char*, char**);
void html_i(char*, char**);
void html_o(char*, char**);
void html_u(char*, char**);
void html_s(char*, char**);
void html_m(char*, char**);
void html_tt(char*, char**);
void html_spoiler(char*, char**);
void html_sub(char*, char**);
void html_sup(char*, char**);
void html_quote(char*, char**);
void html_verbatim(char*, char**);

/* Arity 1*/
void html_url(char*, char**);
void html_code(char*, char**);
void html_img(char*, char**);
void html_ruby(char*, char**);

sexpcode_func html_functions[] = {
    {"b",           0, &html_b},
    {"i",           0, &html_i},
    {"o",           0, &html_o},
    {"u",           0, &html_u},
    {"s",           0, &html_s},
    {"m",           0, &html_m},
    {"tt",          0, &html_tt},
    {"spoiler",     0, &html_spoiler},
    {"sub",         0, &html_sub},
    {"sup",         0, &html_sup},
    {"quote",       0, &html_quote},
    {"verbatim",    0, &html_verbatim},
    {"url",         1, &html_url},
    {"code",        1, &html_code},
    {"img",         1, &html_img},
    {"ruby",        1, &html_ruby}
};

sexpcode_func *html_get_func(char *funcname)
{
    unsigned int j;

    for (j = 0; j < sizeof(html_functions) / sizeof(sexpcode_func); ++j)
        if (strcmp(funcname, html_functions[j].name) == 0)
            return &html_functions[j];

    return NULL;
}


/* Arity 0 */

void html_b(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<b>");
    do_sexpcode(content);
    fprintf(output, "</b>");
}
void html_i(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<i>");
    do_sexpcode(content);
    fprintf(output, "</i>");
}
void html_o(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<span class=\"o\">");
    do_sexpcode(content);
    fprintf(output, "</span>");
}
void html_u(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<u>");
    do_sexpcode(content);
    fprintf(output, "</u>");
}
void html_s(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<s>");
    do_sexpcode(content);
    fprintf(output, "</s>");
}
void html_m(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<m>");
    do_sexpcode(content);
    fprintf(output, "</m>");
}
void html_tt(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<tt>");
    do_sexpcode(content);
    fprintf(output, "</tt>");
}
void html_spoiler(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<span class=\"spoiler\">");
    do_sexpcode(content);
    fprintf(output, "</span>");
}
void html_sub(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<sub>");
    do_sexpcode(content);
    fprintf(output, "</sub>");
}
void html_sup(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<sup>");
    do_sexpcode(content);
    fprintf(output, "</sup>");
}
void html_quote(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, "<blockquote>");
    do_sexpcode(content);
    fprintf(output, "</blockquote>");
}
void html_verbatim(char *content, char **args __attribute__ ((unused)))
{
    fprintf(output, sexpcode_unescape(content));
}


/* Arity 1 */

void html_url(char *content, char **args)
{
    fprintf(output, "<a href=\"%s\">", args[0]);
    do_sexpcode(content);
    fprintf(output, "</a>");
}
void html_code(char *content, char **args)
{
    fprintf(output, "<code title=\"%s code\">", args[0]);
    do_sexpcode(content);
    fprintf(output, "</code>");
}
void html_img(char *content, char **args)
{
    fprintf(output,
            "<img src=\"%s\" title=\"%s\" />",
            args[0],
            sexpcode_unescape(content));
}
void html_ruby(char *content, char **args)
{
    fprintf(output, "<ruby>");
    do_sexpcode(content);
    fprintf(output, "<rt>%s</rt></ruby>", args[0]);
}
