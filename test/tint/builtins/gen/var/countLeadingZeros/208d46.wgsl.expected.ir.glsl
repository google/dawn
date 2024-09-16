#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = (((v_1 <= 65535u)) ? (16u) : (0u));
  uint v_3 = ((((v_1 << v_2) <= 16777215u)) ? (8u) : (0u));
  uint v_4 = (((((v_1 << v_2) << v_3) <= 268435455u)) ? (4u) : (0u));
  uint v_5 = ((((((v_1 << v_2) << v_3) << v_4) <= 1073741823u)) ? (2u) : (0u));
  uint v_6 = (((((((v_1 << v_2) << v_3) << v_4) << v_5) <= 2147483647u)) ? (1u) : (0u));
  uint v_7 = (((((((v_1 << v_2) << v_3) << v_4) << v_5) == 0u)) ? (1u) : (0u));
  uint res = ((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7);
  return res;
}
void main() {
  v.tint_symbol = countLeadingZeros_208d46();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = (((v_1 <= 65535u)) ? (16u) : (0u));
  uint v_3 = ((((v_1 << v_2) <= 16777215u)) ? (8u) : (0u));
  uint v_4 = (((((v_1 << v_2) << v_3) <= 268435455u)) ? (4u) : (0u));
  uint v_5 = ((((((v_1 << v_2) << v_3) << v_4) <= 1073741823u)) ? (2u) : (0u));
  uint v_6 = (((((((v_1 << v_2) << v_3) << v_4) << v_5) <= 2147483647u)) ? (1u) : (0u));
  uint v_7 = (((((((v_1 << v_2) << v_3) << v_4) << v_5) == 0u)) ? (1u) : (0u));
  uint res = ((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = countLeadingZeros_208d46();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = (((v <= 65535u)) ? (16u) : (0u));
  uint v_2 = ((((v << v_1) <= 16777215u)) ? (8u) : (0u));
  uint v_3 = (((((v << v_1) << v_2) <= 268435455u)) ? (4u) : (0u));
  uint v_4 = ((((((v << v_1) << v_2) << v_3) <= 1073741823u)) ? (2u) : (0u));
  uint v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= 2147483647u)) ? (1u) : (0u));
  uint v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == 0u)) ? (1u) : (0u));
  uint res = ((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_208d46();
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
