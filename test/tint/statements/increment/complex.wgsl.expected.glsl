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
  v = (v + 1u);
  return 1;
}
int idx2() {
  v = (v + 1u);
  return 2;
}
int idx3() {
  v = (v + 1u);
  return 3;
}
int idx4() {
  v = (v + 1u);
  return 4;
}
int idx5() {
  v = (v + 1u);
  return 0;
}
int idx6() {
  v = (v + 1u);
  return 2;
}
void v_2() {
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    int v_3 = idx1();
    int v_4 = idx2();
    uint v_5 = (uint(v_1.inner.length()) - 1u);
    uint v_6 = min(uint(v_3), v_5);
    uint v_7 = min(uint(v_4), 3u);
    int v_8 = idx3();
    uint v_9 = uint(v_1.inner[v_6].a[v_7][min(uint(v_8), 3u)]);
    int v_10 = int((v_9 + uint(1)));
    v_1.inner[v_6].a[v_7][min(uint(v_8), 3u)] = v_10;
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
        int v_11 = idx4();
        int v_12 = idx5();
        uint v_13 = (uint(v_1.inner.length()) - 1u);
        uint v_14 = min(uint(v_11), v_13);
        uint v_15 = min(uint(v_12), 3u);
        int v_16 = idx6();
        uint v_17 = uint(v_1.inner[v_14].a[v_15][min(uint(v_16), 3u)]);
        int v_18 = int((v_17 + uint(1)));
        v_1.inner[v_14].a[v_15][min(uint(v_16), 3u)] = v_18;
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
