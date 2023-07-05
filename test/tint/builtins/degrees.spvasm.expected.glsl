#version 310 es

float tint_degrees(float param_0) {
  return param_0 * 57.29577951308232286465f;
}


void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  b = tint_degrees(a);
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
