#version 310 es

ivec3 tint_first_trailing_bit(ivec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(16u), uvec3(0u), bvec3((x & uvec3(65535u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(8u), uvec3(0u), bvec3((x & uvec3(255u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(4u), uvec3(0u), bvec3((x & uvec3(15u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(2u), uvec3(0u), bvec3((x & uvec3(3u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(1u), uvec3(0u), bvec3((x & uvec3(1u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_7496d6() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_first_trailing_bit(arg_0);
}

vec4 vertex_main() {
  firstTrailingBit_7496d6();
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

ivec3 tint_first_trailing_bit(ivec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(16u), uvec3(0u), bvec3((x & uvec3(65535u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(8u), uvec3(0u), bvec3((x & uvec3(255u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(4u), uvec3(0u), bvec3((x & uvec3(15u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(2u), uvec3(0u), bvec3((x & uvec3(3u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(1u), uvec3(0u), bvec3((x & uvec3(1u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_7496d6() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_first_trailing_bit(arg_0);
}

void fragment_main() {
  firstTrailingBit_7496d6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

ivec3 tint_first_trailing_bit(ivec3 v) {
  uvec3 x = uvec3(v);
  uvec3 b16 = mix(uvec3(16u), uvec3(0u), bvec3((x & uvec3(65535u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(8u), uvec3(0u), bvec3((x & uvec3(255u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(4u), uvec3(0u), bvec3((x & uvec3(15u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(2u), uvec3(0u), bvec3((x & uvec3(3u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(1u), uvec3(0u), bvec3((x & uvec3(1u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_7496d6() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_first_trailing_bit(arg_0);
}

void compute_main() {
  firstTrailingBit_7496d6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
