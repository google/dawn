#version 310 es

void a() {
}

void _a() {
}

void b() {
  a();
}

void _b() {
  _a();
}

void tint_symbol() {
  b();
  _b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
