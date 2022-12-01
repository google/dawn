#version 310 es

void func(inout vec2 pointer) {
  pointer = vec2(0.0f);
}

mat2 P = mat2(0.0f, 0.0f, 0.0f, 0.0f);
void tint_symbol() {
  func(P[1]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
