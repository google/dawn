#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint tint_bitcast_from_f16(f16vec2 src) {
  return uint(packFloat2x16(src));
}
uint bitcast_a58b50() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}
void main() {
  v.inner = bitcast_a58b50();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint tint_bitcast_from_f16(f16vec2 src) {
  return uint(packFloat2x16(src));
}
uint bitcast_a58b50() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = bitcast_a58b50();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint tint_bitcast_from_f16(f16vec2 src) {
  return uint(packFloat2x16(src));
}
uint bitcast_a58b50() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_a58b50();
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
