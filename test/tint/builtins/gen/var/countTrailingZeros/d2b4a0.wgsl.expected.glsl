#version 310 es

uvec4 tint_select(uvec4 param_0, uvec4 param_1, bvec4 param_2) {
    return uvec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


uvec4 tint_count_trailing_zeros(uvec4 v) {
  uvec4 x = uvec4(v);
  uvec4 b16 = tint_select(uvec4(16u), uvec4(0u), bvec4((x & uvec4(65535u))));
  x = (x >> b16);
  uvec4 b8 = tint_select(uvec4(8u), uvec4(0u), bvec4((x & uvec4(255u))));
  x = (x >> b8);
  uvec4 b4 = tint_select(uvec4(4u), uvec4(0u), bvec4((x & uvec4(15u))));
  x = (x >> b4);
  uvec4 b2 = tint_select(uvec4(2u), uvec4(0u), bvec4((x & uvec4(3u))));
  x = (x >> b2);
  uvec4 b1 = tint_select(uvec4(1u), uvec4(0u), bvec4((x & uvec4(1u))));
  uvec4 is_zero = tint_select(uvec4(0u), uvec4(1u), equal(x, uvec4(0u)));
  return uvec4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

void countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 res = tint_count_trailing_zeros(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  countTrailingZeros_d2b4a0();
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
precision highp float;

uvec4 tint_select(uvec4 param_0, uvec4 param_1, bvec4 param_2) {
    return uvec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


uvec4 tint_count_trailing_zeros(uvec4 v) {
  uvec4 x = uvec4(v);
  uvec4 b16 = tint_select(uvec4(16u), uvec4(0u), bvec4((x & uvec4(65535u))));
  x = (x >> b16);
  uvec4 b8 = tint_select(uvec4(8u), uvec4(0u), bvec4((x & uvec4(255u))));
  x = (x >> b8);
  uvec4 b4 = tint_select(uvec4(4u), uvec4(0u), bvec4((x & uvec4(15u))));
  x = (x >> b4);
  uvec4 b2 = tint_select(uvec4(2u), uvec4(0u), bvec4((x & uvec4(3u))));
  x = (x >> b2);
  uvec4 b1 = tint_select(uvec4(1u), uvec4(0u), bvec4((x & uvec4(1u))));
  uvec4 is_zero = tint_select(uvec4(0u), uvec4(1u), equal(x, uvec4(0u)));
  return uvec4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

void countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 res = tint_count_trailing_zeros(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  countTrailingZeros_d2b4a0();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec4 tint_select(uvec4 param_0, uvec4 param_1, bvec4 param_2) {
    return uvec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


uvec4 tint_count_trailing_zeros(uvec4 v) {
  uvec4 x = uvec4(v);
  uvec4 b16 = tint_select(uvec4(16u), uvec4(0u), bvec4((x & uvec4(65535u))));
  x = (x >> b16);
  uvec4 b8 = tint_select(uvec4(8u), uvec4(0u), bvec4((x & uvec4(255u))));
  x = (x >> b8);
  uvec4 b4 = tint_select(uvec4(4u), uvec4(0u), bvec4((x & uvec4(15u))));
  x = (x >> b4);
  uvec4 b2 = tint_select(uvec4(2u), uvec4(0u), bvec4((x & uvec4(3u))));
  x = (x >> b2);
  uvec4 b1 = tint_select(uvec4(1u), uvec4(0u), bvec4((x & uvec4(1u))));
  uvec4 is_zero = tint_select(uvec4(0u), uvec4(1u), equal(x, uvec4(0u)));
  return uvec4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

void countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 res = tint_count_trailing_zeros(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  countTrailingZeros_d2b4a0();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
