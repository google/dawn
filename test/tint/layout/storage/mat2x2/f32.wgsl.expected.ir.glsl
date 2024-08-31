#version 310 es

struct SSBO {
  mat2 m;
};

SSBO ssbo;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 v = ssbo.m;
  ssbo.m = v;
}
