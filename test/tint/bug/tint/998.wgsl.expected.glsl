#version 310 es

struct Constants {
  uint zero;
};

layout(binding = 0, std140) uniform constants_block_ubo {
  Constants inner;
} constants;

struct Result {
  uint value;
};

struct S {
  uint data[3];
};

S s = S(uint[3](0u, 0u, 0u));
void tint_symbol() {
  s.data[constants.inner.zero] = 0u;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
