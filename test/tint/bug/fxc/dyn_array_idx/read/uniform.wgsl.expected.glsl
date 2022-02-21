#version 310 es

struct UBO {
  ivec4 data[4];
  int dynamic_idx;
};

layout(binding = 0) uniform UBO_1 {
  ivec4 data[4];
  int dynamic_idx;
} ubo;

struct Result {
  int tint_symbol;
};

layout(binding = 2, std430) buffer Result_1 {
  int tint_symbol;
} result;
void f() {
  result.tint_symbol = ubo.data[ubo.dynamic_idx].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
