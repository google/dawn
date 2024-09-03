#version 310 es

struct UBO {
  ivec4 data[4];
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

uniform UBO ubo;
Result result;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  result.tint_symbol = ubo.data[ubo.dynamic_idx].x;
}
