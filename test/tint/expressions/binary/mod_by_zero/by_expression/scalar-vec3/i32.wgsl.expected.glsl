#version 310 es

ivec3 tint_select(ivec3 param_0, ivec3 param_1, bvec3 param_2) {
    return ivec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


ivec3 tint_mod(int lhs, ivec3 rhs) {
  ivec3 l = ivec3(lhs);
  ivec3 rhs_or_one = tint_select(rhs, ivec3(1), bvec3(uvec3(equal(rhs, ivec3(0))) | uvec3(bvec3(uvec3(equal(l, ivec3((-2147483647 - 1)))) & uvec3(equal(rhs, ivec3(-1)))))));
  if (any(notEqual((uvec3((l | rhs_or_one)) & uvec3(2147483648u)), uvec3(0u)))) {
    return (l - ((l / rhs_or_one) * rhs_or_one));
  } else {
    return (l % rhs_or_one);
  }
}

void f() {
  int a = 4;
  ivec3 b = ivec3(0, 2, 0);
  ivec3 r = tint_mod(a, (b + b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
