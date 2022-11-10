#version 310 es

float tint_acosh(float x) {
  return ((x < 1.0f) ? 0.0f : acosh(x));
}

void acosh_ecf2d1() {
  float arg_0 = 2.0f;
  float res = tint_acosh(arg_0);
}

vec4 vertex_main() {
  acosh_ecf2d1();
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

float tint_acosh(float x) {
  return ((x < 1.0f) ? 0.0f : acosh(x));
}

void acosh_ecf2d1() {
  float arg_0 = 2.0f;
  float res = tint_acosh(arg_0);
}

void fragment_main() {
  acosh_ecf2d1();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

float tint_acosh(float x) {
  return ((x < 1.0f) ? 0.0f : acosh(x));
}

void acosh_ecf2d1() {
  float arg_0 = 2.0f;
  float res = tint_acosh(arg_0);
}

void compute_main() {
  acosh_ecf2d1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
