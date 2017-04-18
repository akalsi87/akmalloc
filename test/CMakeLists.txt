# TEST: sub module

add_dependencies(check akmalloc)

macro(test_create testname)
  create_test     (${testname})
  # add -Wno-unused-function as not all tests exercise
  # all functions
  if (NOT is_msvc)
    add_comp_flag(${testname} -Wno-unused-function)
    add_comp_flag(${testname} -Wno-unused-variable)
    add_comp_flag(${testname} -std=c++11)
    add_comp_flag(${testname} -pthread)
    add_link_flag(${testname} -pthread)
  endif()
endmacro(test_create)

if (AKMALLOC_LIBRARY OR AKMALLOC_LINK_STATIC)
  set(to_link akmalloc)
  if (is_msvc)
    add_definitions(-DAKMALLOC_USE_PREFIX=1)
  endif()
  set(compdef -D__UNUSED_AKMALLOC_LIBS_ALREADY_USE_LOCKS_BY_DEFAULT)
else()
  set(compdef -DAKMALLOC_USE_LOCKS)
  set(to_link )
endif()

if ((NOT AKMALLOC_LIBRARY) AND (NOT AKMALLOC_LINK_STATIC))
add_test_exe    (tConfig tConfig.cpp)
test_link_libs  (tConfig )
test_create     (tConfig)
endif()

add_test_exe    (tMapUnmap tMapUnmap.cpp)
test_link_libs  (tMapUnmap )
test_create     (tMapUnmap)

add_test_exe    (tBitSet tBitSet.cpp)
test_link_libs  (tBitSet )
test_create     (tBitSet)

add_test_exe    (tSlab tSlab.cpp)
test_link_libs  (tSlab )
test_create     (tSlab)

add_test_exe    (tCoalescingAlloc tCoalescingAlloc.cpp)
test_link_libs  (tCoalescingAlloc )
test_create     (tCoalescingAlloc)

# if (NOT WIN32) # UNIX test
# add_test_exe    (malloctest malloctest.c)
# test_link_libs  (malloctest ${to_link})
# test_create     (malloctest)
# endif()

add_test_exe    (tMallocPerf tMallocPerf.cpp)
add_comp_def    (tMallocPerf ${compdef})
test_link_libs  (tMallocPerf ${to_link})
test_create     (tMallocPerf)

add_test_exe    (tMallocPerfRef tMallocPerf.cpp)
add_comp_def    (tMallocPerfRef -DUSE_MALLOC=1)
test_link_libs  (tMallocPerfRef )
test_create     (tMallocPerfRef)
