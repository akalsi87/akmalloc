# header only library : akmalloc
# -- Headers

# update the rc.h.template file when the version is bumped
set(akmalloc_major 1)
set(akmalloc_minor 0)
set(akmalloc_patch 1)

set(xprt include/akmalloc)

# create the configured rc file
get_filename_component(rc_dirpath ${xprt}/rc.h.template DIRECTORY)
get_filename_component(rc_absdirpath ${rc_dirpath} ABSOLUTE)
configure_file(${xprt}/rc.h.template
               ${rc_absdirpath}/rc.h
               @ONLY
               )

set(akmalloc_impl
    ${xprt}/malloc.c
    )

# export
set(akmalloc_exp_hdr
    ${xprt}/assert.h;
    ${xprt}/coalescingalloc.h;
    ${xprt}/config.h;
    ${xprt}/constants.h;
    ${xprt}/inline.h;
    ${xprt}/malloc.h;
    ${xprt}/mallocstate.h;
    ${xprt}/memmap.h;
    ${xprt}/types.h;
    ${xprt}/setup.h;
    ${xprt}/slab.h;
    ${xprt}/spinlock.h;
    ${xprt}/rc.h;
    # fix exportsym being missing
    ${xprt}/exportsym.h
    )

set(akmalloc_exp_hdr ${akmalloc_exp_hdr})

set(akmalloc_all_src ${akmalloc_exp_hdr};${akmalloc_impl})
if (AKMALLOC_LIBRARY)
  add_lib(akmalloc SHARED ${akmalloc_impl})
  add_comp_def(akmalloc -DAKMALLOC_USE_LOCKS)
  if (is_msvc)
    add_comp_def(akmalloc -DAKMALLOC_USE_PREFIX=1)
    #projmsg("Cannot build this project as a library on Windows overriding default names. Using AKMALLOC_USE_PREFIX")
  else()
    add_comp_flag(akmalloc "-Wno-unused-function")
  endif()
  add_lib_build_def(akmalloc include/akmalloc/exportsym.h AKMALLOC)
  link_libs(akmalloc )
  set_tgt_ver(akmalloc "${akmalloc_major}.${akmalloc_minor}.${akmalloc_patch}" "${akmalloc_major}.${akmalloc_minor}")
  install_tgt(akmalloc)
else()
  if (AKMALLOC_LINK_STATIC)
    add_lib(akmalloc STATIC ${akmalloc_impl})
    add_comp_def(akmalloc -DAKMALLOC_USE_LOCKS)
    if (is_msvc)
      add_comp_def(akmalloc -DAKMALLOC_USE_PREFIX=1)
      #projmsg("Cannot build this project as a library on Windows overriding default names. Using AKMALLOC_USE_PREFIX")
    else()
      add_comp_flag(akmalloc "-Wno-unused-function")
    endif()
    add_lib_build_def(akmalloc include/akmalloc/exportsym.h AKMALLOC)
    link_libs(akmalloc )
    set_tgt_ver(akmalloc "${akmalloc_major}.${akmalloc_minor}.${akmalloc_patch}" "${akmalloc_major}.${akmalloc_minor}")
    install_tgt(akmalloc)
  else()
    list(APPEND akmalloc_exp_hdr ${akmalloc_impl})
    set(akmalloc_all_src ${akmalloc_exp_hdr})
    add_custom_target(akmalloc SOURCES ${akmalloc_exp_hdr})
  endif()
endif()

# -- Install!
install_hdr(${akmalloc_exp_hdr})
