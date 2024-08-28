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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b();
  tint_symbol_1();
}
