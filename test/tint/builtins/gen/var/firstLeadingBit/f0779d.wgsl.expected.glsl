#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint res = mix((mix(16u, 0u, ((v_1 & 4294901760u) == 0u)) | (mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u)) | (mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u)) | (mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u)) | mix(1u, 0u, ((((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) & 2u) == 0u)))))), 4294967295u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) == 0u));
  return res;
}
void main() {
  v.inner = firstLeadingBit_f0779d();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint res = mix((mix(16u, 0u, ((v_1 & 4294901760u) == 0u)) | (mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u)) | (mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u)) | (mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u)) | mix(1u, 0u, ((((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) & 2u) == 0u)))))), 4294967295u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) == 0u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstLeadingBit_f0779d();
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
  uint res = mix((mix(16u, 0u, ((v & 4294901760u) == 0u)) | (mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u)) | (mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u)) | (mix(2u, 0u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u)) | mix(1u, 0u, ((((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) & 2u) == 0u)))))), 4294967295u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) == 0u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_f0779d();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
