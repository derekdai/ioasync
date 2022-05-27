test_coro = executable('test-coro',
                       'test-coro.c', 
                       link_with: ioasync,
                       include_directories: inc)

test_dlist = executable('test-dlist',
                       'test-dlist.c', 
                       link_with: ioasync,
                       include_directories: inc)

test('test-dlist', test_dlist)
