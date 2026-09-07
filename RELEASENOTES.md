## 0.1.0





feature: add `mulle_arena` variable-size bump allocator

* new mulle-arena.h implementing a scoped bump/arena allocator for variable-sized allocations
* usable as a drop-in `mulle_allocator` via `MULLE_ALLOCATOR_BASE` for any code expecting an allocator
* allocator protocol semantics: calloc bumps + zeroes, realloc extends last allocation in place when possible, free is a no-op
* oversized allocations (> page size) get dedicated pages; alignment-aware allocation helpers
* reset API to rewind or keep a percentage of pages, plus strdup/memdup and NULL-safe wrappers
* submitted as a public header, exported in mulle-storage.h



feature: expose indexed storage element size and widen count to `size_t`

* new ``_mulle_indexedstorage_get_element_size`` API to query the element size
* ``mulle_storage_get_count`,` ``mulle_indexedstorage_get_count`` and the underscore variants now return ``size_t`` instead of `unsigned int`
* freed elements in the indexed storage debug path are now consistently filled with `0xDEADDEAD`



* new API reference at asset/dox/api/toc/ covering `mulle_storage` and `mulle_indexedstorage`
* usage examples for node allocation, reuse, copy, and indexed access patterns


### 0.0.6

Various small improvements
