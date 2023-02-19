#include <stdio.h>
#include <string.h>

int main() {
  int x; int y;
  int *iptr;
  printf( "%p\n", iptr);
  x= 10; y= 20;
  iptr= (&x)+1;
  printf( "%p, %p, %p\n", &x, *iptr, &y);
}
