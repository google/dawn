#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
layout(binding = 1, std430)
buffer SB_RO_1_ssbo {
  float16_t arg_0[];
} sb_ro;
uint arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
void main() {
  v.tint_symbol = arrayLength_8421b9();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
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
  v.tint_symbol = arrayLength_8421b9();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(binding = 1, std430)
buffer SB_RO_1_ssbo {
  float16_t arg_0[];
} sb_ro;
layout(location = 0) flat out uint vertex_main_loc0_Output;
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
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v.prevent_dce;
  gl_PointSize = 1.0f;
}
