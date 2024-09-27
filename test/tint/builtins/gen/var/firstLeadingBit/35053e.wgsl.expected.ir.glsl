#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uvec3 v_2 = mix(~(v_1), v_1, lessThan(v_1, uvec3(2147483648u)));
  uvec3 v_3 = mix(uvec3(16u), uvec3(0u), equal((v_2 & uvec3(4294901760u)), uvec3(0u)));
  uvec3 v_4 = mix(uvec3(8u), uvec3(0u), equal(((v_2 >> v_3) & uvec3(65280u)), uvec3(0u)));
  uvec3 v_5 = mix(uvec3(4u), uvec3(0u), equal((((v_2 >> v_3) >> v_4) & uvec3(240u)), uvec3(0u)));
  uvec3 v_6 = mix(uvec3(2u), uvec3(0u), equal(((((v_2 >> v_3) >> v_4) >> v_5) & uvec3(12u)), uvec3(0u)));
  uvec3 v_7 = (v_3 | (v_4 | (v_5 | (v_6 | mix(uvec3(1u), uvec3(0u), equal((((((v_2 >> v_3) >> v_4) >> v_5) >> v_6) & uvec3(2u)), uvec3(0u)))))));
  ivec3 res = ivec3(mix(v_7, uvec3(4294967295u), equal(((((v_2 >> v_3) >> v_4) >> v_5) >> v_6), uvec3(0u))));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_35053e();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uvec3 v_2 = mix(~(v_1), v_1, lessThan(v_1, uvec3(2147483648u)));
  uvec3 v_3 = mix(uvec3(16u), uvec3(0u), equal((v_2 & uvec3(4294901760u)), uvec3(0u)));
  uvec3 v_4 = mix(uvec3(8u), uvec3(0u), equal(((v_2 >> v_3) & uvec3(65280u)), uvec3(0u)));
  uvec3 v_5 = mix(uvec3(4u), uvec3(0u), equal((((v_2 >> v_3) >> v_4) & uvec3(240u)), uvec3(0u)));
  uvec3 v_6 = mix(uvec3(2u), uvec3(0u), equal(((((v_2 >> v_3) >> v_4) >> v_5) & uvec3(12u)), uvec3(0u)));
  uvec3 v_7 = (v_3 | (v_4 | (v_5 | (v_6 | mix(uvec3(1u), uvec3(0u), equal((((((v_2 >> v_3) >> v_4) >> v_5) >> v_6) & uvec3(2u)), uvec3(0u)))))));
  ivec3 res = ivec3(mix(v_7, uvec3(4294967295u), equal(((((v_2 >> v_3) >> v_4) >> v_5) >> v_6), uvec3(0u))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_35053e();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v = uvec3(arg_0);
  uvec3 v_1 = mix(~(v), v, lessThan(v, uvec3(2147483648u)));
  uvec3 v_2 = mix(uvec3(16u), uvec3(0u), equal((v_1 & uvec3(4294901760u)), uvec3(0u)));
  uvec3 v_3 = mix(uvec3(8u), uvec3(0u), equal(((v_1 >> v_2) & uvec3(65280u)), uvec3(0u)));
  uvec3 v_4 = mix(uvec3(4u), uvec3(0u), equal((((v_1 >> v_2) >> v_3) & uvec3(240u)), uvec3(0u)));
  uvec3 v_5 = mix(uvec3(2u), uvec3(0u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec3(12u)), uvec3(0u)));
  uvec3 v_6 = (v_2 | (v_3 | (v_4 | (v_5 | mix(uvec3(1u), uvec3(0u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec3(2u)), uvec3(0u)))))));
  ivec3 res = ivec3(mix(v_6, uvec3(4294967295u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec3(0u))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_35053e();
  return tint_symbol;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = v_7.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
