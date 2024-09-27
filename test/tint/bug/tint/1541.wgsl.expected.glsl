#version 310 es

void tint_symbol() {
  bool a = true;
  bool v = mix(bool(uint(a) & uint(true)), true, false);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
