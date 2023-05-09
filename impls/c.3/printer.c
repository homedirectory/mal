#include <stdio.h>
#include "printer.h"
#include "reader.h"
#include "utils.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include "common.h"

// When print_readably is true, doublequotes, newlines, and backslashes are
// translated into their printed representations (the reverse of the reader).
// In other words, print escapes as 2 characters
char *pr_str(const MalDatum *datum, bool print_readably) 
{
    if (datum == NULL) return NULL;

    char *str = NULL;

    switch (datum->type) {
        case INT:
            str = malloc(MAX_INT_DIGITS + 1);
            sprintf(str, "%d", datum->value.i);
            break;
        case SYMBOL:
            // TODO symbols with spaces
            str = dyn_strcpy(datum->value.sym->name);
            break;
        case LIST:
            str = pr_list(datum->value.list, print_readably);
            break;
        case STRING:
            char *string = datum->value.string;
            if (string == NULL) break;

            if (print_readably) {
                // TODO optimise
                char *escaped = str_escape(string);
                size_t esc_len = strlen(escaped);

                char *out = calloc(esc_len + 2 + 1, sizeof(char));
                out[0] = '"';
                memcpy(out + 1, escaped, esc_len);
                out[esc_len + 1] = '"';
                out[esc_len + 2] = '\0';
                str = out;

                free(escaped);
            }
            else
                str = dyn_strcpy(string);
            break;
        case NIL:
            str = dyn_strcpy("nil");
            break;
        case TRUE:
            str = dyn_strcpy("true");
            break;
        case FALSE:
            str = dyn_strcpy("false");
            break;
        case PROCEDURE:
            Proc *proc = datum->value.proc;

            StrAsm sa;
            StrAsm_init(&sa);

            StrAsm_add(&sa, "#<");
            StrAsm_add(&sa, Proc_is_macro(proc) ? "macro" : "procedure");
            if (proc->name) {
                StrAsm_addc(&sa, ':');
                StrAsm_add(&sa, proc->name);
            }
            StrAsm_addc(&sa, '>');

            str = StrAsm_str(&sa);
            break;
        case ATOM:
            Atom *atom = datum->value.atom;
            char *dtm_str = pr_str(atom->datum, print_readably);
            char *parts[] = { "(atom ", dtm_str, ")" };
            str = str_join(parts, ARR_LEN(parts), "");
            free(dtm_str);
            break;
        case EXCEPTION:
            str = dyn_strcpy("#<exn>");
            // Exception *exn = datum->value.exn;
            // if (exn->msg) {
            //     char *parts[] = { "#<exn:\"", exn->msg, "\">" };
            //     str = str_join(parts, ARR_LEN(parts), "");
            // }
            // else {
            //     str = dyn_strcpy("#<exn>");
            // }
            break;
        default:
            DEBUG("Unknown MalType %s", MalType_tostr(datum->type));
            str = NULL;
            break;
    }

    return str;
}

// returns a new string with the contents of the given list separeted by spaces 
// and wrapped in parens
char *pr_list(const List *list, bool print_readably) 
{
    if (list == NULL) return NULL;

    StrAsm sa;
    StrAsm_initsz(&sa, List_len(list) * 6);
    StrAsm_addc(&sa, '(');

    for (struct Node *node = list->head; node != NULL; node = node->next) {
        char *s = pr_str(node->value, print_readably);
        StrAsm_add(&sa, s);
        free(s);
        StrAsm_addc(&sa, ' ');
    }

    // if non-empty list, drop last redundant ' '
    if (!List_isempty(list))
        StrAsm_drop(&sa, 1);

    StrAsm_addc(&sa, ')');

    return StrAsm_str(&sa);
}

char *pr_repr(const MalDatum *datum)
{
    char *str = pr_str(datum, false);
    char *type_str = MalType_tostr(datum->type);

    char *parts[] = { type_str, str };
    char *out = str_join(parts, ARR_LEN(parts), " ");
    free(str);

    return out;
}
