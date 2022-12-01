#version 310 es

vec4 func(inout vec4 pointer) {
  return pointer;
}

void tint_symbol() {
  mat2x4 F = mat2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec4 r = func(F[1]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
