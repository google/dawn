#version 310 es

void firstLeadingBit_f0779d() {
  uint res = 0u;
}

vec4 vertex_main() {
  firstLeadingBit_f0779d();
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

void firstLeadingBit_f0779d() {
  uint res = 0u;
}

void fragment_main() {
  firstLeadingBit_f0779d();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void firstLeadingBit_f0779d() {
  uint res = 0u;
}

void compute_main() {
  firstLeadingBit_f0779d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
