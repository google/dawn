#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint tint_ftou(float v) {
  return mix(4294967295u, mix(uint(v), 0u, (v < 0.0f)), (v <= 4294967040.0f));
}

float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  float tint_symbol = m();
  uint v = tint_ftou(tint_symbol);
}

