#version 310 es

void f() {
  bvec3 a = bvec3(true, true, false);
  bvec3 b = bvec3(true, false, true);
  bvec3 r = bvec3(uvec3(a) & uvec3(b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
