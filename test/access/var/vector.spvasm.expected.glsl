#version 310 es
precision mediump float;

void main_1() {
  vec3 v = vec3(0.0f, 0.0f, 0.0f);
  float x_14 = v.y;
  vec3 x_16 = v;
  vec2 x_17 = vec2(x_16.x, x_16.z);
  vec3 x_18 = v;
  vec3 x_19 = vec3(x_18.x, x_18.z, x_18.y);
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


