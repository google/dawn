#version 310 es
precision mediump float;


layout (binding = 0) uniform UBO_1 {
  int dynamic_idx;
} ubo;

struct S {
  int data[64];
};

layout (binding = 1) buffer Result_1 {
  int tint_symbol;
} result;

void x(inout S p) {
  p.data[ubo.dynamic_idx] = 1;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  S s = S(int[64](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  x(s);
  result.tint_symbol = s.data[3];
  return;
}
void main() {
  f();
}


