void set_int4(inout int4 vec, int idx, int val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

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
  const int tint_symbol_save = foo();
  const int tint_symbol_1 = bar();
  {
    int4 tint_symbol_3[4] = x.a;
    set_int4(tint_symbol_3[tint_symbol_save], tint_symbol_1, (x.a[tint_symbol_save][tint_symbol_1] + 5));
    x.a = tint_symbol_3;
  }
}
