#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  f16vec3 tint_symbol;
} v;
f16vec3 select_53d518() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bvec3 arg_2 = bvec3(true);
  f16vec3 v_1 = arg_0;
  f16vec3 v_2 = arg_1;
  bvec3 v_3 = arg_2;
  float16_t v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  float16_t v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  f16vec3 res = f16vec3(v_4, v_5, ((v_3.z) ? (v_2.z) : (v_1.z)));
  return res;
}
void main() {
  v.tint_symbol = select_53d518();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  f16vec3 tint_symbol;
} v;
f16vec3 select_53d518() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bvec3 arg_2 = bvec3(true);
  f16vec3 v_1 = arg_0;
  f16vec3 v_2 = arg_1;
  bvec3 v_3 = arg_2;
  float16_t v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  float16_t v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  f16vec3 res = f16vec3(v_4, v_5, ((v_3.z) ? (v_2.z) : (v_1.z)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_53d518();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec3 prevent_dce;
};

layout(location = 0) flat out f16vec3 vertex_main_loc0_Output;
f16vec3 select_53d518() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bvec3 arg_2 = bvec3(true);
  f16vec3 v = arg_0;
  f16vec3 v_1 = arg_1;
  bvec3 v_2 = arg_2;
  float16_t v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  float16_t v_4 = ((v_2.y) ? (v_1.y) : (v.y));
  f16vec3 res = f16vec3(v_3, v_4, ((v_2.z) ? (v_1.z) : (v.z)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec3(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_53d518();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
