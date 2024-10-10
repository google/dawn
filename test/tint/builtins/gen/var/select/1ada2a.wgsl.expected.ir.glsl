#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  f16vec3 inner;
} v;
f16vec3 select_1ada2a() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bool arg_2 = true;
  f16vec3 v_1 = arg_0;
  f16vec3 v_2 = arg_1;
  f16vec3 res = mix(v_1, v_2, bvec3(arg_2));
  return res;
}
void main() {
  v.inner = select_1ada2a();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  f16vec3 inner;
} v;
f16vec3 select_1ada2a() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bool arg_2 = true;
  f16vec3 v_1 = arg_0;
  f16vec3 v_2 = arg_1;
  f16vec3 res = mix(v_1, v_2, bvec3(arg_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = select_1ada2a();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec3 prevent_dce;
};

layout(location = 0) flat out f16vec3 vertex_main_loc0_Output;
f16vec3 select_1ada2a() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bool arg_2 = true;
  f16vec3 v = arg_0;
  f16vec3 v_1 = arg_1;
  f16vec3 res = mix(v, v_1, bvec3(arg_2));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec3(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_1ada2a();
  return tint_symbol;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = v_2.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
