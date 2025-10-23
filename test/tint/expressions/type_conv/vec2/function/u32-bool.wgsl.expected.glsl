#version 310 es

uint t = 0u;
uvec2 m() {
  t = 1u;
  return uvec2(t);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bvec2 v = bvec2(m());
}
