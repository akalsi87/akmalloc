# TEST: sub module

add_dependencies(check akmalloc)

macro(test_create testname)
  create_test     (${testname})
  # add -Wno-unused-function as not all tests exercise
  # all functions
  if (NOT is_msvc)
    add_comp_flag(${testname} "-Wno-unused-function")
  endif()
endmacro(test_create)

if (AKMALLOC_LIBRARY)
  add_definitions(-DAKMALLOC_BUILD)
  set(to_link akmalloc)
else()
  set(to_link )
endif()

add_test_exe    (tConfig tConfig.cpp)
test_link_libs  (tConfig ${to_link})
test_create     (tConfig)

add_test_exe    (tMapUnmap tMapUnmap.cpp)
test_link_libs  (tMapUnmap ${to_link})
test_create     (tMapUnmap)

add_test_exe    (tBitSet tBitSet.cpp)
test_link_libs  (tBitSet ${to_link})
test_create     (tBitSet)

if (NOT AKMALLOC_LIBRARY)
add_test_exe    (tSlab tSlab.cpp)
test_link_libs  (tSlab ${to_link})
test_create     (tSlab)
endif()