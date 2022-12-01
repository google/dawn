#version 310 es

void func(inout vec4 pointer) {
  pointer = vec4(0.0f);
}

vec4 P = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void tint_symbol() {
  func(P);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
