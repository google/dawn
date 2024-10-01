#version 310 es


struct UBO {
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

struct S {
  int data[64];
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  UBO tint_symbol_1;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  Result tint_symbol_3;
} v_1;
shared S s;
void f_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 64u)) {
        break;
      }
      s.data[v_3] = 0;
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  barrier();
  int v_4 = v.tint_symbol_1.dynamic_idx;
  s.data[v_4] = 1;
  v_1.tint_symbol_3.tint_symbol = s.data[3];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
