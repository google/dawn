#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

f16vec3 tint_select(f16vec3 param_0, f16vec3 param_1, bvec3 param_2) {
    return f16vec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


f16vec3 select_53d518() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bvec3 arg_2 = bvec3(true);
  f16vec3 res = tint_select(arg_0, arg_1, arg_2);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec3 inner;
} prevent_dce;

struct VertexOutput {
  vec4 pos;
  f16vec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = select_53d518();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_select(f16vec3 param_0, f16vec3 param_1, bvec3 param_2) {
    return f16vec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


f16vec3 select_53d518() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bvec3 arg_2 = bvec3(true);
  f16vec3 res = tint_select(arg_0, arg_1, arg_2);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec3 inner;
} prevent_dce;

struct VertexOutput {
  vec4 pos;
  f16vec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = select_53d518();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_select(f16vec3 param_0, f16vec3 param_1, bvec3 param_2) {
    return f16vec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


layout(location = 0) flat out f16vec3 prevent_dce_1;
f16vec3 select_53d518() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 arg_1 = f16vec3(1.0hf);
  bvec3 arg_2 = bvec3(true);
  f16vec3 res = tint_select(arg_0, arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  f16vec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), f16vec3(0.0hf, 0.0hf, 0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_53d518();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
