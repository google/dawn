#version 310 es
precision highp float;
precision highp int;

uvec3 tint_select(uvec3 param_0, uvec3 param_1, bvec3 param_2) {
    return uvec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


ivec3 tint_first_leading_bit(ivec3 v) {
  uvec3 x = tint_select(uvec3(v), uvec3(~(v)), lessThan(v, ivec3(0)));
  uvec3 b16 = tint_select(uvec3(0u), uvec3(16u), bvec3((x & uvec3(4294901760u))));
  x = (x >> b16);
  uvec3 b8 = tint_select(uvec3(0u), uvec3(8u), bvec3((x & uvec3(65280u))));
  x = (x >> b8);
  uvec3 b4 = tint_select(uvec3(0u), uvec3(4u), bvec3((x & uvec3(240u))));
  x = (x >> b4);
  uvec3 b2 = tint_select(uvec3(0u), uvec3(2u), bvec3((x & uvec3(12u))));
  x = (x >> b2);
  uvec3 b1 = tint_select(uvec3(0u), uvec3(1u), bvec3((x & uvec3(2u))));
  uvec3 is_zero = tint_select(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_first_leading_bit(arg_0);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec3 inner;
  uint pad;
} prevent_dce;

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = firstLeadingBit_35053e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec3 tint_select(uvec3 param_0, uvec3 param_1, bvec3 param_2) {
    return uvec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


ivec3 tint_first_leading_bit(ivec3 v) {
  uvec3 x = tint_select(uvec3(v), uvec3(~(v)), lessThan(v, ivec3(0)));
  uvec3 b16 = tint_select(uvec3(0u), uvec3(16u), bvec3((x & uvec3(4294901760u))));
  x = (x >> b16);
  uvec3 b8 = tint_select(uvec3(0u), uvec3(8u), bvec3((x & uvec3(65280u))));
  x = (x >> b8);
  uvec3 b4 = tint_select(uvec3(0u), uvec3(4u), bvec3((x & uvec3(240u))));
  x = (x >> b4);
  uvec3 b2 = tint_select(uvec3(0u), uvec3(2u), bvec3((x & uvec3(12u))));
  x = (x >> b2);
  uvec3 b1 = tint_select(uvec3(0u), uvec3(1u), bvec3((x & uvec3(2u))));
  uvec3 is_zero = tint_select(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_first_leading_bit(arg_0);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec3 inner;
  uint pad;
} prevent_dce;

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = firstLeadingBit_35053e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

uvec3 tint_select(uvec3 param_0, uvec3 param_1, bvec3 param_2) {
    return uvec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


ivec3 tint_first_leading_bit(ivec3 v) {
  uvec3 x = tint_select(uvec3(v), uvec3(~(v)), lessThan(v, ivec3(0)));
  uvec3 b16 = tint_select(uvec3(0u), uvec3(16u), bvec3((x & uvec3(4294901760u))));
  x = (x >> b16);
  uvec3 b8 = tint_select(uvec3(0u), uvec3(8u), bvec3((x & uvec3(65280u))));
  x = (x >> b8);
  uvec3 b4 = tint_select(uvec3(0u), uvec3(4u), bvec3((x & uvec3(240u))));
  x = (x >> b4);
  uvec3 b2 = tint_select(uvec3(0u), uvec3(2u), bvec3((x & uvec3(12u))));
  x = (x >> b2);
  uvec3 b1 = tint_select(uvec3(0u), uvec3(1u), bvec3((x & uvec3(2u))));
  uvec3 is_zero = tint_select(uvec3(0u), uvec3(4294967295u), equal(x, uvec3(0u)));
  return ivec3((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

layout(location = 0) flat out ivec3 prevent_dce_1;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = tint_first_leading_bit(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), ivec3(0, 0, 0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_35053e();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
