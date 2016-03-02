# header only library : akmalloc
# -- Headers

set(xprt include/akmalloc)

# export
set(akmalloc_exp_hdr
    ${xprt}/atomic.h;
    ${xprt}/config.h;
    ${xprt}/malloc.h
    ${xprt}/memmap.h
    ${xprt}/threadlocal.h;
    )

# -- Install!
install_hdr(${akmalloc_exp_hdr})
