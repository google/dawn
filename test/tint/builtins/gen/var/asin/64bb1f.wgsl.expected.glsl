#version 310 es

void asin_64bb1f() {
  vec4 res = vec4(0.5f);
}

vec4 vertex_main() {
  asin_64bb1f();
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

void asin_64bb1f() {
  vec4 res = vec4(0.5f);
}

void fragment_main() {
  asin_64bb1f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void asin_64bb1f() {
  vec4 res = vec4(0.5f);
}

void compute_main() {
  asin_64bb1f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
