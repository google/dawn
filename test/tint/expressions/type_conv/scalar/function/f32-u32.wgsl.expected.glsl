#version 310 es

float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}
uint tint_f32_to_u32(float value) {
  return uint(clamp(value, 0.0f, 4294967040.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v = tint_f32_to_u32(m());
}
