#version 310 es

void main_1() {
  vec2 distance_1 = vec2(2.0f);
  float x_10 = distance(distance_1, vec2(2.0f));
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
