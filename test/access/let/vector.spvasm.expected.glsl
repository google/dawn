#version 310 es
precision mediump float;

void main_1() {
  float x_11 = vec3(1.0f, 2.0f, 3.0f).y;
  vec2 x_13 = vec2(vec3(1.0f, 2.0f, 3.0f).x, vec3(1.0f, 2.0f, 3.0f).z);
  vec3 x_14 = vec3(vec3(1.0f, 2.0f, 3.0f).x, vec3(1.0f, 2.0f, 3.0f).z, vec3(1.0f, 2.0f, 3.0f).y);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


