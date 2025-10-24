struct a {
  int a;
};


void f(a a_1) {
  a b = a_1;
}

[numthreads(1, 1, 1)]
void main() {
  a v = (a)0;
  f(v);
}

