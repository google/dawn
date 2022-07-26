#version 310 es

uvec3 tint_first_leading_bit(uvec3 v) {
  uvec3 x = v;
  uvec3 b16 = mix(uvec3(0u), uvec3(16u), bvec3((x & uvec3(4294901760u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(0u), uvec3(8u), bvec3((x & uvec3(65280u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(0u), uvec3(4u), bvec3((x & uvec3(240u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(0u), uvec3(2u), bvec3((x & uvec3(12u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(0u), uvec3(1u), bvec3((x & uvec3(2u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return uvec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstLeadingBit_3fd7d0() {
  uvec3 res = tint_first_leading_bit(uvec3(1u));
}

vec4 vertex_main() {
  firstLeadingBit_3fd7d0();
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

uvec3 tint_first_leading_bit(uvec3 v) {
  uvec3 x = v;
  uvec3 b16 = mix(uvec3(0u), uvec3(16u), bvec3((x & uvec3(4294901760u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(0u), uvec3(8u), bvec3((x & uvec3(65280u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(0u), uvec3(4u), bvec3((x & uvec3(240u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(0u), uvec3(2u), bvec3((x & uvec3(12u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(0u), uvec3(1u), bvec3((x & uvec3(2u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return uvec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstLeadingBit_3fd7d0() {
  uvec3 res = tint_first_leading_bit(uvec3(1u));
}

void fragment_main() {
  firstLeadingBit_3fd7d0();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec3 tint_first_leading_bit(uvec3 v) {
  uvec3 x = v;
  uvec3 b16 = mix(uvec3(0u), uvec3(16u), bvec3((x & uvec3(4294901760u))));
  x = (x >> b16);
  uvec3 b8 = mix(uvec3(0u), uvec3(8u), bvec3((x & uvec3(65280u))));
  x = (x >> b8);
  uvec3 b4 = mix(uvec3(0u), uvec3(4u), bvec3((x & uvec3(240u))));
  x = (x >> b4);
  uvec3 b2 = mix(uvec3(0u), uvec3(2u), bvec3((x & uvec3(12u))));
  x = (x >> b2);
  uvec3 b1 = mix(uvec3(0u), uvec3(1u), bvec3((x & uvec3(2u))));
  uvec3 is_zero = mix(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return uvec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstLeadingBit_3fd7d0() {
  uvec3 res = tint_first_leading_bit(uvec3(1u));
}

void compute_main() {
  firstLeadingBit_3fd7d0();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
