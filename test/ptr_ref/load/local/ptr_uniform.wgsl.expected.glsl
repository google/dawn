#version 310 es
precision mediump float;


layout (binding = 0) uniform S_1 {
  int a;
} v;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  int use = (v.a + 1);
  return;
}
void main() {
  tint_symbol();
}


