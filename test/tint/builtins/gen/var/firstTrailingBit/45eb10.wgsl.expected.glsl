#version 310 es

uvec2 tint_first_trailing_bit(uvec2 v) {
  uvec2 x = uvec2(v);
  uvec2 b16 = mix(uvec2(16u), uvec2(0u), bvec2((x & uvec2(65535u))));
  x = (x >> b16);
  uvec2 b8 = mix(uvec2(8u), uvec2(0u), bvec2((x & uvec2(255u))));
  x = (x >> b8);
  uvec2 b4 = mix(uvec2(4u), uvec2(0u), bvec2((x & uvec2(15u))));
  x = (x >> b4);
  uvec2 b2 = mix(uvec2(2u), uvec2(0u), bvec2((x & uvec2(3u))));
  x = (x >> b2);
  uvec2 b1 = mix(uvec2(1u), uvec2(0u), bvec2((x & uvec2(1u))));
  uvec2 is_zero = mix(uvec2(0u), uvec2(4294967295u), equal(x, uvec2(0u)));
  return uvec2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = tint_first_trailing_bit(arg_0);
}

vec4 vertex_main() {
  firstTrailingBit_45eb10();
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

uvec2 tint_first_trailing_bit(uvec2 v) {
  uvec2 x = uvec2(v);
  uvec2 b16 = mix(uvec2(16u), uvec2(0u), bvec2((x & uvec2(65535u))));
  x = (x >> b16);
  uvec2 b8 = mix(uvec2(8u), uvec2(0u), bvec2((x & uvec2(255u))));
  x = (x >> b8);
  uvec2 b4 = mix(uvec2(4u), uvec2(0u), bvec2((x & uvec2(15u))));
  x = (x >> b4);
  uvec2 b2 = mix(uvec2(2u), uvec2(0u), bvec2((x & uvec2(3u))));
  x = (x >> b2);
  uvec2 b1 = mix(uvec2(1u), uvec2(0u), bvec2((x & uvec2(1u))));
  uvec2 is_zero = mix(uvec2(0u), uvec2(4294967295u), equal(x, uvec2(0u)));
  return uvec2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = tint_first_trailing_bit(arg_0);
}

void fragment_main() {
  firstTrailingBit_45eb10();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec2 tint_first_trailing_bit(uvec2 v) {
  uvec2 x = uvec2(v);
  uvec2 b16 = mix(uvec2(16u), uvec2(0u), bvec2((x & uvec2(65535u))));
  x = (x >> b16);
  uvec2 b8 = mix(uvec2(8u), uvec2(0u), bvec2((x & uvec2(255u))));
  x = (x >> b8);
  uvec2 b4 = mix(uvec2(4u), uvec2(0u), bvec2((x & uvec2(15u))));
  x = (x >> b4);
  uvec2 b2 = mix(uvec2(2u), uvec2(0u), bvec2((x & uvec2(3u))));
  x = (x >> b2);
  uvec2 b1 = mix(uvec2(1u), uvec2(0u), bvec2((x & uvec2(1u))));
  uvec2 is_zero = mix(uvec2(0u), uvec2(4294967295u), equal(x, uvec2(0u)));
  return uvec2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 res = tint_first_trailing_bit(arg_0);
}

void compute_main() {
  firstTrailingBit_45eb10();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
