#version 310 es

void select_e381c3() {
  bool arg_2 = true;
  ivec4 res = (arg_2 ? ivec4(1) : ivec4(1));
}

vec4 vertex_main() {
  select_e381c3();
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

void select_e381c3() {
  bool arg_2 = true;
  ivec4 res = (arg_2 ? ivec4(1) : ivec4(1));
}

void fragment_main() {
  select_e381c3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void select_e381c3() {
  bool arg_2 = true;
  ivec4 res = (arg_2 ? ivec4(1) : ivec4(1));
}

void compute_main() {
  select_e381c3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
