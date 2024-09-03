#version 310 es

mat2x4 S;
void func(uint pointer_indices[1]) {
  S[pointer_indices[0u]] = vec4(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func(uint[1](uint(1)));
}
