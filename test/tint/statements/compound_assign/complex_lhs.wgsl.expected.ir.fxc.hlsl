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
  int v = foo();
  int v_1 = bar();
  int4 v_2 = x.a[v];
  int4 v_3 = (x.a[v][v_1] + int(5)).xxxx;
  x.a[v] = (((v_1.xxxx == int4(int(0), int(1), int(2), int(3)))) ? (v_3) : (v_2));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

