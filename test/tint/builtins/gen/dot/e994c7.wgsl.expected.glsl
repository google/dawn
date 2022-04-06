#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

void dot_e994c7() {
  uint res = tint_int_dot(uvec4(0u, 0u, 0u, 0u), uvec4(0u, 0u, 0u, 0u));
}

vec4 vertex_main() {
  dot_e994c7();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

void dot_e994c7() {
  uint res = tint_int_dot(uvec4(0u, 0u, 0u, 0u), uvec4(0u, 0u, 0u, 0u));
}

void fragment_main() {
  dot_e994c7();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

void dot_e994c7() {
  uint res = tint_int_dot(uvec4(0u, 0u, 0u, 0u), uvec4(0u, 0u, 0u, 0u));
}

void compute_main() {
  dot_e994c7();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
