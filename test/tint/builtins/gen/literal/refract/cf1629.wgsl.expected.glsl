#version 310 es

void refract_cf1629() {
  vec2 res = vec2(-3.0f);
}

vec4 vertex_main() {
  refract_cf1629();
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

void refract_cf1629() {
  vec2 res = vec2(-3.0f);
}

void fragment_main() {
  refract_cf1629();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void refract_cf1629() {
  vec2 res = vec2(-3.0f);
}

void compute_main() {
  refract_cf1629();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
