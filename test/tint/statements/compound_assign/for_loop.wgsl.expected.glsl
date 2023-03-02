#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  int a;
  uint pad;
  uint pad_1;
  uint pad_2;
  vec4 b;
  mat2 c;
};

layout(binding = 0, std430) buffer v_block_ssbo {
  S inner;
} v;

uint i = 0u;
int idx1() {
  i = (i + 1u);
  return 1;
}

int idx2() {
  i = (i + 2u);
  return 1;
}

int idx3() {
  i = (i + 3u);
  return 1;
}

void foo() {
  float a[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    int tint_symbol_2 = idx1();
    int tint_symbol_save = tint_symbol_2;
    a[tint_symbol_save] = (a[tint_symbol_save] * 2.0f);
    while (true) {
      int tint_symbol_3 = idx2();
      if (!((a[tint_symbol_3] < 10.0f))) {
        break;
      }
      {
      }
      {
        int tint_symbol_4 = idx3();
        int tint_symbol_1_save = tint_symbol_4;
        a[tint_symbol_1_save] = (a[tint_symbol_1_save] + 1.0f);
      }
    }
  }
}

