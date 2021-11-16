#version 310 es
precision mediump float;

void main_1() {
  mat3 m = mat3(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  vec3 x_15 = m[1];
  float x_16 = x_15.y;
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


