
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
    int v_1 = idx1();
    int v_2 = idx2();
    uint v_3 = (uint(v_1) * 64u);
    uint v_4 = (uint(v_2) * 16u);
    int v_5 = idx3();
    uint v_6 = (uint(v_5) * 4u);
    int v_7 = (asint(buffer.Load((((0u + v_3) + v_4) + v_6))) - 1);
    uint v_8 = ((((0u + v_3) + v_4) + v_6) + (uint(v_5) * 4u));
    buffer.Store(v_8, asuint(v_7));
    while(true) {
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        int v_9 = idx4();
        int v_10 = idx5();
        uint v_11 = (uint(v_9) * 64u);
        uint v_12 = (uint(v_10) * 16u);
        int v_13 = idx6();
        uint v_14 = (uint(v_13) * 4u);
        int v_15 = (asint(buffer.Load((((0u + v_11) + v_12) + v_14))) - 1);
        uint v_16 = ((((0u + v_11) + v_12) + v_14) + (uint(v_13) * 4u));
        buffer.Store(v_16, asuint(v_15));
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

