#version 310 es

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void dot_7548a0() {
  uint res = tint_int_dot(uvec3(1u), uvec3(1u));
}

vec4 vertex_main() {
  dot_7548a0();
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

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void dot_7548a0() {
  uint res = tint_int_dot(uvec3(1u), uvec3(1u));
}

void fragment_main() {
  dot_7548a0();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void dot_7548a0() {
  uint res = tint_int_dot(uvec3(1u), uvec3(1u));
}

void compute_main() {
  dot_7548a0();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
