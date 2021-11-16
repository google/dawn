#version 310 es
precision mediump float;


layout (binding = 0) uniform UBO_1 {
  int dynamic_idx;
} ubo;

layout (binding = 2) buffer Result_1 {
  int tint_symbol;
} result;

layout (binding = 1) buffer SSBO_1 {
  int data[4];
} ssbo;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  result.tint_symbol = ssbo.data[ubo.dynamic_idx];
  return;
}
void main() {
  f();
}


