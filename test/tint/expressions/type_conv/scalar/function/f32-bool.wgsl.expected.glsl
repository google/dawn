#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  float tint_symbol = m();
  bool v = bool(tint_symbol);
}

