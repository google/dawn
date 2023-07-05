#version 310 es

void main_1() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  float x_16 = m[1].y;
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
