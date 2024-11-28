#version 310 es


struct S {
  ivec4 a[4];
};

layout(binding = 0, std430)
buffer tint_symbol_block_1_ssbo {
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
void tint_symbol_1() {
  {
    int v_2 = idx1();
    int v_3 = idx2();
    uint v_4 = (uint(v_1.inner.length()) - 1u);
    uint v_5 = min(uint(v_2), v_4);
    uint v_6 = min(uint(v_3), 3u);
    int v_7 = idx3();
    int v_8 = (v_1.inner[v_5].a[v_6][min(uint(v_7), 3u)] + 1);
    v_1.inner[v_5].a[v_6][min(uint(v_7), 3u)] = v_8;
    while(true) {
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        int v_9 = idx4();
        int v_10 = idx5();
        uint v_11 = (uint(v_1.inner.length()) - 1u);
        uint v_12 = min(uint(v_9), v_11);
        uint v_13 = min(uint(v_10), 3u);
        int v_14 = idx6();
        int v_15 = (v_1.inner[v_12].a[v_13][min(uint(v_14), 3u)] + 1);
        v_1.inner[v_12].a[v_13][min(uint(v_14), 3u)] = v_15;
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
