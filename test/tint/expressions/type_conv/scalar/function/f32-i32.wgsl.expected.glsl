#version 310 es

float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
void f() {
  int v = tint_f32_to_i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
