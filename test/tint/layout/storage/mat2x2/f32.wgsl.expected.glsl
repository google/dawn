#version 310 es

struct SSBO {
  mat2 m;
};

layout(binding = 0, std430) buffer SSBO_1 {
  mat2 m;
} ssbo;
void f() {
  mat2 v = ssbo.m;
  ssbo.m = v;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
