#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
ivec4 firstLeadingBit_c1f940() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v_1 = uvec4(arg_0);
  uvec4 v_2 = mix(~(v_1), v_1, lessThan(v_1, uvec4(2147483648u)));
  uvec4 v_3 = mix(uvec4(16u), uvec4(0u), equal((v_2 & uvec4(4294901760u)), uvec4(0u)));
  uvec4 v_4 = mix(uvec4(8u), uvec4(0u), equal(((v_2 >> v_3) & uvec4(65280u)), uvec4(0u)));
  uvec4 v_5 = mix(uvec4(4u), uvec4(0u), equal((((v_2 >> v_3) >> v_4) & uvec4(240u)), uvec4(0u)));
  uvec4 v_6 = mix(uvec4(2u), uvec4(0u), equal(((((v_2 >> v_3) >> v_4) >> v_5) & uvec4(12u)), uvec4(0u)));
  uvec4 v_7 = (v_3 | (v_4 | (v_5 | (v_6 | mix(uvec4(1u), uvec4(0u), equal((((((v_2 >> v_3) >> v_4) >> v_5) >> v_6) & uvec4(2u)), uvec4(0u)))))));
  ivec4 res = ivec4(mix(v_7, uvec4(4294967295u), equal(((((v_2 >> v_3) >> v_4) >> v_5) >> v_6), uvec4(0u))));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_c1f940();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
ivec4 firstLeadingBit_c1f940() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v_1 = uvec4(arg_0);
  uvec4 v_2 = mix(~(v_1), v_1, lessThan(v_1, uvec4(2147483648u)));
  uvec4 v_3 = mix(uvec4(16u), uvec4(0u), equal((v_2 & uvec4(4294901760u)), uvec4(0u)));
  uvec4 v_4 = mix(uvec4(8u), uvec4(0u), equal(((v_2 >> v_3) & uvec4(65280u)), uvec4(0u)));
  uvec4 v_5 = mix(uvec4(4u), uvec4(0u), equal((((v_2 >> v_3) >> v_4) & uvec4(240u)), uvec4(0u)));
  uvec4 v_6 = mix(uvec4(2u), uvec4(0u), equal(((((v_2 >> v_3) >> v_4) >> v_5) & uvec4(12u)), uvec4(0u)));
  uvec4 v_7 = (v_3 | (v_4 | (v_5 | (v_6 | mix(uvec4(1u), uvec4(0u), equal((((((v_2 >> v_3) >> v_4) >> v_5) >> v_6) & uvec4(2u)), uvec4(0u)))))));
  ivec4 res = ivec4(mix(v_7, uvec4(4294967295u), equal(((((v_2 >> v_3) >> v_4) >> v_5) >> v_6), uvec4(0u))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_c1f940();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(location = 0) flat out ivec4 vertex_main_loc0_Output;
ivec4 firstLeadingBit_c1f940() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v = uvec4(arg_0);
  uvec4 v_1 = mix(~(v), v, lessThan(v, uvec4(2147483648u)));
  uvec4 v_2 = mix(uvec4(16u), uvec4(0u), equal((v_1 & uvec4(4294901760u)), uvec4(0u)));
  uvec4 v_3 = mix(uvec4(8u), uvec4(0u), equal(((v_1 >> v_2) & uvec4(65280u)), uvec4(0u)));
  uvec4 v_4 = mix(uvec4(4u), uvec4(0u), equal((((v_1 >> v_2) >> v_3) & uvec4(240u)), uvec4(0u)));
  uvec4 v_5 = mix(uvec4(2u), uvec4(0u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec4(12u)), uvec4(0u)));
  uvec4 v_6 = (v_2 | (v_3 | (v_4 | (v_5 | mix(uvec4(1u), uvec4(0u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec4(2u)), uvec4(0u)))))));
  ivec4 res = ivec4(mix(v_6, uvec4(4294967295u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec4(0u))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec4(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_c1f940();
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
