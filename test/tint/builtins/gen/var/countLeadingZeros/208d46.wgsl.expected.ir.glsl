#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(0u, 16u, (v_1 <= 65535u));
  uint v_3 = mix(0u, 8u, ((v_1 << v_2) <= 16777215u));
  uint v_4 = mix(0u, 4u, (((v_1 << v_2) << v_3) <= 268435455u));
  uint v_5 = mix(0u, 2u, ((((v_1 << v_2) << v_3) << v_4) <= 1073741823u));
  uint v_6 = mix(0u, 1u, (((((v_1 << v_2) << v_3) << v_4) << v_5) <= 2147483647u));
  uint v_7 = mix(0u, 1u, (((((v_1 << v_2) << v_3) << v_4) << v_5) == 0u));
  uint res = ((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7);
  return res;
}
void main() {
  v.inner = countLeadingZeros_208d46();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(0u, 16u, (v_1 <= 65535u));
  uint v_3 = mix(0u, 8u, ((v_1 << v_2) <= 16777215u));
  uint v_4 = mix(0u, 4u, (((v_1 << v_2) << v_3) <= 268435455u));
  uint v_5 = mix(0u, 2u, ((((v_1 << v_2) << v_3) << v_4) <= 1073741823u));
  uint v_6 = mix(0u, 1u, (((((v_1 << v_2) << v_3) << v_4) << v_5) <= 2147483647u));
  uint v_7 = mix(0u, 1u, (((((v_1 << v_2) << v_3) << v_4) << v_5) == 0u));
  uint res = ((v_2 | (v_3 | (v_4 | (v_5 | (v_6 | v_7))))) + v_7);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countLeadingZeros_208d46();
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
  uint v_1 = mix(0u, 16u, (v <= 65535u));
  uint v_2 = mix(0u, 8u, ((v << v_1) <= 16777215u));
  uint v_3 = mix(0u, 4u, (((v << v_1) << v_2) <= 268435455u));
  uint v_4 = mix(0u, 2u, ((((v << v_1) << v_2) << v_3) <= 1073741823u));
  uint v_5 = mix(0u, 1u, (((((v << v_1) << v_2) << v_3) << v_4) <= 2147483647u));
  uint v_6 = mix(0u, 1u, (((((v << v_1) << v_2) << v_3) << v_4) == 0u));
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
