#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int countTrailingZeros_42fed6() {
  int arg_0 = 1;
  uint v_1 = uint(arg_0);
  int res = int(((mix(0u, 16u, ((v_1 & 65535u) == 0u)) | (mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u)) | (mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u)) | (mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u)) | mix(0u, 1u, ((((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) & 1u) == 0u)))))) + mix(0u, 1u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) == 0u))));
  return res;
}
void main() {
  v.inner = countTrailingZeros_42fed6();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int countTrailingZeros_42fed6() {
  int arg_0 = 1;
  uint v_1 = uint(arg_0);
  int res = int(((mix(0u, 16u, ((v_1 & 65535u) == 0u)) | (mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u)) | (mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u)) | (mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u)) | mix(0u, 1u, ((((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) & 1u) == 0u)))))) + mix(0u, 1u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) >> mix(0u, 8u, (((v_1 >> mix(0u, 16u, ((v_1 & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) == 0u))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countTrailingZeros_42fed6();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int vertex_main_loc0_Output;
int countTrailingZeros_42fed6() {
  int arg_0 = 1;
  uint v = uint(arg_0);
  int res = int(((mix(0u, 16u, ((v & 65535u) == 0u)) | (mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u)) | (mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u)) | (mix(0u, 2u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u)) | mix(0u, 1u, ((((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) & 1u) == 0u)))))) + mix(0u, 1u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) >> mix(0u, 2u, (((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) >> mix(0u, 4u, ((((v >> mix(0u, 16u, ((v & 65535u) == 0u))) >> mix(0u, 8u, (((v >> mix(0u, 16u, ((v & 65535u) == 0u))) & 255u) == 0u))) & 15u) == 0u))) & 3u) == 0u))) == 0u))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countTrailingZeros_42fed6();
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
