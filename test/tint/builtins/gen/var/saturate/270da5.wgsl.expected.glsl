#version 310 es

float tint_saturate(float v) {
  return clamp(v, 0.0f, 1.0f);
}

void saturate_270da5() {
  float arg_0 = 2.0f;
  float res = tint_saturate(arg_0);
}

vec4 vertex_main() {
  saturate_270da5();
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

float tint_saturate(float v) {
  return clamp(v, 0.0f, 1.0f);
}

void saturate_270da5() {
  float arg_0 = 2.0f;
  float res = tint_saturate(arg_0);
}

void fragment_main() {
  saturate_270da5();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

float tint_saturate(float v) {
  return clamp(v, 0.0f, 1.0f);
}

void saturate_270da5() {
  float arg_0 = 2.0f;
  float res = tint_saturate(arg_0);
}

void compute_main() {
  saturate_270da5();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
