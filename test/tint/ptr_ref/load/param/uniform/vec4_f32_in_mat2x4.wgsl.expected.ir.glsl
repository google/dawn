#version 310 es

uniform mat2x4 S;
vec4 func(uint pointer_indices[1]) {
  return S[pointer_indices[0u]];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func(uint[1](uint(1)));
}
