#version 310 es

vec4 tint_radians(vec4 param_0) {
  return param_0 * 0.017453292519943295474f;
}


void radians_09b7fc() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = tint_radians(arg_0);
}

vec4 vertex_main() {
  radians_09b7fc();
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

vec4 tint_radians(vec4 param_0) {
  return param_0 * 0.017453292519943295474f;
}


void radians_09b7fc() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = tint_radians(arg_0);
}

void fragment_main() {
  radians_09b7fc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec4 tint_radians(vec4 param_0) {
  return param_0 * 0.017453292519943295474f;
}


void radians_09b7fc() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = tint_radians(arg_0);
}

void compute_main() {
  radians_09b7fc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
