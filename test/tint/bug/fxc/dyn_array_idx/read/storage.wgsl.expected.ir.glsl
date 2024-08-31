#version 310 es

struct UBO {
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

struct SSBO {
  int data[4];
};

uniform UBO ubo;
Result result;
SSBO ssbo;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  result.tint_symbol = ssbo.data[ubo.dynamic_idx];
}
