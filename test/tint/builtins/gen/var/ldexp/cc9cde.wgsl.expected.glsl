#version 310 es

void ldexp_cc9cde() {
  vec4 arg_0 = vec4(1.0f);
  ivec4 arg_1 = ivec4(1);
  vec4 res = ldexp(arg_0, arg_1);
}

vec4 vertex_main() {
  ldexp_cc9cde();
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
precision mediump float;

void ldexp_cc9cde() {
  vec4 arg_0 = vec4(1.0f);
  ivec4 arg_1 = ivec4(1);
  vec4 res = ldexp(arg_0, arg_1);
}

void fragment_main() {
  ldexp_cc9cde();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void ldexp_cc9cde() {
  vec4 arg_0 = vec4(1.0f);
  ivec4 arg_1 = ivec4(1);
  vec4 res = ldexp(arg_0, arg_1);
}

void compute_main() {
  ldexp_cc9cde();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
