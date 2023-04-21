#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <string.h>
#include <stddef.h>
#include "reader.h"
#include "types.h"

#define PROMPT "user> "


static MalDatum *read(char* in) {
    Reader *rdr = read_str(in);
    if (rdr == NULL || rdr->tokens->len == 0) {
        return NULL;
    }
    MalDatum *form = read_form(rdr);
    Reader_free(rdr);
    return form;
}

static MalDatum *eval(MalDatum *datum) {
    return datum;
}

static char *print(MalDatum *datum) {
    if (datum == NULL) {
        fprintf(stderr, "ERROR!\n");
        return NULL;
    }

    char *str = MalDatum_tostr(datum);
    return str;
}

int main(int argc, char **argv) {
    while (1) {
        //char *sp = malloc(50);
        //strcpy(sp, " 123 ");
        //char *line = sp;
        char *line = readline(PROMPT);
        if (line == NULL) {
            exit(EXIT_SUCCESS);
        }

        MalDatum *r = read(line);
        free(line);
        MalDatum *e = eval(r);
        //MalDatum_free(r);
        char *p = print(e);
        MalDatum_free(e);

        if (p != NULL) { 
            if (p[0] != '\0')
                printf("%s\n", p);
            free(p);
        }
    }
}
