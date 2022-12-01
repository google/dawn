#version 310 es

vec2 func(inout vec2 pointer) {
  return pointer;
}

mat2 P = mat2(0.0f, 0.0f, 0.0f, 0.0f);
void tint_symbol() {
  vec2 r = func(P[1]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
