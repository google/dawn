#version 310 es

layout(binding = 0) uniform S_ubo {
  int i;
} u;

void tint_symbol() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
