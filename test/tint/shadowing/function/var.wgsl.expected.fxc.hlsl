struct a {
  int a;
};


[numthreads(1, 1, 1)]
void f() {
  a a_1 = (a)0;
  a b = a_1;
  a a_2 = (a)0;
  a b_1 = a_2;
}

