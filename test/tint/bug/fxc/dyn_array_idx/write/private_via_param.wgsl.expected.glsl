#version 310 es

struct UBO {
  int dynamic_idx;
};

layout(binding = 0) uniform UBO_1 {
  int dynamic_idx;
} ubo;

struct S {
  int data[64];
};

struct Result {
  int tint_symbol;
};

layout(binding = 1, std430) buffer Result_1 {
  int tint_symbol;
} result;
S s = S(int[64](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
void x(inout S p) {
  p.data[ubo.dynamic_idx] = 1;
}

void f() {
  x(s);
  result.tint_symbol = s.data[3];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
