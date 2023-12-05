#version 310 es

void a() {
}

void tint_symbol() {
}

void b() {
  a();
}

void tint_symbol_1() {
  tint_symbol();
}

void tint_symbol_2() {
  b();
  tint_symbol_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2();
  return;
}
