#version 310 es

void atanh_c0e634() {
  vec2 res = vec2(0.54930615425109863281f);
}

vec4 vertex_main() {
  atanh_c0e634();
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

void atanh_c0e634() {
  vec2 res = vec2(0.54930615425109863281f);
}

void fragment_main() {
  atanh_c0e634();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void atanh_c0e634() {
  vec2 res = vec2(0.54930615425109863281f);
}

void compute_main() {
  atanh_c0e634();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
