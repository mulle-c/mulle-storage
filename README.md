# mulle-storage

#### 🛅 Memory management for tree nodes

Why not just use `malloc` ? **mulle-storage** can be useful as a storage for 
nodes of a tree. It's likely faster and it may produce less fragmentation and 
it may improve locality of reference. Freed nodes will be reused. All the tree
nodes can be "blown" away at once, without having to free each node
individually.

Why not use a `mulle_structarray` ?  A struct array can realloc and so pointers
inside the struct array are not stable.



| Release Version                                       | Release Notes  | AI Documentation
|-------------------------------------------------------|----------------|---------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-storage.svg) [![Build Status](https://github.com/mulle-c/mulle-storage/workflows/CI/badge.svg)](//github.com/mulle-c/mulle-storage/actions) | [RELEASENOTES](RELEASENOTES.md) | [DeepWiki for mulle-storage](https://deepwiki.com/mulle-c/mulle-storage)






### You are here

![Overview](overview.dot.svg)





## Quickstart

Install [mulle-core developer](https://github.com/MulleFoundation/foundation-developer?tab=readme-ov-file#install)
then:


``` sh
mulle-sde init -d my-project -m mulle-core/c-developer executable
cd my-project
mulle-sde run
```

## Add

**This project is a component of the [mulle-core](//github.com/mulle-core/mulle-core) library. As such you usually will *not* add or install it
individually, unless you specifically do not want to link against
`mulle-core`.**


### Add as an individual component

Use [mulle-sde](//github.com/mulle-sde) to add mulle-storage to your project:

``` sh
mulle-sde add github:mulle-c/mulle-storage
```

To only add the sources of mulle-storage with dependency
sources use [clib](https://github.com/clibs/clib):


``` sh
clib install --out src/mulle-c mulle-c/mulle-storage
```

Add `-isystem src/mulle-c` to your `CFLAGS` and compile all the sources that were downloaded with your project.


## Install

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-storage and all dependencies:

``` sh
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-c/mulle-storage/archive/latest.tar.gz
```

### Legacy Installation

Install the requirements:

| Requirements                                 | Description
|----------------------------------------------|-----------------------
| [mulle-container](https://github.com/mulle-c/mulle-container)             | 🛄 Arrays, hashtables and a queue

Download the latest [tar](https://github.com/mulle-c/mulle-storage/archive/refs/tags/latest.tar.gz) or [zip](https://github.com/mulle-c/mulle-storage/archive/refs/tags/latest.zip) archive and unpack it.

Install **mulle-storage** into `/usr/local` with [cmake](https://cmake.org):

``` sh
PREFIX_DIR="/usr/local"
cmake -B build                               \
      -DMULLE_SDK_PATH="${PREFIX_DIR}"       \
      -DCMAKE_INSTALL_PREFIX="${PREFIX_DIR}" \
      -DCMAKE_PREFIX_PATH="${PREFIX_DIR}"    \
      -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --config Release &&
cmake --install build --config Release
```


## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK  



