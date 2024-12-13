//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  f16vec4 inner;
} v;
f16vec4 smoothstep_c43ebd() {
  f16vec4 arg_0 = f16vec4(2.0hf);
  f16vec4 arg_1 = f16vec4(4.0hf);
  f16vec4 arg_2 = f16vec4(3.0hf);
  f16vec4 v_1 = arg_0;
  f16vec4 v_2 = clamp(((arg_2 - v_1) / (arg_1 - v_1)), f16vec4(0.0hf), f16vec4(1.0hf));
  f16vec4 res = (v_2 * (v_2 * (f16vec4(3.0hf) - (f16vec4(2.0hf) * v_2))));
  return res;
}
void main() {
  v.inner = smoothstep_c43ebd();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  f16vec4 inner;
} v;
f16vec4 smoothstep_c43ebd() {
  f16vec4 arg_0 = f16vec4(2.0hf);
  f16vec4 arg_1 = f16vec4(4.0hf);
  f16vec4 arg_2 = f16vec4(3.0hf);
  f16vec4 v_1 = arg_0;
  f16vec4 v_2 = clamp(((arg_2 - v_1) / (arg_1 - v_1)), f16vec4(0.0hf), f16vec4(1.0hf));
  f16vec4 res = (v_2 * (v_2 * (f16vec4(3.0hf) - (f16vec4(2.0hf) * v_2))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = smoothstep_c43ebd();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

layout(location = 0) flat out f16vec4 tint_interstage_location0;
f16vec4 smoothstep_c43ebd() {
  f16vec4 arg_0 = f16vec4(2.0hf);
  f16vec4 arg_1 = f16vec4(4.0hf);
  f16vec4 arg_2 = f16vec4(3.0hf);
  f16vec4 v = arg_0;
  f16vec4 v_1 = clamp(((arg_2 - v) / (arg_1 - v)), f16vec4(0.0hf), f16vec4(1.0hf));
  f16vec4 res = (v_1 * (v_1 * (f16vec4(3.0hf) - (f16vec4(2.0hf) * v_1))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), f16vec4(0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = smoothstep_c43ebd();
  return tint_symbol;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = v_2.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
