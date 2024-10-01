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
  int4 v = x.a[foo()];
  int v_1 = bar();
  v[v_1] = (v[v_1] + int(5));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

