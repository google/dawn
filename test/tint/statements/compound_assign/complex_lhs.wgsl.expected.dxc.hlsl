struct S {
  int4 a[4];
};


static int counter = int(0);
int foo() {
  counter = asint((asuint(counter) + 1u));
  return counter;
}

int bar() {
  counter = asint((asuint(counter) + 2u));
  return counter;
}

[numthreads(1, 1, 1)]
void main() {
  S x = (S)0;
  uint v = min(uint(foo()), 3u);
  int v_1 = bar();
  int v_2 = asint((asuint(x.a[v][min(uint(v_1), 3u)]) + 5u));
  x.a[v][min(uint(v_1), 3u)] = v_2;
}

