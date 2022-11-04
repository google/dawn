#version 310 es

void cross_041cb0() {
  vec3 res = cross(vec3(1.0f), vec3(1.0f));
}

vec4 vertex_main() {
  cross_041cb0();
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

void cross_041cb0() {
  vec3 res = cross(vec3(1.0f), vec3(1.0f));
}

void fragment_main() {
  cross_041cb0();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void cross_041cb0() {
  vec3 res = cross(vec3(1.0f), vec3(1.0f));
}

void compute_main() {
  cross_041cb0();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
