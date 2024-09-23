#version 310 es

float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}
uint tint_f32_to_u32(float value) {
  return (((value <= 4294967040.0f)) ? ((((value >= 0.0f)) ? (uint(value)) : (0u))) : (4294967295u));
}
void f() {
  uint v = tint_f32_to_u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
