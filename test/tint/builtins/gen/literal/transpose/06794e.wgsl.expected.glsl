//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  int inner;
} v;
int transpose_06794e() {
  f16mat3 res = f16mat3(f16vec3(1.0hf), f16vec3(1.0hf), f16vec3(1.0hf));
  return mix(0, 1, (res[0u].x == 0.0hf));
}
void main() {
  v.inner = transpose_06794e();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int transpose_06794e() {
  f16mat3 res = f16mat3(f16vec3(1.0hf), f16vec3(1.0hf), f16vec3(1.0hf));
  return mix(0, 1, (res[0u].x == 0.0hf));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = transpose_06794e();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int tint_interstage_location0;
int transpose_06794e() {
  f16mat3 res = f16mat3(f16vec3(1.0hf), f16vec3(1.0hf), f16vec3(1.0hf));
  return mix(0, 1, (res[0u].x == 0.0hf));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0);
  v.pos = vec4(0.0f);
  v.prevent_dce = transpose_06794e();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
