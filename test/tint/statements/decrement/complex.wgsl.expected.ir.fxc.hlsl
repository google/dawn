
RWByteAddressBuffer buffer : register(u0);
static uint v = 0u;
int idx1() {
  v = (v - 1u);
  return int(1);
}

int idx2() {
  v = (v - 1u);
  return int(2);
}

int idx3() {
  v = (v - 1u);
  return int(3);
}

int idx4() {
  v = (v - 1u);
  return int(4);
}

int idx5() {
  v = (v - 1u);
  return int(0);
}

int idx6() {
  v = (v - 1u);
  return int(2);
}

void main() {
  {
    int v_1 = idx1();
    int v_2 = idx2();
    uint v_3 = (uint(v_1) * 64u);
    uint v_4 = (uint(v_2) * 16u);
    int v_5 = idx3();
    int v_6 = (asint(buffer.Load((((0u + v_3) + v_4) + (uint(v_5) * 4u)))) - int(1));
    buffer.Store((((0u + v_3) + v_4) + (uint(v_5) * 4u)), asuint(v_6));
    while(true) {
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        int v_7 = idx4();
        int v_8 = idx5();
        uint v_9 = (uint(v_7) * 64u);
        uint v_10 = (uint(v_8) * 16u);
        int v_11 = idx6();
        int v_12 = (asint(buffer.Load((((0u + v_9) + v_10) + (uint(v_11) * 4u)))) - int(1));
        buffer.Store((((0u + v_9) + v_10) + (uint(v_11) * 4u)), asuint(v_12));
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

