#include <stdio.h>
#include <stdint.h>

/*
  Must be compiled for x86 using cdecl
 */

/*
  When a program is compiled with GCC without the -fomit-frame-pointer,
  the base pointer register is preserved upon function entry.

  If we have two functions, "foo" and "bar", and "foo" makes a call to "bar", then:
  - foo will push %ebp upon function entry, and move %esp into %ebp
  - foo will push the return address onto stack, prior to calling "bar"
  - foo will jump to "bar"
  - bar will push %ebp upon function entry, and move %esp into %ebp

  If we are inside "bar", we can traverse the stack frames using %ebp:
  cur_frame = %ebp
  next_frame = %ebp + sizeof(uint32_t)

  Because the stack grows downwards, the "next_frame" (or old %ebp), will have a
  higher address than "cur_frame".

  This structure forms a sort of a linked list, which we can traverse using C.
 */

typedef struct _frame {
  /* "prev" must be declared before "stack_frame", as the previous %ebp
    is stored at a higher address than the current %ebp */
  struct _frame* prev;
  uint32_t return_addr;
} __attribute__((packed)) stack_frame_t;

void bar()
{
  /* %ebp now points to the value of %esp, just after pushing "old" %ebp */

  /* x86/gcc-specific: this tells gcc that the fp
     variable should be an alias to the %ebp register
     which keeps the frame pointer */
  stack_frame_t* frame;

  __asm__("movl %%ebp, %[frame]" : /* output */ [frame] "=r" (frame));

  printf("#0: %p\n", frame);

  int i = 1;
  while (frame->prev) {
    printf("#%d: 0x%x\n", i++, frame->return_addr);
    frame = frame->prev;
  }
}

void foo()
{
  /* if compiled with optimizations, this function call will produce a tail-call,
    meaning that foo will just contain "jmp bar" */
  bar();
}

int main()
{
  foo();

  return 0;
}
