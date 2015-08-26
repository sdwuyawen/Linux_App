#include <stdio.h>

// Find current frame address (i.e. __builtin_frame_address)
// The first four bytes(i.e. *fp) of frame points to program counter (PC), so skip it
// The next four bytes(i.e. *(fp – 1)) since stack grow downwards) is link register or return address of current function
// Using this we can find the parent/caller
// The fourth four bytes(i.e. *(fp – 3)) points to the previous/caller activation record
// This forms a linked list of activation records and end when *(fp – 3) is ‘0’

void backtrace(void* fp)
{
  if (fp == 0)
    return;

  fprintf (stderr, "%p\n", *((int*)fp - 1));
  backtrace ((void*)(*((int*)fp - 3)));
//  fprintf (stderr, "%p\n", *((int*)fp - 0));
//  backtrace ((void*)(*((int*)fp - 1)));
}

int bar()
{
  printf ("bar\n");
  printf ("*** backtrack ***\n");
  backtrace (__builtin_frame_address (0));
  printf ("\n");

  return 0;
}

int foo()
{
  printf ("foo\n");
  printf ("*** backtrack ***\n");
  backtrace (__builtin_frame_address (0));
  printf ("\n");

  return bar();
}

int main()
{
  printf ("main at %p\n", main);
  printf ("foo at %p\n", foo);
  printf ("bar at %p\n\n", bar);

  printf ("main\n");
  printf ("*** backtrack ***\n");
  backtrace (__builtin_frame_address (0));
  printf ("\n");
 
  return foo();
}
