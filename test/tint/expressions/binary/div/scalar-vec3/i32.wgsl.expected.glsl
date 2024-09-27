#version 310 es

ivec3 tint_div(int lhs, ivec3 rhs) {
  ivec3 l = ivec3(lhs);
  return (l / mix(rhs, ivec3(1), bvec3(uvec3(equal(rhs, ivec3(0))) | uvec3(bvec3(uvec3(equal(l, ivec3((-2147483647 - 1)))) & uvec3(equal(rhs, ivec3(-1))))))));
}

void f() {
  int a = 4;
  ivec3 b = ivec3(1, 2, 3);
  ivec3 r = tint_div(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
