[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer buffer : register(u0);
static uint v = 0u;

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

void main() {
  {
    int tint_symbol_save = idx1();
    int tint_symbol_save_1 = idx2();
    int tint_symbol_1 = idx3();
    buffer.Store((((64u * uint(tint_symbol_save)) + (16u * uint(tint_symbol_save_1))) + (4u * uint(tint_symbol_1))), asuint((asint(buffer.Load((((64u * uint(tint_symbol_save)) + (16u * uint(tint_symbol_save_1))) + (4u * uint(tint_symbol_1))))) - 1)));
    while (true) {
      if (!((v < 10u))) {
        break;
      }
      {
      }
      {
        int tint_symbol_2_save = idx4();
        int tint_symbol_2_save_1 = idx5();
        int tint_symbol_3 = idx6();
        buffer.Store((((64u * uint(tint_symbol_2_save)) + (16u * uint(tint_symbol_2_save_1))) + (4u * uint(tint_symbol_3))), asuint((asint(buffer.Load((((64u * uint(tint_symbol_2_save)) + (16u * uint(tint_symbol_2_save_1))) + (4u * uint(tint_symbol_3))))) - 1)));
      }
    }
  }
}
