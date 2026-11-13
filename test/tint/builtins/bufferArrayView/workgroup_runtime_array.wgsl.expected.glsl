#version 310 es

layout(binding = 0, std430)
buffer in_block_1_ssbo {
  uint inner[];
} v;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  uint inner[];
} v_1;
shared uint b[64];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 64u)) {
        break;
      }
      b[v_2] = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  {
    int i = 0;
    while(true) {
      if ((i < 64)) {
      } else {
        break;
      }
      uint v_3 = (min(uint(i), (64u - 1u)) * 4u);
      int v_4 = i;
      uint v_5 = (uint(v.inner.length()) - 1u);
      uint v_6 = min(uint(v_4), v_5);
      b[(v_3 / 4u)] = v.inner[v_6];
      {
        i = int((uint(i) + 1u));
      }
    }
  }
  barrier();
  {
    int i = 0;
    while(true) {
      if ((i < 64)) {
      } else {
        break;
      }
      int v_7 = i;
      uint v_8 = (uint(v_1.inner.length()) - 1u);
      uint v_9 = min(uint(v_7), v_8);
      uint v_10 = ((min(uint(i), (64u - 1u)) * 4u) / 4u);
      v_1.inner[v_9] = b[v_10];
      {
        i = int((uint(i) + 1u));
      }
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
