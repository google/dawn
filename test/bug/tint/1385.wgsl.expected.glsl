#version 310 es
precision mediump float;


layout (binding = 1) buffer data_block_1 {
  int inner[];
} data;

int foo() {
  return data.inner[0];
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void tint_symbol() {
  foo();
  return;
}
void main() {
  tint_symbol();
}


