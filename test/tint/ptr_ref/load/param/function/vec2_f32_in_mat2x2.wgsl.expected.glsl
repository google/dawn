#version 310 es

vec2 func(inout vec2 pointer) {
  return pointer;
}

void tint_symbol() {
  mat2 F = mat2(0.0f, 0.0f, 0.0f, 0.0f);
  vec2 r = func(F[1]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
