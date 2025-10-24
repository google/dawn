#version 310 es


struct S {
  ivec4 a[4];
};

layout(binding = 0, std430)
buffer buffer_block_1_ssbo {
  S inner[];
} v_1;
uint v = 0u;
int idx1() {
  v = (v - 1u);
  return 1;
}
int idx2() {
  v = (v - 1u);
  return 2;
}
int idx3() {
  v = (v - 1u);
  return 3;
}
int idx4() {
  v = (v - 1u);
  return 4;
}
int idx5() {
  v = (v - 1u);
  return 0;
}
int idx6() {
  v = (v - 1u);
  return 2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    int v_2 = idx1();
    int v_3 = idx2();
    uint v_4 = (uint(v_1.inner.length()) - 1u);
    uint v_5 = min(uint(v_2), v_4);
    uint v_6 = min(uint(v_3), 3u);
    int v_7 = idx3();
    uint v_8 = uint(v_1.inner[v_5].a[v_6][min(uint(v_7), 3u)]);
    int v_9 = int((v_8 - uint(1)));
    v_1.inner[v_5].a[v_6][min(uint(v_7), 3u)] = v_9;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        int v_10 = idx4();
        int v_11 = idx5();
        uint v_12 = (uint(v_1.inner.length()) - 1u);
        uint v_13 = min(uint(v_10), v_12);
        uint v_14 = min(uint(v_11), 3u);
        int v_15 = idx6();
        uint v_16 = uint(v_1.inner[v_13].a[v_14][min(uint(v_15), 3u)]);
        int v_17 = int((v_16 - uint(1)));
        v_1.inner[v_13].a[v_14][min(uint(v_15), 3u)] = v_17;
      }
      continue;
    }
  }
}
