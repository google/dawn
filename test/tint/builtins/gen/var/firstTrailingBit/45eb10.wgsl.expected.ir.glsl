#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
uvec2 firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v_1 = arg_0;
  uvec2 v_2 = mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)));
  uvec2 v_3 = mix(uvec2(0u), uvec2(8u), equal(((v_1 >> v_2) & uvec2(255u)), uvec2(0u)));
  uvec2 v_4 = mix(uvec2(0u), uvec2(4u), equal((((v_1 >> v_2) >> v_3) & uvec2(15u)), uvec2(0u)));
  uvec2 v_5 = mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec2(3u)), uvec2(0u)));
  uvec2 v_6 = (v_2 | (v_3 | (v_4 | (v_5 | mix(uvec2(0u), uvec2(1u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec2(1u)), uvec2(0u)))))));
  uvec2 res = mix(v_6, uvec2(4294967295u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec2(0u)));
  return res;
}
void main() {
  v.tint_symbol = firstTrailingBit_45eb10();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
uvec2 firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v_1 = arg_0;
  uvec2 v_2 = mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)));
  uvec2 v_3 = mix(uvec2(0u), uvec2(8u), equal(((v_1 >> v_2) & uvec2(255u)), uvec2(0u)));
  uvec2 v_4 = mix(uvec2(0u), uvec2(4u), equal((((v_1 >> v_2) >> v_3) & uvec2(15u)), uvec2(0u)));
  uvec2 v_5 = mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec2(3u)), uvec2(0u)));
  uvec2 v_6 = (v_2 | (v_3 | (v_4 | (v_5 | mix(uvec2(0u), uvec2(1u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec2(1u)), uvec2(0u)))))));
  uvec2 res = mix(v_6, uvec2(4294967295u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec2(0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstTrailingBit_45eb10();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(location = 0) flat out uvec2 vertex_main_loc0_Output;
uvec2 firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v = arg_0;
  uvec2 v_1 = mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)));
  uvec2 v_2 = mix(uvec2(0u), uvec2(8u), equal(((v >> v_1) & uvec2(255u)), uvec2(0u)));
  uvec2 v_3 = mix(uvec2(0u), uvec2(4u), equal((((v >> v_1) >> v_2) & uvec2(15u)), uvec2(0u)));
  uvec2 v_4 = mix(uvec2(0u), uvec2(2u), equal(((((v >> v_1) >> v_2) >> v_3) & uvec2(3u)), uvec2(0u)));
  uvec2 v_5 = (v_1 | (v_2 | (v_3 | (v_4 | mix(uvec2(0u), uvec2(1u), equal((((((v >> v_1) >> v_2) >> v_3) >> v_4) & uvec2(1u)), uvec2(0u)))))));
  uvec2 res = mix(v_5, uvec2(4294967295u), equal(((((v >> v_1) >> v_2) >> v_3) >> v_4), uvec2(0u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstTrailingBit_45eb10();
  return tint_symbol;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = v_6.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
