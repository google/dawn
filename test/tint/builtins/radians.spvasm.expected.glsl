#version 310 es

float tint_radians(float param_0) {
  return param_0 * 0.01745329251994329547f;
}


void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  b = tint_radians(a);
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
