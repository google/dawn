#version 310 es

layout(binding = 0, std140) uniform Constants_ubo {
  uint zero;
  uint pad;
  uint pad_1;
  uint pad_2;
} constants;

struct Result {
  uint value;
};

struct S {
  uint data[3];
};

S s = S(uint[3](0u, 0u, 0u));
void tint_symbol() {
  s.data[constants.zero] = 0u;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
