#version 310 es


struct Result {
  int member_0;
};

struct S {
  int data[64];
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
shared S s;
void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 64u)) {
        break;
      }
      s.data[v_2] = 0;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  uvec4 v_3 = v.inner[0u];
  uint v_4 = min(uint(int(v_3.x)), 63u);
  s.data[v_4] = 1;
  v_1.inner.member_0 = s.data[3];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
