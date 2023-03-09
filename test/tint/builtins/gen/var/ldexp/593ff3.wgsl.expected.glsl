#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void ldexp_593ff3() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = ldexp(arg_0, ivec3(1));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  ldexp_593ff3();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void ldexp_593ff3() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = ldexp(arg_0, ivec3(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  ldexp_593ff3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void ldexp_593ff3() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = ldexp(arg_0, ivec3(1));
  prevent_dce.inner = res;
}

void compute_main() {
  ldexp_593ff3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
