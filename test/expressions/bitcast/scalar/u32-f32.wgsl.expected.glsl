#version 310 es

void f() {
  float b = uintBitsToFloat(1u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
