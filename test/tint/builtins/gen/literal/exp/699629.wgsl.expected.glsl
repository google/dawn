#version 310 es

void exp_699629() {
  vec2 res = vec2(2.71828174591064453125f);
}

vec4 vertex_main() {
  exp_699629();
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

void exp_699629() {
  vec2 res = vec2(2.71828174591064453125f);
}

void fragment_main() {
  exp_699629();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void exp_699629() {
  vec2 res = vec2(2.71828174591064453125f);
}

void compute_main() {
  exp_699629();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
