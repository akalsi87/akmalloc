# TEST: sub module

add_test_exe    (tBasic ./tBasic.cpp)
test_link_libs  (tBasic )
create_test     (tBasic)

add_dependencies(check akmalloc)

