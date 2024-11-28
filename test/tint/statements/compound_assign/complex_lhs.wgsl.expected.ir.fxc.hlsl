struct S {
  int4 a[4];
};


static int counter = int(0);
int foo() {
  counter = (counter + int(1));
  return counter;
}

int bar() {
  counter = (counter + int(2));
  return counter;
}

void main() {
  S x = (S)0;
  uint v = min(uint(foo()), 3u);
  int v_1 = bar();
  int v_2 = (x.a[v][min(uint(v_1), 3u)] + int(5));
  int4 v_3 = x.a[v];
  x.a[v] = (((v_1.xxxx == int4(int(0), int(1), int(2), int(3)))) ? (v_2.xxxx) : (v_3));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

