# header only library : akmalloc
# -- Headers

set(xprt include/akmalloc)

# export
set(akmalloc_exp_hdr
    ${xprt}/assert.h;
    ${xprt}/atomic.h;
    ${xprt}/config.h;
    ${xprt}/constants.h;
    ${xprt}/malloc.h;
    ${xprt}/memmap.h;
    ${xprt}/threadlocal.h;
    ${xprt}/types.h;
    ${xprt}/utils.h;
    )

add_custom_target(akmalloc SOURCES ${akmalloc_exp_hdr})

# -- Install!
install_hdr(${akmalloc_exp_hdr})
