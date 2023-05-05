#include "types.h"
#include "env.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"
#include "mem_debug.h"
#include <assert.h>
#include "printer.h"


List *List_new() {
    List *list = malloc(sizeof(List));
    list->len = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

size_t List_len(const List *list) {
    return list->len;
}

List *List_copy(const List *list) {
    if (list == NULL) {
        LOG_NULL(list);
        return NULL;
    }

    List *out = List_new();

    struct Node *node = list->head;
    while (node) {
        List_add(out, node->value);
        node = node->next;
    }

    return out;
}

List *List_deep_copy(const List *list) {
    if (list == NULL) {
        LOG_NULL(list);
        return NULL;
    }

    List *out = List_new();

    struct Node *node = list->head;
    while (node) {
        MalDatum *cpy = MalDatum_deep_copy(node->value);
        if (cpy == NULL) {
            LOG_NULL(cpy);
            List_free(out);
            return NULL;
        }
        List_add(out, cpy);
        node = node->next;
    }

    return out;
}

static List _empty_list = { .len = 0, .head = NULL, .tail = NULL };
List *List_empty() {
    return &_empty_list;
}

bool List_isempty(const List *list) {
    if (list == NULL) {
        LOG_NULL(list);
        exit(EXIT_FAILURE);
    }
    return list->len == 0;
}

void List_add(List *list, MalDatum *datum) {
    struct Node *node = malloc(sizeof(struct Node));
    node->value = datum;
    node->next = NULL;

    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    MalDatum_own(datum);

    list->len += 1;
}

void List_append(List *dst, const List *src)
{
    if (dst == NULL) {
        LOG_NULL(dst);
        return;
    }
    if (src == NULL) {
        LOG_NULL(src);
        return;
    }

    for (struct Node *node = src->head; node != NULL; node = node->next) {
        List_add(dst, node->value);
    }
}

MalDatum *List_ref(const List *list, size_t idx) { 
    if (list == NULL) {
        LOG_NULL(list);
        return NULL;
    }
    if (idx >= list->len)
        return NULL;

    size_t i = 0;
    struct Node *node = list->head;
    while (i < idx) {
        node = node->next;
        i++;
    }
    return node->value;
}

/* Frees the memory allocated for each Node of the list including the MalDatums they point to. */
void List_free(List *list) {
    if (list == NULL || list == &_empty_list) return;
    else if (list->head == NULL) {
        free(list);
    } 
    else {
        struct Node *node = list->head;
        while (node) {
            MalDatum_release(node->value);
            MalDatum_free(node->value);
            struct Node *p = node;
            node = node->next;
            free(p);
        }
        free(list);
    }
}

bool List_eq(const List *lst1, const List *lst2) {
    if (lst1 == lst2) return true;
    if (lst1->len != lst2->len) return false;

    struct Node *node1 = lst1->head;
    struct Node *node2 = lst2->head;
    while (node1 != NULL) {
        if (!MalDatum_eq(node1->value, node2->value))
            return false;
        node1 = node1->next;
        node2 = node2->next;
    }
    
    return true;
}

// Symbol ----------------------------------------
Symbol *Symbol_new(const char *name) {
    Symbol *sym = malloc(sizeof(Symbol));
    strcpy(sym->name, name);
    return sym;
}

void Symbol_free(Symbol *symbol) {
    free(symbol);
}

// TODO intern all symbols
bool Symbol_eq(const Symbol *sym1, const Symbol *sym2) {
    if (sym1 == NULL) {
        LOG_NULL(sym1);
        return false;
    }
    if (sym2 == NULL) {
        LOG_NULL(sym1);
        return false;
    }

    return sym1 == sym2 || strcmp(sym1->name, sym2->name) == 0;
}

bool Symbol_eq_str(const Symbol *sym, const char *str) {
    if (sym == NULL) {
        LOG_NULL(sym);
        return false;
    }
    if (str == NULL) {
        LOG_NULL(str);
        return false;
    }

    return strcmp(sym->name, str) == 0;
}

Symbol *Symbol_copy(const Symbol *sym) {
    if (sym == NULL) {
        LOG_NULL(sym);
        return NULL;
    }

    return Symbol_new(sym->name);
}

// Procedures ----------------------------------------
Proc *Proc_new(const char *name, 
        int argc, bool variadic, 
        const Arr *params, const Arr *body, 
        MalEnv *env) 
{
    Proc *proc = malloc(sizeof(Proc));
    proc->name = dyn_strcpy(name);
    proc->argc = argc;
    proc->variadic = variadic;
    proc->params = Arr_copy(params, (copier_t) Symbol_copy);
    proc->builtin = false;
    proc->macro = false;
    {
        Arr *proc_body = Arr_copy(body, (copier_t) MalDatum_deep_copy);
        Arr_foreach(proc_body, (unary_void_t) MalDatum_own);
        proc->logic.body = proc_body;
    }
    proc->env = env;
    MalEnv_own(env);

    return proc;
}

Proc *Proc_builtin(const char *name, int argc, bool variadic, const builtin_apply_t apply) {
    Proc *proc = malloc(sizeof(Proc));
    proc->name = dyn_strcpy(name);
    proc->argc = argc;
    proc->variadic = variadic;
    proc->builtin = true;
    proc->macro = false;
    proc->logic.apply = apply;
    proc->env = NULL;
    return proc;
}

Proc *Proc_new_lambda(int argc, bool variadic, const Arr *params, const Arr *body, MalEnv *env) 
{
    Proc *proc = malloc(sizeof(Proc));
    proc->name = NULL;
    proc->argc = argc;
    proc->variadic = variadic;
    proc->params = Arr_copy(params, (copier_t) Symbol_copy);
    proc->builtin = false;
    proc->macro = false;
    {
        Arr *proc_body = Arr_copy(body, (copier_t) MalDatum_deep_copy);
        Arr_foreach(proc_body, (unary_void_t) MalDatum_own);
        proc->logic.body = proc_body;
    }
    proc->env = env;
    MalEnv_own(env);
    return proc;
}

char *Proc_name(const Proc *proc)
{
    static const char* lambda_name = "*lambda*";
    if (proc == NULL) {
        LOG_NULL(proc);
        return NULL;
    }

    const char* name = proc->name ? proc->name : lambda_name;
    return dyn_strcpy(name);
}

bool Proc_is_named(const Proc *proc)
{
    return proc->name != NULL;
}

bool Proc_is_macro(const Proc *proc)
{
    return proc->macro;
}

void Proc_free(Proc *proc) {
    if (proc == NULL) {
        LOG_NULL(proc);
        return;
    }

    if (!proc->builtin) {
        // free params
        Arr_freep(proc->params, (free_t) Symbol_free);
        // free body
        Arr_freep(proc->logic.body, (free_t) MalDatum_release_free);
    }

    if (proc->name) 
        free(proc->name);

    MalEnv_release(proc->env);
    MalEnv_free(proc->env);

    free(proc);
}

Proc *Proc_copy(const Proc *proc) {
    if (proc == NULL) {
        LOG_NULL(proc);
        return NULL;
    }

    if (proc->builtin) {
        return Proc_builtin(proc->name, proc->argc, proc->variadic, proc->logic.apply);
    }
    else if (proc->name) { // named procedure?
        return Proc_new(proc->name, proc->argc, proc->variadic, proc->params, proc->logic.body, 
                proc->env);
    }
    else { // lambda
        return Proc_new_lambda(proc->argc, proc->variadic, proc->params, proc->logic.body, 
                proc->env);
    }
}

bool Proc_eq(const Proc *proc1, const Proc *proc2) {
    return proc1 == proc2;
}


// Atom -----------------------------------------------------------------------
Atom *Atom_new(MalDatum *dtm)
{
    if (!dtm) {
        LOG_NULL(dtm);
        return NULL;
    }

    Atom *atom = malloc(sizeof(Atom));
    atom->datum = dtm;
    MalDatum_own(dtm);
    return atom;
}

void Atom_free(Atom *atom)
{
    if (!atom) {
        LOG_NULL(atom);
        return;
    }

    MalDatum_release_free(atom->datum);
    atom->datum = NULL;
    free(atom);
}

Atom *Atom_copy(const Atom *atom)
{
    FATAL("UNIMPLEMENTED");
}

bool Atom_eq(const Atom *a1, const Atom *a2)
{
    if (!a1) {
        LOG_NULL(a1);
        return false;
    }
    if (!a2) {
        LOG_NULL(a2);
        return false;
    }

    return a1->datum == a2->datum;
}

void Atom_reset(Atom *atom, MalDatum *dtm) 
{
    if (!atom) {
        LOG_NULL(atom);
        return;
    }
    if (!dtm) {
        LOG_NULL(dtm);
        return;
    }

    if (atom->datum == dtm) return;

    MalDatum_release_free(atom->datum);
    atom->datum = dtm;
    MalDatum_own(dtm);
}

// Exception -------------------------------------------------------------------
Exception *Exception_new(const char *msg)
{
    Exception *exn = malloc(sizeof(Exception));
    exn->msg = dyn_strcpy(msg);
    return exn;
}

void Exception_free(Exception *exn)
{
    if (!exn) {
        LOG_NULL(exn);
        return;
    }
    free(exn->msg);
    free(exn);
}

Exception *Exception_copy(const Exception *exn)
{
    if (!exn) {
        LOG_NULL(exn);
        return NULL;
    }
    return Exception_new(exn->msg);
}

bool Exception_eq(const Exception *exn1, const Exception *exn2)
{
    return exn1 == exn2 
        || exn1->msg == exn2->msg
        || strcmp(exn1->msg, exn2->msg) == 0;
}

// global struct that stores the last raised exception
static Exception _Exception_last = {
    .msg = NULL
};

void Exception_last_store(const char *msg)
{
    if (_Exception_last.msg) {
        free(_Exception_last.msg);
    }
    _Exception_last.msg = dyn_strcpy(msg);
}

Exception *Exception_last_copy()
{
    if (!_Exception_last.msg) {
        LOG_NULL(_Excetpion_last.msg);
    }
    return Exception_copy(&_Exception_last);
}

// MalType ----------------------------------------
char *MalType_tostr(MalType type) {
    static char* const names[] = {
        "INT", 
        "SYMBOL", 
        "LIST", 
        "EMPTY_LIST",
        "STRING", 
        "NIL", 
        "TRUE", 
        "FALSE", 
        "PROCEDURE",
        "ATOM",
        "EXCEPTION",
        "*undefined*"
    };

    return names[type];
}

// singletons
/*const*/ static MalDatum _MalDatum_nil = { 
    .refc = 1,
    .type = NIL 
};
/*const*/ static MalDatum _MalDatum_true = { 
    .refc = 1,
    .type = TRUE 
};
/*const*/ static MalDatum _MalDatum_false = { 
    .refc = 1,
    .type = FALSE 
};

static MalDatum _MalDatum_empty_list = { 
    .refc = 1,
    .type = EMPTY_LIST, 
    .value.list = &_empty_list
};

MalDatum *MalDatum_nil() {
    return &_MalDatum_nil;
};
MalDatum *MalDatum_true() {
    return &_MalDatum_true;
};
MalDatum *MalDatum_false() {
    return &_MalDatum_false;
};
MalDatum *MalDatum_empty_list() {
    return &_MalDatum_empty_list;
}

MalDatum *MalDatum_new_int(const int i) {
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = INT;
    mdp->value.i = i;
    return mdp;
}

MalDatum *MalDatum_new_sym(Symbol *symbol) { 
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = SYMBOL;
    mdp->value.sym = symbol;
    return mdp;
}

MalDatum *MalDatum_new_list(List *list) {
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = LIST;
    mdp->value.list = list;
    return mdp;
}

// str is copied
MalDatum *MalDatum_new_string(const char *str) {
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = STRING;
    mdp->value.string = dyn_strcpy(str);
    return mdp;
}

MalDatum *MalDatum_new_proc(Proc *proc) {
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = PROCEDURE;
    mdp->value.proc = proc;
    return mdp;
}

MalDatum *MalDatum_new_atom(Atom *atom)
{
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = ATOM;
    mdp->value.atom = atom;
    return mdp;
}

MalDatum *MalDatum_new_exn(Exception *exn)
{
    MalDatum *mdp = malloc(sizeof(MalDatum));
    mdp->refc = 0;
    mdp->type = EXCEPTION;
    mdp->value.exn = exn;
    return mdp;
}

void MalDatum_own(MalDatum *datum) 
{
    if (datum == NULL) {
        LOG_NULL(datum);
        return;
    }

    datum->refc += 1;

    char *repr = pr_repr(datum);
    DEBUG("own %p (refc %ld) -> %s", datum, datum->refc, repr);
    free(repr);
}

void MalDatum_release(MalDatum *datum)
{
    if (datum == NULL) {
        LOG_NULL(datum);
        return;
    }
    if (datum->refc <= 0) {
        char *repr = pr_repr(datum);
        FATAL("Invalid ref count %ld @ %p -> %s", datum->refc, datum, repr);
        free(repr);
    }

    datum->refc -= 1;

    char *repr = pr_repr(datum);
    DEBUG("release %p (refc %ld) -> %s", datum, datum->refc, repr);
    free(repr);
}

void MalDatum_free(MalDatum *datum) {
    if (datum == NULL) return;
    if (datum->refc > 0) {
        DEBUG("Refuse to free %p (refc %ld)", datum, datum->refc);
        return;
    }

    switch (datum->type) {
        // singletons will always have ref count >= 1
        // but let's be safe
        case NIL: 
            DEBUG("WTF? freeing NIL");
            return;
        case TRUE: 
            DEBUG("WTF? freeing TRUE");
            return;
        case FALSE: 
            DEBUG("WTF? freeing FALSE");
            return;
        case EMPTY_LIST: 
            DEBUG("WTF? freeing EMPTY_LIST");
            return;
        case INT: break;
        case LIST:
            List_free(datum->value.list);
            break;
        case STRING:
            free(datum->value.string);
            break;
        case SYMBOL:
            Symbol_free(datum->value.sym);
            break;
        case PROCEDURE:
            Proc_free(datum->value.proc);
            break;
        case ATOM:
            Atom_free(datum->value.atom);
            break;
        case EXCEPTION:
            Exception_free(datum->value.exn);
            break;
        default:
            DEBUG("WTF? freeing %s", MalType_tostr(datum->type));
            break;
    }

    FREE(datum);
    free(datum);
}

void MalDatum_release_free(MalDatum *dtm)
{
    if (dtm == NULL) {
        LOG_NULL(dtm);
        return;
    }

    MalDatum_release(dtm);
    MalDatum_free(dtm);
}

bool MalDatum_istype(const MalDatum *datum, MalType type) {
    return datum->type == type;
}

bool MalDatum_islist(const MalDatum *datum) {
    return datum->type == LIST || datum->type == EMPTY_LIST;
}

MalDatum *MalDatum_copy(const MalDatum *datum) {
    if (datum == NULL) {
        LOG_NULL(datum);
        return NULL;
    };

    MalDatum *out = NULL;

    switch (datum->type) {
        case INT:
            out = MalDatum_new_int(datum->value.i);
            break;
        case SYMBOL:
            out = MalDatum_new_sym(Symbol_new(datum->value.sym->name));
            break;
        case STRING:
            out = MalDatum_new_string(datum->value.string);
            break;
        case EMPTY_LIST:
            out = MalDatum_empty_list();
            break;
        case LIST:
            out = MalDatum_new_list(List_copy(datum->value.list));
            break;
        case NIL:
            out = MalDatum_nil();
            break;
        case TRUE:
            out = MalDatum_true();
            break;
        case FALSE:
            out = MalDatum_false();
            break;
        case PROCEDURE:
            out = MalDatum_new_proc(Proc_copy(datum->value.proc));
            break;
        case ATOM:
            out = MalDatum_new_atom(Atom_copy(datum->value.atom));
            break;
        case EXCEPTION:
            out = MalDatum_new_exn(Exception_copy(datum->value.exn));
            break;
        default:
            FATAL("unknown MalType");
            break;
    }

    // this is a completely new object, so reset ref count to 0
    out->refc = 0;

    return out;
}

MalDatum *MalDatum_deep_copy(const MalDatum *datum) {
    if (datum == NULL) {
        LOG_NULL(datum);
        return NULL;
    };

    MalDatum *out = NULL;

    switch (datum->type) {
        case LIST:
            out = MalDatum_new_list(List_deep_copy(datum->value.list));
            break;
        default:
            out = MalDatum_copy(datum);
            break;
    }

    return out;
}

bool MalDatum_isnil(MalDatum *datum) {
    if (datum == NULL) {
        LOG_NULL(datum);
        return false;
    }
    return datum->type == NIL;
}

bool MalDatum_isfalse(MalDatum *datum) {
    if (datum == NULL) {
        LOG_NULL(datum);
        return false;
    }
    return datum->type == FALSE;
}

bool MalDatum_eq(const MalDatum *md1, const MalDatum *md2) {
    if (md1 == NULL) {
        LOG_NULL(md1);
        return false;
    }
    if (md2 == NULL) {
        LOG_NULL(md2);
        return false;
    }

    if (md1->type != md2->type)
        return false;

    switch (md1->type) {
        case INT:
            return md1->value.i == md2->value.i;
        case SYMBOL:
            return Symbol_eq(md1->value.sym, md2->value.sym);
        case STRING:
            return strcmp(md1->value.string, md2->value.string) == 0;
        case LIST:
            return List_eq(md1->value.list, md2->value.list);
        case EMPTY_LIST:
            return true;
        case NIL:
            return true;
        case TRUE:
            return true;
        case FALSE:
            return true;
        case PROCEDURE:
            return Proc_eq(md1->value.proc, md2->value.proc);
        case ATOM:
            return Atom_eq(md1->value.atom, md2->value.atom);
        case EXCEPTION:
            return Exception_eq(md1->value.exn, md2->value.exn);
        default:
            FATAL("unknown MalType");
    }
}
