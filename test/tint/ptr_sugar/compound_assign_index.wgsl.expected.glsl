#version 310 es

void deref() {
  ivec3 a = ivec3(0, 0, 0);
  int tint_symbol_2 = 0;
  a[tint_symbol_2] = (a[tint_symbol_2] + 42);
}

void no_deref() {
  ivec3 a = ivec3(0, 0, 0);
  int tint_symbol_4 = 0;
  a[tint_symbol_4] = (a[tint_symbol_4] + 42);
}

void deref_inc() {
  ivec3 a = ivec3(0, 0, 0);
  int tint_symbol_6 = 0;
  a[tint_symbol_6] = (a[tint_symbol_6] + 1);
}

void no_deref_inc() {
  ivec3 a = ivec3(0, 0, 0);
  int tint_symbol_8 = 0;
  a[tint_symbol_8] = (a[tint_symbol_8] + 1);
}

void tint_symbol() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
