#ifndef JAMOMAPDSYMBOLS_H
#define JAMOMAPDSYMBOLS_H

/*
    commonsyms.h
*/

typedef struct _common_symbols_table
{
    t_symbol *s_box;
    t_symbol *s_subpatcher;
    t_symbol *s_bpatcher;
    t_symbol *s_parentpatcher;
    t_symbol *s_pd;
    t_symbol *s_topmost;
    t_symbol *s_nothing;
    t_symbol *s_float;
    t_symbol *s_list;
    t_symbol *s_bang;
    t_symbol *s_int;
    t_symbol *s_symbol;

} t_common_symbols_table;

#define _sym_box (_common_symbols->s_box)
#define _sym_subpatcher (_common_symbols->s_subpatcher)
#define _sym_bpatcher (_common_symbols->s_bpatcher)
#define _sym_parentpatcher (_common_symbols->s_parentpatcher)
#define _sym_pd (_common_symbols->s_pd)
#define _sym_topmost (_common_symbols->s_topmost)
#define _sym_nothing (_common_symbols->s_nothing)
#define _sym_float (_common_symbols->s_float)
#define _sym_list (_common_symbols->s_list)
#define _sym_bang (_common_symbols->s_bang)
#define _sym_int (_common_symbols->s_int)
#define _sym_symbol (_common_symbols->s_symbol)

extern t_common_symbols_table *_common_symbols;

void common_symbols_init(void);
t_common_symbols_table *common_symbols_gettable(void);

#endif // JAMOMAPDSYMBOLS_H
