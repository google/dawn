#version 310 es

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


ivec2 tint_first_leading_bit(ivec2 v) {
  uvec2 x = tint_select(uvec2(v), uvec2(~(v)), lessThan(v, ivec2(0)));
  uvec2 b16 = tint_select(uvec2(0u), uvec2(16u), bvec2((x & uvec2(4294901760u))));
  x = (x >> b16);
  uvec2 b8 = tint_select(uvec2(0u), uvec2(8u), bvec2((x & uvec2(65280u))));
  x = (x >> b8);
  uvec2 b4 = tint_select(uvec2(0u), uvec2(4u), bvec2((x & uvec2(240u))));
  x = (x >> b4);
  uvec2 b2 = tint_select(uvec2(0u), uvec2(2u), bvec2((x & uvec2(12u))));
  x = (x >> b2);
  uvec2 b1 = tint_select(uvec2(0u), uvec2(1u), bvec2((x & uvec2(2u))));
  uvec2 is_zero = tint_select(uvec2(0u), uvec2(4294967295u), equal(x, uvec2(0u)));
  return ivec2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec2 inner;
} prevent_dce;

void firstLeadingBit_a622c2() {
  ivec2 arg_0 = ivec2(1);
  ivec2 res = tint_first_leading_bit(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  firstLeadingBit_a622c2();
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

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


ivec2 tint_first_leading_bit(ivec2 v) {
  uvec2 x = tint_select(uvec2(v), uvec2(~(v)), lessThan(v, ivec2(0)));
  uvec2 b16 = tint_select(uvec2(0u), uvec2(16u), bvec2((x & uvec2(4294901760u))));
  x = (x >> b16);
  uvec2 b8 = tint_select(uvec2(0u), uvec2(8u), bvec2((x & uvec2(65280u))));
  x = (x >> b8);
  uvec2 b4 = tint_select(uvec2(0u), uvec2(4u), bvec2((x & uvec2(240u))));
  x = (x >> b4);
  uvec2 b2 = tint_select(uvec2(0u), uvec2(2u), bvec2((x & uvec2(12u))));
  x = (x >> b2);
  uvec2 b1 = tint_select(uvec2(0u), uvec2(1u), bvec2((x & uvec2(2u))));
  uvec2 is_zero = tint_select(uvec2(0u), uvec2(4294967295u), equal(x, uvec2(0u)));
  return ivec2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec2 inner;
} prevent_dce;

void firstLeadingBit_a622c2() {
  ivec2 arg_0 = ivec2(1);
  ivec2 res = tint_first_leading_bit(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  firstLeadingBit_a622c2();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


ivec2 tint_first_leading_bit(ivec2 v) {
  uvec2 x = tint_select(uvec2(v), uvec2(~(v)), lessThan(v, ivec2(0)));
  uvec2 b16 = tint_select(uvec2(0u), uvec2(16u), bvec2((x & uvec2(4294901760u))));
  x = (x >> b16);
  uvec2 b8 = tint_select(uvec2(0u), uvec2(8u), bvec2((x & uvec2(65280u))));
  x = (x >> b8);
  uvec2 b4 = tint_select(uvec2(0u), uvec2(4u), bvec2((x & uvec2(240u))));
  x = (x >> b4);
  uvec2 b2 = tint_select(uvec2(0u), uvec2(2u), bvec2((x & uvec2(12u))));
  x = (x >> b2);
  uvec2 b1 = tint_select(uvec2(0u), uvec2(1u), bvec2((x & uvec2(2u))));
  uvec2 is_zero = tint_select(uvec2(0u), uvec2(4294967295u), equal(x, uvec2(0u)));
  return ivec2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec2 inner;
} prevent_dce;

void firstLeadingBit_a622c2() {
  ivec2 arg_0 = ivec2(1);
  ivec2 res = tint_first_leading_bit(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  firstLeadingBit_a622c2();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
