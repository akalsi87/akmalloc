# header only library : akmalloc
# -- Headers

set(xprt include/akmalloc)

# export
set(akmalloc_exp_hdr
    ${xprt}/assert.h;
    ${xprt}/atomic.h;
    ${xprt}/config.h;
    ${xprt}/malloc.h;
    ${xprt}/memmap.h;
    ${xprt}/threadlocal.h;
    ${xprt}/types.h;
    )

add_lib(akmalloc ${akmalloc_exp_hdr})
set_target_properties(akmalloc PROPERTIES LINKER_LANGUAGE C)

# -- Install!
install_hdr(${akmalloc_exp_hdr})
