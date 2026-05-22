## Bug 1: Use-after-free

### Description
The program accesses memory after it has been freed:
```c
free(p);
printf("%d\n", *p);
````

This is undefined behavior because the pointer `p` becomes invalid after `free()`.

### How it was found

* GDB debugging showed that `p` still contained an address after `free()`, but dereferencing it caused incorrect or unstable values.
* Valgrind reported:

  ```
  Invalid read of size 4
  Address ... is freed memory
  ```

### Fix

Do not access memory after freeing it. Either print before freeing or set pointer to NULL:

```c
printf("%d\n", *p);
free(p);
p = NULL;
```

---

## Bug 2: Missing malloc failure check

### Description

The result of `malloc()` is not checked before dereferencing:

```c
int *p = malloc(sizeof(int));
```

If allocation fails, `p` becomes `NULL`, leading to segmentation fault when accessed.

### How it was found

* In GDB, `p` was observed as `0x0` under simulated low-memory conditions.
* Attempting `*p` caused a segmentation fault.
* Valgrind indicated invalid memory access due to null pointer dereference.

### Fix

Always check the result of `malloc`:

```c
if (p == NULL) {
    fprintf(stderr, "malloc failed\n");
    return 1;
}
```

---

## Bug 3: Dangling pointer after free

### Description

After freeing memory, the pointer still holds the address of freed memory:

```c
free(p);
printf("%d\n", *p);
```

Even though the memory is freed, `p` is not reset, making accidental reuse dangerous.

### How it was found

* GDB showed that `p` still pointed to the same memory region after `free()`.
* Valgrind reported:

  ```
  Invalid read after free()
  ```

### Fix

Nullify pointer after freeing to avoid accidental access:

```c
free(p);
p = NULL;
```

and always guard access:

```c
if (p != NULL) {
    printf("%d\n", *p);
}
```

```

If you want, I can also :contentReference[oaicite:0]{index=0}, or :contentReference[oaicite:1]{index=1}.
```

