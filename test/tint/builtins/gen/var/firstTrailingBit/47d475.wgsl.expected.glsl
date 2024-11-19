#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint firstTrailingBit_47d475() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint res = mix((mix(0u, 16u, ((v_1 & 65535u) == 0u)) | (mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u)) | (mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u)) | (mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u)) | mix(0u, 1u, ((((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) & 1u) == 0u)))))), 4294967295u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) == 0u));
  return res;
}
void main() {
  v.inner = firstTrailingBit_47d475();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint firstTrailingBit_47d475() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint res = mix((mix(0u, 16u, ((v_1 & 65535u) == 0u)) | (mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u)) | (mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u)) | (mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u)) | mix(0u, 1u, ((((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) & 1u) == 0u)))))), 4294967295u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) == 0u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstTrailingBit_47d475();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint firstTrailingBit_47d475() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint res = mix((mix(0u, 16u, ((v & 65535u) == 0u)) | (mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u)) | (mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u)) | (mix(0u, 2u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u)) | mix(0u, 1u, ((((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) & 1u) == 0u)))))), 4294967295u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) == 0u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstTrailingBit_47d475();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
