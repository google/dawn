//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer f_SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;
uint arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
void main() {
  v.inner = arrayLength_8421b9();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer SB_RO_1_ssbo {
  float16_t arg_0[];
} sb_ro;
uint arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = arrayLength_8421b9();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(binding = 1, std430)
buffer v_SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;
layout(location = 0) flat out uint tint_interstage_location0;
uint arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = arrayLength_8421b9();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v.prevent_dce;
  gl_PointSize = 1.0f;
}
