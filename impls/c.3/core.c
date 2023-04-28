#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "utils.h"
#include "core.h"
#include "env.h"
#include "printer.h"
#include "mem_debug.h"

static MalDatum *mal_add(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg types
    for (int i = 0; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        if (!MalDatum_istype(arg, INT)) {
            ERROR("+: expected INT arguments, but received %s", MalType_tostr(arg->type));
            return NULL;
        }
    }

    int rslt = ((MalDatum*)args->items[0])->value.i;
    for (int i = 1; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        rslt += arg->value.i;
    }

    return MalDatum_new_int(rslt);
}

static MalDatum *mal_sub(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg types
    for (int i = 0; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        if (!MalDatum_istype(arg, INT)) {
            ERROR("+: expected INT arguments, but received %s", MalType_tostr(arg->type));
            return NULL;
        }
    }

    int rslt = ((MalDatum*)args->items[0])->value.i;
    for (int i = 1; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        rslt -= arg->value.i;
    }

    return MalDatum_new_int(rslt);
}

static MalDatum *mal_mul(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg types
    for (int i = 0; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        if (!MalDatum_istype(arg, INT)) {
            ERROR("+: expected INT arguments, but received %s", MalType_tostr(arg->type));
            return NULL;
        }
    }

    int rslt = ((MalDatum*)args->items[0])->value.i;
    for (int i = 1; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        rslt *= arg->value.i;
    }

    return MalDatum_new_int(rslt);
}

static MalDatum *mal_div(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg types
    for (int i = 0; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        if (!MalDatum_istype(arg, INT)) {
            ERROR("+: expected INT arguments, but received %s", MalType_tostr(arg->type));
            return NULL;
        }
    }

    int rslt = ((MalDatum*)args->items[0])->value.i;
    for (int i = 1; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        rslt /= arg->value.i;
    }

    return MalDatum_new_int(rslt);
}

/* '=' : compare the first two parameters and return true if they are the same type
 * and contain the same value. In the case of equal length lists, each element
 * of the list should be compared for equality and if they are the same return
 * true, otherwise false
 */
static MalDatum *mal_eq(const Proc *proc, const Arr *args, MalEnv *env) {
    MalDatum *arg1 = args->items[0];
    MalDatum *arg2 = args->items[1];

    return MalDatum_eq(arg1, arg2) ? MalDatum_true() : MalDatum_false();
}

/* '>' : compare the first two numeric parameters */
static MalDatum *mal_gt(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg types
    for (int i = 0; i < args->len; i++) {
        MalDatum *arg = args->items[i];
        if (!MalDatum_istype(arg, INT)) {
            ERROR(">: expected INT arguments, but received %s", MalType_tostr(arg->type));
            return NULL;
        }
    }

    MalDatum *arg1 = args->items[0];
    MalDatum *arg2 = args->items[1];

    return arg1->value.i > arg2->value.i ? MalDatum_true() : MalDatum_false();
}

static MalDatum *mal_list(const Proc *proc, const Arr *args, MalEnv *env) {
    if (args->len == 0)
        return MalDatum_empty_list();

    List *list = List_new();
    for (int i = 0; i < args->len; i++) {
        MalDatum *dtm = args->items[i];
        // TODO avoid copying args to speed up
        List_add(list, MalDatum_deep_copy(dtm));
    }

    return MalDatum_new_list(list);
}

static MalDatum *mal_listp(const Proc *proc, const Arr *args, MalEnv *env) {
    return MalDatum_islist(args->items[0]) ? MalDatum_true() : MalDatum_false();
}

static MalDatum *mal_emptyp(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg type
    MalDatum *arg = args->items[0];
    if (!MalDatum_islist(arg)) {
        ERROR("empty?: expected a list, but got %s instead", MalType_tostr(arg->type));
        return NULL;
    }
    List *list = arg->value.list;
    return List_isempty(list) ? MalDatum_true() : MalDatum_false();
}

static MalDatum *mal_count(const Proc *proc, const Arr *args, MalEnv *env) {
    // validate arg type
    MalDatum *arg = args->items[0];

    int len;
    if (MalDatum_istype(arg, NIL))
        len = 0;
    else if (MalDatum_islist(arg))
        len = List_len(arg->value.list);
    else {
        ERROR("count: expected a list, but got %s instead", MalType_tostr(arg->type));
        return NULL;
    }

    return MalDatum_new_int(len);
}

// prn: calls pr_str on each argument with print_readably set to true, joins the
// results with " ", prints the string to the screen and then returns nil.
static MalDatum *mal_prn(const Proc *proc, const Arr *args, MalEnv *env)
{
    if (args->len > 0) {
        char *strings[args->len];

        for (size_t i = 0; i < args->len; i++) {
            MalDatum *arg = args->items[i];
            strings[i] = pr_str(arg, true);
        }

        // join with " "
        char *joined = str_join(strings, args->len, " ");
        printf("%s\n", joined);
        free(joined);

        for (size_t i = 0; i < args->len; i++)
            free(strings[i]);
    }

    return MalDatum_nil();
}

// pr-str: calls pr_str on each argument with print_readably set to true, joins
// the results with " " and returns the new string.
static MalDatum *mal_pr_str(const Proc *proc, const Arr *args, MalEnv *env)
{
    if (args->len == 0) {
        return MalDatum_new_string("");
    }
    else {
        char *strings[args->len];

        for (size_t i = 0; i < args->len; i++) {
            MalDatum *arg = args->items[i];
            strings[i] = pr_str(arg, true);
        }

        // join with " "
        char *joined = str_join(strings, args->len, " ");

        for (size_t i = 0; i < args->len; i++)
            free(strings[i]);

        MalDatum *out = MalDatum_new_string(joined);
        free(joined);
        return out;
    }
}

/*
 * str: calls pr_str on each argument with print_readably set to false,
 * concatenates the results together ("" separator), and returns the new
 * string.
 */
static MalDatum *mal_str(const Proc *proc, const Arr *args, MalEnv *env)
{
    if (args->len == 0) {
        return MalDatum_new_string("");
    }
    else {
        char *strings[args->len];

        for (size_t i = 0; i < args->len; i++) {
            MalDatum *arg = args->items[i];
            strings[i] = pr_str(arg, false);
        }

        char *joined = str_join(strings, args->len, "");

        for (size_t i = 0; i < args->len; i++)
            free(strings[i]);

        MalDatum *out = MalDatum_new_string(joined);
        free(joined);
        return out;
    }
}

/*
 * println: calls pr_str on each argument with print_readably set to false,
 * joins the results with " ", prints the string to the screen and then returns
 * nil.
 */
static MalDatum *mal_println(const Proc *proc, const Arr *args, MalEnv *env)
{
    if (args->len > 0) {
        char *strings[args->len];

        for (size_t i = 0; i < args->len; i++) {
            MalDatum *arg = args->items[i];
            strings[i] = pr_str(arg, false);
        }

        char *joined = str_join(strings, args->len, " ");
        printf("%s\n", joined);
        free(joined);

        for (size_t i = 0; i < args->len; i++)
            free(strings[i]);
    }

    return MalDatum_nil();
}

/* (procedure? datum) : predicate for procedures */
static MalDatum *mal_procedurep(const Proc *proc, const Arr *args, MalEnv *env) {
    MalDatum *arg = Arr_get(args, 0);
    return MalDatum_istype(arg, PROCEDURE) ? MalDatum_true() : MalDatum_false();
}

/* (arity proc) : returns a list of 2 elements:
 * 1. number of mandatory procedure arguments
 * 2. true if procedure is variadic, false otherwise
 */
static MalDatum *mal_arity(const Proc *proc, const Arr *args, MalEnv *env) {
    MalDatum *arg = Arr_get(args, 0);
    if (!MalDatum_istype(arg, PROCEDURE)) {
        ERROR("arity: expected a procedure");
        return NULL;
    }

    Proc *proc_arg = arg->value.proc;
    List *list = List_new();
    List_add(list, MalDatum_new_int(proc_arg->argc));
    List_add(list, proc_arg->variadic ? MalDatum_true() : MalDatum_false());

    return MalDatum_new_list(list);
}

// Returns true if a procedure (1st arg) is builtin
static MalDatum *mal_builtinp(const Proc *proc, const Arr *args, MalEnv *env) {
    MalDatum *arg = Arr_get(args, 0);
    if (!MalDatum_istype(arg, PROCEDURE)) {
        ERROR("builtin?: expected a procedure");
        return NULL;
    }

    Proc *proc_arg = arg->value.proc;

    return proc_arg->builtin ? MalDatum_true() : MalDatum_false();
}

void core_def_procs(MalEnv *env) 
{
#define DEF(name, arity, variadic, func_ptr) \
    do { \
        Symbol *sym = Symbol_new(name); \
        MalEnv_put(env, sym, MalDatum_new_proc(\
                    Proc_builtin(name, arity, variadic, func_ptr))); \
        Symbol_free(sym); \
    } while (0);

    DEF("+", 2, true, mal_add);
    DEF("-", 2, true, mal_sub);
    DEF("*", 2, true, mal_mul);
    DEF("/", 2, true, mal_div);
    DEF("=", 2, false, mal_eq);
    DEF(">", 2, false, mal_gt);

    DEF("list", 0, true, mal_list);
    DEF("list?", 1, false, mal_listp);
    DEF("empty?", 1, false, mal_emptyp);
    DEF("count", 1, false, mal_count);

    DEF("prn", 0, true, mal_prn);
    DEF("pr-str", 0, true, mal_pr_str);
    DEF("str", 0, true, mal_str);
    DEF("println", 0, true, mal_println);

    DEF("procedure?", 1, false, mal_procedurep);
    DEF("arity", 1, false, mal_arity);
    DEF("builtin?", 1, false, mal_builtinp);
}
