#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};

layout (binding = 0) uniform UBO_1 {
  tint_padded_array_element data[4];
  int dynamic_idx;
} ubo;

layout (binding = 2) buffer Result_1 {
  int tint_symbol;
} result;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  result.tint_symbol = ubo.data[ubo.dynamic_idx].el;
  return;
}
void main() {
  f();
}


