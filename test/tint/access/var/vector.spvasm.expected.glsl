#version 310 es

void main_1() {
  vec3 v = vec3(0.0f);
  float x_14 = v.y;
  vec3 x_16 = v;
  vec2 x_17 = x_16.xz;
  vec3 x_18 = v;
  vec3 x_19 = x_18.xzy;
  return;
}

void tint_symbol() {
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
