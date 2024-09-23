#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 tint_bitcast_from_f16(f16vec4 src) {
  uint v_1 = packFloat2x16(src.xy);
  return ivec2(uvec2(v_1, packFloat2x16(src.zw)));
}
ivec2 bitcast_214f23() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  ivec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}
void main() {
  v.tint_symbol = bitcast_214f23();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 tint_bitcast_from_f16(f16vec4 src) {
  uint v_1 = packFloat2x16(src.xy);
  return ivec2(uvec2(v_1, packFloat2x16(src.zw)));
}
ivec2 bitcast_214f23() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  ivec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = bitcast_214f23();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  ivec2 prevent_dce;
};

layout(location = 0) flat out ivec2 vertex_main_loc0_Output;
ivec2 tint_bitcast_from_f16(f16vec4 src) {
  uint v = packFloat2x16(src.xy);
  return ivec2(uvec2(v, packFloat2x16(src.zw)));
}
ivec2 bitcast_214f23() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  ivec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec2(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_214f23();
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
