#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(16u, 0u, ((v_1 & 4294901760u) == 0u));
  uint v_3 = mix(8u, 0u, (((v_1 >> v_2) & 65280u) == 0u));
  uint v_4 = mix(4u, 0u, ((((v_1 >> v_2) >> v_3) & 240u) == 0u));
  uint v_5 = mix(2u, 0u, (((((v_1 >> v_2) >> v_3) >> v_4) & 12u) == 0u));
  uint res = mix((v_2 | (v_3 | (v_4 | (v_5 | mix(1u, 0u, ((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & 2u) == 0u)))))), 4294967295u, (((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == 0u));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_f0779d();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(16u, 0u, ((v_1 & 4294901760u) == 0u));
  uint v_3 = mix(8u, 0u, (((v_1 >> v_2) & 65280u) == 0u));
  uint v_4 = mix(4u, 0u, ((((v_1 >> v_2) >> v_3) & 240u) == 0u));
  uint v_5 = mix(2u, 0u, (((((v_1 >> v_2) >> v_3) >> v_4) & 12u) == 0u));
  uint res = mix((v_2 | (v_3 | (v_4 | (v_5 | mix(1u, 0u, ((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & 2u) == 0u)))))), 4294967295u, (((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == 0u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_f0779d();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = mix(16u, 0u, ((v & 4294901760u) == 0u));
  uint v_2 = mix(8u, 0u, (((v >> v_1) & 65280u) == 0u));
  uint v_3 = mix(4u, 0u, ((((v >> v_1) >> v_2) & 240u) == 0u));
  uint v_4 = mix(2u, 0u, (((((v >> v_1) >> v_2) >> v_3) & 12u) == 0u));
  uint res = mix((v_1 | (v_2 | (v_3 | (v_4 | mix(1u, 0u, ((((((v >> v_1) >> v_2) >> v_3) >> v_4) & 2u) == 0u)))))), 4294967295u, (((((v >> v_1) >> v_2) >> v_3) >> v_4) == 0u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_f0779d();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
