struct B {
  int b;
};

struct A {
  int a;
};


B f(A a) {
  B v = (B)0;
  return v;
}

[numthreads(1, 1, 1)]
void main() {
  A v_1 = {int(1)};
  f(v_1);
}

