#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}

void main() {
  unused_entry_point();
}

const mat2x3 m = mat2x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
