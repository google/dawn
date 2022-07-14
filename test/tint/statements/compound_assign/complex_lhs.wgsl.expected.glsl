#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  ivec4 a[4];
};

int counter = 0;
int foo() {
  counter = (counter + 1);
  return counter;
}

int bar() {
  counter = (counter + 2);
  return counter;
}

void tint_symbol() {
  S x = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  int tint_symbol_3 = foo();
  int tint_symbol_1_save = tint_symbol_3;
  int tint_symbol_2 = bar();
  x.a[tint_symbol_1_save][tint_symbol_2] = (x.a[tint_symbol_1_save][tint_symbol_2] + 5);
}

