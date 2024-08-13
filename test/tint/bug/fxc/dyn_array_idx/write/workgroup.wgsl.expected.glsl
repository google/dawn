#version 310 es

struct S {
  int data[64];
};

shared S s;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 64u); idx = (idx + 1u)) {
      uint i = idx;
      s.data[i] = 0;
    }
  }
  barrier();
}

struct UBO {
  int dynamic_idx;
};

layout(binding = 0, std140) uniform ubo_block_ubo {
  UBO inner;
} ubo;

struct Result {
  int tint_symbol;
};

layout(binding = 1, std430) buffer result_block_ssbo {
  Result inner;
} result;

void f(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  s.data[ubo.inner.dynamic_idx] = 1;
  result.inner.tint_symbol = s.data[3];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
