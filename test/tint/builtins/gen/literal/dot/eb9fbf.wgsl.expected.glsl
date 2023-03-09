#version 310 es

void dot_eb9fbf() {
  int res = 4;
}

vec4 vertex_main() {
  dot_eb9fbf();
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

void dot_eb9fbf() {
  int res = 4;
}

void fragment_main() {
  dot_eb9fbf();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void dot_eb9fbf() {
  int res = 4;
}

void compute_main() {
  dot_eb9fbf();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
