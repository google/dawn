#version 310 es

ivec4 tint_count_leading_zeros(ivec4 v) {
  uvec4 x = uvec4(v);
  uvec4 b16 = mix(uvec4(0u), uvec4(16u), lessThanEqual(x, uvec4(65535u)));
  x = (x << b16);
  uvec4 b8 = mix(uvec4(0u), uvec4(8u), lessThanEqual(x, uvec4(16777215u)));
  x = (x << b8);
  uvec4 b4 = mix(uvec4(0u), uvec4(4u), lessThanEqual(x, uvec4(268435455u)));
  x = (x << b4);
  uvec4 b2 = mix(uvec4(0u), uvec4(2u), lessThanEqual(x, uvec4(1073741823u)));
  x = (x << b2);
  uvec4 b1 = mix(uvec4(0u), uvec4(1u), lessThanEqual(x, uvec4(2147483647u)));
  uvec4 is_zero = mix(uvec4(0u), uvec4(1u), equal(x, uvec4(0u)));
  return ivec4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

void countLeadingZeros_eab32b() {
  ivec4 arg_0 = ivec4(1);
  ivec4 res = tint_count_leading_zeros(arg_0);
}

vec4 vertex_main() {
  countLeadingZeros_eab32b();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

ivec4 tint_count_leading_zeros(ivec4 v) {
  uvec4 x = uvec4(v);
  uvec4 b16 = mix(uvec4(0u), uvec4(16u), lessThanEqual(x, uvec4(65535u)));
  x = (x << b16);
  uvec4 b8 = mix(uvec4(0u), uvec4(8u), lessThanEqual(x, uvec4(16777215u)));
  x = (x << b8);
  uvec4 b4 = mix(uvec4(0u), uvec4(4u), lessThanEqual(x, uvec4(268435455u)));
  x = (x << b4);
  uvec4 b2 = mix(uvec4(0u), uvec4(2u), lessThanEqual(x, uvec4(1073741823u)));
  x = (x << b2);
  uvec4 b1 = mix(uvec4(0u), uvec4(1u), lessThanEqual(x, uvec4(2147483647u)));
  uvec4 is_zero = mix(uvec4(0u), uvec4(1u), equal(x, uvec4(0u)));
  return ivec4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

void countLeadingZeros_eab32b() {
  ivec4 arg_0 = ivec4(1);
  ivec4 res = tint_count_leading_zeros(arg_0);
}

void fragment_main() {
  countLeadingZeros_eab32b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

ivec4 tint_count_leading_zeros(ivec4 v) {
  uvec4 x = uvec4(v);
  uvec4 b16 = mix(uvec4(0u), uvec4(16u), lessThanEqual(x, uvec4(65535u)));
  x = (x << b16);
  uvec4 b8 = mix(uvec4(0u), uvec4(8u), lessThanEqual(x, uvec4(16777215u)));
  x = (x << b8);
  uvec4 b4 = mix(uvec4(0u), uvec4(4u), lessThanEqual(x, uvec4(268435455u)));
  x = (x << b4);
  uvec4 b2 = mix(uvec4(0u), uvec4(2u), lessThanEqual(x, uvec4(1073741823u)));
  x = (x << b2);
  uvec4 b1 = mix(uvec4(0u), uvec4(1u), lessThanEqual(x, uvec4(2147483647u)));
  uvec4 is_zero = mix(uvec4(0u), uvec4(1u), equal(x, uvec4(0u)));
  return ivec4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

void countLeadingZeros_eab32b() {
  ivec4 arg_0 = ivec4(1);
  ivec4 res = tint_count_leading_zeros(arg_0);
}

void compute_main() {
  countLeadingZeros_eab32b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
