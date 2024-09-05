#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  f16vec4 tint_symbol;
} v;
f16vec4 sqrt_803d1c() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  f16vec4 res = sqrt(arg_0);
  return res;
}
void main() {
  v.tint_symbol = sqrt_803d1c();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  f16vec4 tint_symbol;
} v;
f16vec4 sqrt_803d1c() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  f16vec4 res = sqrt(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = sqrt_803d1c();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

layout(location = 0) flat out f16vec4 vertex_main_loc0_Output;
f16vec4 sqrt_803d1c() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  f16vec4 res = sqrt(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec4(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = sqrt_803d1c();
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
