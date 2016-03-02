# TEST: sub module

add_dependencies(check akmalloc)

add_test_exe    (tConfig ./tConfig.cpp)
test_link_libs  (tConfig )
create_test     (tConfig)
