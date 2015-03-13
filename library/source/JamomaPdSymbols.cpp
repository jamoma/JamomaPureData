#include "JamomaPdSymbols.h"

void common_symbol_init(){
    _common_symbols = new(_common_symbols_table);

    _common_symbols->s_box=gensym("box");
    _common_symbols->s_subpatcher=gensym("subpatcher");
    _common_symbols->s_bpatcher=gensym("bpatcher");
    _common_symbols->s_parentpatcher=gensym("parentpatcher");
    _common_symbols->s_pd=gensym("pd");
    _common_symbols->s_topmost=gensym("topmost");
    _common_symbols->s_nothing=gensym("nothing");
    _common_symbols->s_float=gensym("float");
    _common_symbols->s_list=gensym("list");
    _common_symbols->s_bang=gensym("bang");
    _common_symbols->s_int=gensym("int");
    _common_symbols->s_symbol=gensym("symbol");
}
