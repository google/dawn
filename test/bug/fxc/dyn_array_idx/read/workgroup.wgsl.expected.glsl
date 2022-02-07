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
shared S s;
void f(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 64u); idx = (idx + 1u)) {
      uint i = idx;
      s.data[i] = 0;
    }
  }
  barrier();
  result.tint_symbol = s.data[ubo.dynamic_idx];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
