#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 1, std430) buffer SB_RW_ssbo {
  float16_t arg_0[];
} sb_rw;

uint arrayLength_cbd6b5() {
  uint res = uint(sb_rw.arg_0.length());
  return res;
}

void fragment_main() {
  prevent_dce.inner = arrayLength_cbd6b5();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 1, std430) buffer SB_RW_ssbo {
  float16_t arg_0[];
} sb_rw;

uint arrayLength_cbd6b5() {
  uint res = uint(sb_rw.arg_0.length());
  return res;
}

void compute_main() {
  prevent_dce.inner = arrayLength_cbd6b5();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
