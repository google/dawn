#version 310 es

ivec3 tint_mod(ivec3 lhs, ivec3 rhs) {
  ivec3 rhs_or_one = mix(rhs, ivec3(1), bvec3(uvec3(equal(rhs, ivec3(0))) | uvec3(bvec3(uvec3(equal(lhs, ivec3((-2147483647 - 1)))) & uvec3(equal(rhs, ivec3(-1)))))));
  if (any(notEqual((uvec3((lhs | rhs_or_one)) & uvec3(2147483648u)), uvec3(0u)))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void f() {
  ivec3 a = ivec3(1, 2, 3);
  ivec3 b = ivec3(0, 5, 0);
  ivec3 r = tint_mod(a, (b + b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
