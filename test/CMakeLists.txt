# TEST: sub module

add_dependencies(check akmalloc)

add_test_exe    (tConfig tConfig.cpp)
test_link_libs  (tConfig )
create_test     (tConfig)
if (NOT is_msvc)
  add_comp_flag(tConfig "-Wno-unused-function")
endif()

add_test_exe    (tMapUnmap tMapUnmap.cpp)
test_link_libs  (tMapUnmap )
create_test     (tMapUnmap)
if (NOT is_msvc)
  add_comp_flag(tConfig "-Wno-unused-function")
endif()
