#version 310 es

int tint_int_dot(ivec2 a, ivec2 b) {
  return a[0]*b[0] + a[1]*b[1];
}

void dot_fc5f7c() {
  int res = tint_int_dot(ivec2(1), ivec2(1));
}

vec4 vertex_main() {
  dot_fc5f7c();
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

int tint_int_dot(ivec2 a, ivec2 b) {
  return a[0]*b[0] + a[1]*b[1];
}

void dot_fc5f7c() {
  int res = tint_int_dot(ivec2(1), ivec2(1));
}

void fragment_main() {
  dot_fc5f7c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

int tint_int_dot(ivec2 a, ivec2 b) {
  return a[0]*b[0] + a[1]*b[1];
}

void dot_fc5f7c() {
  int res = tint_int_dot(ivec2(1), ivec2(1));
}

void compute_main() {
  dot_fc5f7c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
