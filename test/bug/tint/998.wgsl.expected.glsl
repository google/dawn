#version 310 es
precision mediump float;


layout (binding = 0) uniform Constants_1 {
  uint zero;
} constants;

struct S {
  uint data[3];
};

S s = S(uint[3](0u, 0u, 0u));

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  s.data[constants.zero] = 0u;
  return;
}
void main() {
  tint_symbol();
}


