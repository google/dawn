#version 310 es

struct S {
  mat2 m;
};

S SSBO;
uniform S UBO;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
