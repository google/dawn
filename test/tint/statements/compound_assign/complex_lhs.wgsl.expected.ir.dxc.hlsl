struct S {
  int4 a[4];
};


static int counter = 0;
int foo() {
  counter = (counter + 1);
  return counter;
}

int bar() {
  counter = (counter + 2);
  return counter;
}

void main() {
  S x = (S)0;
  S p = x;
  int4 v = p.a[foo()];
  int v_1 = bar();
  v[v_1] = (v[v_1] + 5);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

