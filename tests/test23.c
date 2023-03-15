char *str;
int   x;

int main() {
  x= -23; printf(x);
  printf("%"-10 * -10);

  x= 1; x= ~x; printf(x);

  x= 2 > 5; printf(x);
  x= !x; printf(x);
  x= !x; printf(x);

  x= 13; if (x) { printf(13); }
  x= 0; if (!x) { printf(14); }

  for (str= "Hello world\n"; *str; str++) {
    printf(*str);
  }

  return(0);
}