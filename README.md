# Traversing the call stack

First off, read this blog post:
[http://yosefk.com/blog/getting-the-call-stack-without-a-frame-pointer.html](http://yosefk.com/blog/getting-the-call-stack-without-a-frame-pointer.html).

We aware that the example in this repository will not work in most situations. From the top of my head, I assume the following:

* C calling convention
* No `-fomit-frame-pointer`
* x86
* Possibly more.

## Short introduction

C calling convention declares that every function must *preserve* the frame base pointer (`%ebp`) on the stack.

This means that upon entering a function, `foo(int, int)`, the stack may look something like this:

| Addr  | Value           |
|-------|-----------------|
| 0xAF  | arg2            |
| 0xAB  | arg1            |
| 0xA7  | return address  |

After preserving the `%ebx` register, the stack would look like:

| Addr  | Value           |
|-------|-----------------|
| 0xAF  | arg2            |
| 0xAB  | arg1            |
| 0xA7  | return address  |
| 0xA3  | %ebp            |

After the function prologue, foo's `%ebp` register will point to `0xA3` and 
the return address is located at `%ebx + 4` (because a long is 4 bytes). 
The same applies for whoever called `foo`.

Let's imagine that the function `bar` called `foo`. Upon entering `foo`, the `%ebp` register will be 
pointing to the memory address where `bar` pushed *its* incoming `%ebp`!

To summarize:

* `foo`'s `%ebp` register is pointing to a memory location where `foo` preserved its incoming `%ebp` register
* The address at which `foo` should return, is stored at `%ebp + 4`

We can then get the call stack manually, like so:

```c
    void foo() {
        uint32_t ebp, ret_addr_loc, ret_addr;
        __asm__("movl %%ebp, %[frame]" : /* output */ [frame] "=r" (ebp));
    
        printf("Foo's %%ebp is stored at 0x%x\n", ebp);
        ret_addr_loc = ebp + sizeof(uint32_t); 
        printf("Foo's return address is stored at: 0x%x\n", ret_addr_loc);
        ret_addr = *((uint32_t*)ret_addr_loc);
        printf("Foo should return to 0x%x\n", ret_addr);
        
        uint32_t caller_ebp = *((uint32_t*)ebp);
        printf("Foo's caller's %%ebp is stored at 0x%x\n", caller_ebp);
        ret_addr_loc = caller_ebp + sizeof(uint32_t);
        printf("Foo's caller's return address is stored at: 0x%x\n", ret_addr_loc);
        ret_addr = *((uint32_t*)ret_addr_loc);
        printf("Foo's caller should return to 0x%x\n", ret_addr);
        
        /* ... and so on */
    }
```

Do you spot the linked list? Because `%ebp` basically is a pointer to a higher memory address, 
we can dereference the pointer until we reach the top (which would be the `main` function).

`main` in turn is usually called from an assembly function `start` which is the actual starting point of a process. 
But C calling convention does not apply there, the `%ebp`will then have a value of `0`. 
The x86 ABI suggests this be done, to mark the outermost frame. 
By using this information, we can traverse the call stack until the `%ebp` points to `0`.
