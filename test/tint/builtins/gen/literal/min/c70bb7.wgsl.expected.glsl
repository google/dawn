#version 310 es

void min_c70bb7() {
  uvec3 res = uvec3(1u);
}

vec4 vertex_main() {
  min_c70bb7();
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

void min_c70bb7() {
  uvec3 res = uvec3(1u);
}

void fragment_main() {
  min_c70bb7();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void min_c70bb7() {
  uvec3 res = uvec3(1u);
}

void compute_main() {
  min_c70bb7();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
