# malloc
Malloc project for coding school 42

* malloc man page

```
malloc()
    The malloc() function allocates size bytes and returns a pointer
    to the allocated memory.  The memory is not initialized.  If size
    is 0, then malloc() returns a unique pointer value that can later
    be successfully passed to free().  (See "Nonportable behavior" for
    portability issues.)

free()
   The free() function frees the memory space pointed to by ptr,
   which must have been returned by a previous call to malloc() or
   related functions.  Otherwise, or if ptr has already been freed,
   undefined behavior occurs.  If ptr is NULL, no operation is
   performed.

calloc()
   The calloc() function allocates memory for an array of n elements
   of size bytes each and returns a pointer to the allocated memory.
   The memory is set to zero.  If n or size is 0, then calloc()
   returns a unique pointer value that can later be successfully
   passed to free().

   If the multiplication of n and size would result in integer
   overflow, then calloc() returns an error.  By contrast, an integer
   overflow would not be detected in the following call to malloc(),
   with the result that an incorrectly sized block of memory would be
   allocated:

        malloc(n * size);

realloc()
   The realloc() function changes the size of the memory block
   pointed to by ptr to size bytes.  The contents of the memory will
   be unchanged in the range from the start of the region up to the
   minimum of the old and new sizes.  If the new size is larger than
   the old size, the added memory will not be initialized.

   If ptr is NULL, then the call is equivalent to malloc(size), for
   all values of size.

   If size is equal to zero, and ptr is not NULL, then the call is
   equivalent to free(ptr) (but see "Nonportable behavior" for
   portability issues).

   Unless ptr is NULL, it must have been returned by an earlier call
   to malloc or related functions.  If the area pointed to was moved,
   a free(ptr) is done.

reallocarray()
   The reallocarray() function changes the size of (and possibly
   moves) the memory block pointed to by ptr to be large enough for
   an array of n elements, each of which is size bytes.  It is
   equivalent to the call

        realloc(ptr, n * size);

    However, unlike that realloc() call, reallocarray() fails safely
    in the case where the multiplication would overflow.  If such an
    overflow occurs, reallocarray() returns an error.
```

This project aim to teach about kernel mapped region, and how to deal with them