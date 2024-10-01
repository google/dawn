#version 310 es


struct S {
  ivec4 a[4];
};

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  S tint_symbol_2[];
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
void tint_symbol_1() {
  {
    int v_2 = idx1();
    int v_3 = idx2();
    int v_4 = idx3();
    v_1.tint_symbol_2[v_2].a[v_3][v_4] = (v_1.tint_symbol_2[v_2].a[v_3][v_4] - 1);
    while(true) {
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        int v_5 = idx4();
        int v_6 = idx5();
        int v_7 = idx6();
        v_1.tint_symbol_2[v_5].a[v_6][v_7] = (v_1.tint_symbol_2[v_5].a[v_6][v_7] - 1);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
