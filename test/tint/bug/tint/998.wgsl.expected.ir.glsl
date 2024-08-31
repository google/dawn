#version 310 es

struct Constants {
  uint zero;
};

struct Result {
  uint value;
};

struct S {
  uint data[3];
};

uniform Constants constants;
Result result;
S s = S(uint[3](0u, 0u, 0u));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  s.data[constants.zero] = 0u;
}
