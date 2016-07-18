akmalloc is a customizable memory allocator and its constituent parts. It can be a drop in replacement for <tt>malloc()</tt> and <tt>free()</tt> in many cases, and is composed of slabs and coalescing allocators.

The inspiration and motivation for this library comes from <tt>dlmalloc</tt>.

The goals for this library are:

# Easy to read and maintain.
# Be more memory conserving.
# High efficiency and good performance.
# Portability.

The source code is under <tt>include/</tt> and documentation artifacts are under <tt>doc/html/</tt>.

[Documentation](https://rawgit.com/akalsi87/akmalloc/single-file/doc/html/index.html)